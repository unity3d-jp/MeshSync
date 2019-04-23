#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"
#include "msxmContext.h"



int msxmGetXismoVersion()
{
    static int s_version;
    if (s_version == 0) {
        char buf[2048];
        GetModuleFileNameA(nullptr, buf, sizeof(buf));
        std::string path = buf;
        auto spos = path.find_last_of("\\");
        if (spos != std::string::npos) {
            path.resize(spos + 1);
            path += "xismo.ini";
        }

        std::fstream ifs(path.c_str(), std::ios::in|std::ios::binary);
        if (ifs) {
            ifs.getline(buf, sizeof(buf));
            ifs.getline(buf, sizeof(buf));
            int v;
            if (sscanf(buf, "AppVer=%d", &v)) {
                s_version = v;
            }
        }
    }
    return s_version;
}

msxmContext* msxmGetContext()
{
    static std::unique_ptr<msxmContext> s_ctx;
    if (!s_ctx) {
        s_ctx.reset(new msxmContext());
        msxmInitializeWidget();
    }
    return s_ctx.get();
}


bool MaterialRecord::operator==(const MaterialRecord& v) const
{
    return
        program == v.program &&
        diffuse_color == v.diffuse_color &&
        emission_color == v.emission_color &&
        color_map == v.color_map&&
        bump_map == v.bump_map;
}
bool MaterialRecord::operator!=(const MaterialRecord& v) const
{
    return !operator==(v);
}
uint64_t MaterialRecord::checksum() const
{
    uint64_t ret = 0;
    ret += ms::csum(program);
    ret += ms::csum(diffuse_color);
    ret += ms::csum(emission_color);
    ret += ms::csum(color_map);
    ret += ms::csum(bump_map);
    return ret;
}


template<class SrcVertexT, class DstVertexT>
static void Weld(const SrcVertexT src[], int num_vertices, RawVector<DstVertexT>& dst_vertices, RawVector<int>& dst_indices)
{
    dst_vertices.clear();
    dst_vertices.reserve_discard(num_vertices / 2);
    dst_indices.resize_discard(num_vertices);

    for (int vi = 0; vi < num_vertices; ++vi) {
        auto tmp = src[vi];
        auto it = std::find_if(dst_vertices.begin(), dst_vertices.end(), [&](const DstVertexT& v) { return tmp == v; });
        if (it != dst_vertices.end()) {
            int pos = (int)std::distance(dst_vertices.begin(), it);
            dst_indices[vi] = pos;
        }
        else {
            int pos = (int)dst_vertices.size();
            dst_indices[vi] = pos;
            dst_vertices.push_back(tmp);
        }
    }
}

void BufferRecord::startBuildMeshData(const msxmSettings& settings)
{
    if (!dst_mesh) {
        dst_mesh = ms::Mesh::create();
        dst_mesh->index = (int)handle;

        char path[128];
        sprintf(path, "/XismoMesh:ID[%08x]", handle);
        dst_mesh->path = path;
    }

    if (dirty) {
        dirty = false;
        vertices_tmp.resize_discard(num_elements);
        data.copy_to((char*)vertices_tmp.data());
        task = std::async(std::launch::async, [this, &settings]() {
            buildMeshDataBody(settings);
        });
    }
}

void BufferRecord::buildMeshDataBody(const msxmSettings& settings)
{
    int num_indices = (int)vertices_tmp.size();
    int num_triangles = num_indices / 3;

    auto& mesh = *dst_mesh;
    auto vertices = vertices_tmp.data();
    if (settings.weld_vertices) {
        Weld(vertices, num_indices, vertices_welded, mesh.indices);

        int num_vertices = (int)vertices_welded.size();
        mesh.points.resize_discard(num_vertices);
        mesh.normals.resize_discard(num_vertices);
        mesh.uv0.resize_discard(num_vertices);
        mesh.colors.resize_discard(num_vertices);

        for (int vi = 0; vi < num_vertices; ++vi) {
            auto& v = vertices_welded[vi];
            mesh.points[vi] = v.vertex;
            mesh.normals[vi] = v.normal;
            mesh.uv0[vi] = v.uv;
            mesh.colors[vi] = v.color;
        }
    }
    else {
        int num_vertices = num_indices;
        mesh.points.resize_discard(num_vertices);
        mesh.normals.resize_discard(num_vertices);
        mesh.uv0.resize_discard(num_vertices);
        mesh.colors.resize_discard(num_vertices);
        mesh.indices.resize_discard(num_vertices);

        for (int vi = 0; vi < num_vertices; ++vi) {
            mesh.points[vi] = vertices[vi].vertex;
            mesh.normals[vi] = vertices[vi].normal;
            mesh.uv0[vi] = vertices[vi].uv;
            mesh.colors[vi] = vertices[vi].color;
            mesh.indices[vi] = vi;
        }
    }

    mesh.counts.resize_discard(num_triangles);
    mesh.material_ids.resize_discard(num_triangles);
    for (int ti = 0; ti < num_triangles; ++ti) {
        mesh.counts[ti] = 3;
        mesh.material_ids[ti] = material_id;
    }

    mesh.setupFlags();
    mesh.flags.has_refine_settings = 1;
    mesh.refine_settings.flags.flip_faces = true;
    mesh.refine_settings.flags.gen_tangents = 1;
    mesh.refine_settings.flags.make_double_sided = settings.make_double_sided;
}

void BufferRecord::wait()
{
    if (task.valid()) {
        task.wait();
        task = {};
    }
}

BufferRecord::~BufferRecord()
{
    wait();
}

bool BufferRecord::isModelData() const
{
    return stride == sizeof(xm_vertex1) && triangle && !data.empty();
}



msxmContext::msxmContext()
{
}

msxmContext::~msxmContext()
{
    m_sender.wait();
}

msxmSettings& msxmContext::getSettings()
{
    return m_settings;
}

void msxmContext::send(bool force)
{
    if (m_sender.isSending()) {
        // previous request is not completed yet
        if (force)
            m_sender.wait();
        else
            return;
    }

    if (force) {
        m_material_manager.makeDirtyAll();
        m_entity_manager.makeDirtyAll();
    }

    bool dirty_meshes = false;
    for (auto& pair : m_buffer_records) {
        auto& buf = pair.second;
        if (buf.isModelData())
            m_mesh_buffers.push_back(&buf);
        if (buf.dirty)
            dirty_meshes = true;
    }
    if ((!dirty_meshes && !force) && m_meshes_deleted.empty() && (!m_settings.sync_camera || !m_camera_dirty)) {
        // nothing to send
        return;
    }


    // textures
    if (m_settings.sync_textures) {
        for (auto& kvp : m_texture_records) {
            auto& rec = kvp.second;
            if (rec.dst && rec.dirty && rec.used) {
                m_texture_manager.add(rec.dst);
                rec.dirty = false;
            }
        }
    }

    // build material list
    {
        char name[128];
        for (auto& mr : m_material_records) {
            auto mat = ms::Material::create();
            sprintf(name, "XismoMaterial:ID[%04x]", mr.id);
            mat->id = mr.id;
            mat->index = mr.id;
            mat->name = name;

            auto& stdmat = ms::AsStandardMaterial(*mat);
            if (mr.diffuse_color != float4::zero())
                stdmat.setColor(mr.diffuse_color);
            if (mr.emission_color != float4::zero())
                stdmat.setEmissionColor(mr.emission_color);
            if (mr.color_map != ms::InvalidID)
                stdmat.setColorMap({ mr.color_map });
            if (mr.bump_map != ms::InvalidID)
                stdmat.setBumpMap({ mr.bump_map });
            m_material_manager.add(mat);
        }
        m_material_records.clear();
    }

    // camera
    if (m_settings.sync_camera) {
        if (!m_camera) {
            m_camera = ms::Camera::create();
        }
        m_camera->path = m_settings.camera_path;
        m_camera->position = m_camera_pos;
        m_camera->rotation = m_camera_rot;
        m_camera->fov = m_camera_fov;
        m_camera->near_plane = m_camera_near;
        m_camera->far_plane = m_camera_far;
        m_camera_dirty = false;
    }

    // begin build mesh data
    mu::parallel_for_each(m_mesh_buffers.begin(), m_mesh_buffers.end(), [&](BufferRecord *v) {
        v->startBuildMeshData(m_settings);
    });

    m_sender.on_prepare = [this]() {
        // add camera
        if (m_camera)
            m_entity_manager.add(m_camera);

        // gen welded meshes
        for (auto *v : m_mesh_buffers) {
            v->wait();
            m_entity_manager.add(v->dst_mesh);
        }
        m_mesh_buffers.clear();

        // handle deleted objects
        for (auto h : m_meshes_deleted) {
            char path[128];
            sprintf(path, "/XismoMesh:ID[%08x]", h);
            m_entity_manager.erase(ms::Identifier(path, (int)h));
        }
        m_meshes_deleted.clear();


        auto& t = m_sender;
        t.client_settings = m_settings.client_settings;
        t.scene_settings.handedness = ms::Handedness::Left;
        t.scene_settings.scale_factor = m_settings.scale_factor;

        t.textures = m_texture_manager.getDirtyTextures();
        t.materials = m_material_manager.getDirtyMaterials();
        t.transforms = m_entity_manager.getDirtyTransforms();
        t.geometries = m_entity_manager.getDirtyGeometries();
        t.deleted_entities = m_entity_manager.getDeleted();
    };
    m_sender.on_success = [this]() {
        m_texture_manager.clearDirtyFlags();
        m_material_manager.clearDirtyFlags();
        m_entity_manager.clearDirtyFlags();
    };
    m_sender.kick();
}

void msxmContext::onGenTextures(GLsizei n, GLuint *textures)
{
    for (int i = 0; i < (int)n; ++i) {
        m_texture_records[textures[i]];
    }
}

void msxmContext::onDeleteTextures(GLsizei n, const GLuint *textures)
{
    for (int i = 0; i < (int)n; ++i) {
        m_texture_records.erase(textures[i]);
    }
}

void msxmContext::onActiveTexture(GLenum texture)
{
    m_active_texture = texture - GL_TEXTURE0;
}

void msxmContext::onBindTexture(GLenum target, GLuint texture)
{
    m_texture_slots[m_active_texture] = texture;
}

void msxmContext::onTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    if (!pixels)
        return;

    int pixel_size = 0;
    ms::TextureFormat msf = ms::TextureFormat::Unknown;
    switch (format) {
    case GL_RED:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            pixel_size = 1;
            msf = ms::TextureFormat::Ru8;
            break;
        case GL_HALF_FLOAT:
            pixel_size = 2;
            msf = ms::TextureFormat::Rf16;
            break;
        case GL_FLOAT:
            pixel_size = 4;
            msf = ms::TextureFormat::Rf32;
            break;
        }
        break;
    case GL_RG:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            pixel_size = 2;
            msf = ms::TextureFormat::RGu8;
            break;
        case GL_HALF_FLOAT:
            pixel_size = 4;
            msf = ms::TextureFormat::RGf16;
            break;
        case GL_FLOAT:
            pixel_size = 8;
            msf = ms::TextureFormat::RGf32;
            break;
        }
        break;
    case GL_RGB:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            pixel_size = 3;
            msf = ms::TextureFormat::RGBu8;
            break;
        case GL_HALF_FLOAT:
            pixel_size = 6;
            msf = ms::TextureFormat::RGBf16;
            break;
        case GL_FLOAT:
            pixel_size = 12;
            msf = ms::TextureFormat::RGBf32;
            break;
        }
        break;
    case GL_RGBA:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            pixel_size = 4;
            msf = ms::TextureFormat::RGBAu8;
            break;
        case GL_HALF_FLOAT:
            pixel_size = 8;
            msf = ms::TextureFormat::RGBAf16;
            break;
        case GL_FLOAT:
            pixel_size = 16;
            msf = ms::TextureFormat::RGBAf32;
            break;
        }
        break;
    }

    if (pixel_size) {
        auto dst = ms::Texture::create();
        char name[128];
        sprintf(name, "XismoTexture_ID%08x", texture);
        dst->name = name;
        dst->id = (int)texture;
        dst->format = msf;
        dst->width = width;
        dst->height = height;

        int size = width * height * pixel_size;
        dst->data.assign((char*)pixels, (char*)pixels + size);

        auto& rec = m_texture_records[texture];
        rec.dst = dst;
        rec.dirty = true;
    }
    else {
        msLogWarning("unsupported texture format\n");
    }
}

BufferRecord* msxmContext::getActiveBuffer(GLenum target)
{
    uint32_t bid = 0;
    if (target == GL_ARRAY_BUFFER) {
        bid = m_vb_handle;
    }
    return bid != 0 ? &m_buffer_records[bid] : nullptr;
}

ProgramRecord::Uniform * msxmContext::findUniform(GLint location)
{
    auto& uniforms = m_program_records[m_program_handle].uniforms;
    auto it = uniforms.find(location);
    return it != uniforms.end() ? &it->second : nullptr;
}


void msxmContext::onGenBuffers(GLsizei n, GLuint *buffers)
{
    for (int i = 0; i < (int)n; ++i) {
        auto it = std::find(m_meshes_deleted.begin(), m_meshes_deleted.end(), buffers[i]);
        if (it != m_meshes_deleted.end()) {
            m_meshes_deleted.erase(it);
        }
    }
}

void msxmContext::onDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    if (m_settings.sync_delete) {
        for (int i = 0; i < n; ++i) {
            auto it = m_buffer_records.find(buffers[i]);
            if (it != m_buffer_records.end()) {
                if (m_settings.sync_delete && it->second.isModelData())
                    m_meshes_deleted.push_back(it->second.handle);
                m_buffer_records.erase(it);
            }
        }
    }
}

void msxmContext::onBindBuffer(GLenum target, GLuint buffer)
{
    if (target == GL_ARRAY_BUFFER) {
        m_vb_handle = buffer;
    }
}

void msxmContext::onBufferData(GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    if (auto *buf = getActiveBuffer(target)) {
        buf->handle = m_vb_handle;
        buf->data.resize_discard(size);
        if (data) {
            memcpy(buf->data.data(), data, buf->data.size());
            buf->dirty = true;
        }
    }
}

void msxmContext::onMapBuffer(GLenum target, GLenum access, void *&mapped_data)
{
    if (target != GL_ARRAY_BUFFER || access != GL_WRITE_ONLY) {
        return;
    }

    // mapped memory returned by glMapBuffer() is a special kind of memory and reading it is exetemery slow.
    // so make temporary memory and return it to application, and copy it to actual mapped memory later (onUnmapBuffer()).
    if (auto *buf = getActiveBuffer(target)) {
        buf->mapped_data = mapped_data;
        buf->tmp_data.resize_discard(buf->data.size());
        mapped_data = buf->tmp_data.data();
    }
}

void msxmContext::onUnmapBuffer(GLenum target)
{
    if (auto *buf = getActiveBuffer(target)) {
        if (buf->mapped_data) {
            memcpy(buf->mapped_data, buf->tmp_data.data(), buf->tmp_data.size());
            if (buf->data != buf->tmp_data) {
                buf->data = buf->tmp_data;
                buf->dirty = true;
            }
            buf->mapped_data = nullptr;
        }
    }
}

void msxmContext::onVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer)
{
    if (auto *buf = getActiveBuffer(GL_ARRAY_BUFFER)) {
        buf->stride = stride;
        m_vertex_attributes |= 1 << index;
    }
}


extern void(*_glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
extern void(*_glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern GLint(*_glGetUniformLocation)(GLuint program, const GLchar *name);

void msxmContext::onLinkProgram(GLuint program)
{
}

void msxmContext::onDeleteProgram(GLuint program)
{
    m_program_records.erase(program);
}

void msxmContext::onUseProgram(GLuint program)
{
    m_program_handle = program;

    auto& rec = m_program_records[program];
    if (rec.uniforms.empty()) {
        rec.mrec.program = program;
        int num_uniforms = 0;
        _glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num_uniforms);
        for (int ui = 0; ui < num_uniforms; ++ui) {
            char name[128];
            GLsizei len = 0, size = 0;
            GLenum type;
            _glGetActiveUniform(program, ui, sizeof(name), &len, &size, &type, name);

            auto mstype = ms::MaterialProperty::Type::Unknown;
            switch (type) {
            case GL_FLOAT:
                mstype = ms::MaterialProperty::Type::Float;
                break;
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4:
                mstype = ms::MaterialProperty::Type::Vector;
                break;
            case GL_FLOAT_MAT2:
            case GL_FLOAT_MAT3:
            case GL_FLOAT_MAT4:
                mstype = ms::MaterialProperty::Type::Matrix;
                break;
            case GL_SAMPLER_2D:
            case GL_SAMPLER_2D_ARRAY:
            case GL_SAMPLER_2D_MULTISAMPLE:
            case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
            case GL_SAMPLER_2D_RECT:
            case GL_INT_SAMPLER_2D:
            case GL_INT_SAMPLER_2D_ARRAY:
            case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
            case GL_INT_SAMPLER_2D_RECT:
            case GL_UNSIGNED_INT_SAMPLER_2D:
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
            case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
            case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
                mstype = ms::MaterialProperty::Type::Texture;
                break;
            }

            GLuint index = _glGetUniformLocation(program, name);
            auto& uni = rec.uniforms[index];
            uni.name = name;
            uni.type = mstype;
            uni.size = size;
        }
    }
}

void msxmContext::onUniform1i(GLint location, GLint v0)
{
    if (auto *prop = findUniform(location)) {
        if (prop->type == ms::MaterialProperty::Type::Texture) {
            auto& mr = m_program_records[m_program_handle].mrec;
            if (prop->name == "dif_tex")
                mr.color_map = v0;
            else if (prop->name == "normal_tex")
                mr.bump_map = v0;
        }
    }
}

void msxmContext::onUniform4fv(GLint location, GLsizei count, const GLfloat *value)
{
    if (auto *prop = findUniform(location)) {
        auto& mr = m_program_records[m_program_handle].mrec;
        if (prop->name == "diffuse")
            mr.diffuse_color = *(float4*)value;
        else if (prop->name == "emissive")
            mr.emission_color = *(float4*)value;
    }
}

void msxmContext::onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (auto *prop = findUniform(location)) {
        auto& mr = m_program_records[m_program_handle].mrec;
        if (prop->name == "projMatrix")
            m_proj = *(float4x4*)value;
        else if (prop->name == "mvMatrix")
            m_modelview = *(float4x4*)value;
    }
}

void msxmContext::onDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
    auto *buf = getActiveBuffer(GL_ARRAY_BUFFER);
    if (mode == GL_TRIANGLES &&
        ((m_vertex_attributes & 0x1f) == 0x1f) && // model vb has 5 attributes
        (buf && buf->stride == sizeof(xm_vertex1)))
    {
        {
            // projection matrix -> fov, aspect, clippling planes
            float fov, aspect, near_plane, far_plane;
            extract_projection_data(m_proj, fov, aspect, near_plane, far_plane);

            if (fov > 179.0f) {
                // camera is not perspective. capture only when camera is perspective.
                goto bailout;
            }
            if (fov != m_camera_fov) {
                m_camera_dirty = true;
                m_camera_fov = fov;
                m_camera_near = near_plane;
                m_camera_far = far_plane;
            }
        }
        {
            // modelview matrix -> camera pos & rot
            float3 pos;
            quatf rot;
            extract_look_data(m_modelview, pos, rot);
            if (pos != m_camera_pos ||
                rot != m_camera_rot)
            {
                m_camera_dirty = true;
                m_camera_pos = pos;
                m_camera_rot = rot;
            }
        }

        buf->triangle = true;
        buf->num_elements = (int)count;

        // material
        {
            auto& prec = m_program_records[m_program_handle];
            auto mrec = prec.mrec; // copy

            // texture slot -> id
            auto texture_slot_to_id = [this](int slot, ms::TextureType ttype) {
                if (slot == ms::InvalidID)
                    return ms::InvalidID;
                auto& trec = m_texture_records[m_texture_slots[slot]];
                if (trec.dst) {
                    trec.used = true;
                    trec.dst->type = ttype;
                    return trec.dst->id;
                }
                else {
                    return ms::InvalidID;
                }
            };
            mrec.color_map = texture_slot_to_id(mrec.color_map, ms::TextureType::Default);
            mrec.bump_map = texture_slot_to_id(mrec.bump_map, ms::TextureType::NormalMap);

            auto it = std::find(m_material_records.begin(), m_material_records.end(), mrec);
            if (it != m_material_records.end()) {
                buf->material_id = it->id;
            }
            else {
                mrec.id = m_material_ids.getID(mrec);
                buf->material_id = mrec.id;
                m_material_records.push_back(mrec);
            }
        }

    }
bailout:
    m_vertex_attributes = 0;
}

void msxmContext::onFlush()
{
    if (m_settings.auto_sync) {
        send(false);
    }
}

