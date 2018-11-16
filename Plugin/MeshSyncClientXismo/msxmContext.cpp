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

void BufferRecord::buildMeshData(bool weld_vertices)
{
    if (!data.data())
        return;

    int num_indices = num_elements;
    int num_triangles = num_indices / 3;

    if (!dst_mesh) {
        dst_mesh = ms::Mesh::create();

        char path[128];
        sprintf(path, "/XismoMesh:ID[%08x]", handle);
        dst_mesh->path = path;
    }
    auto& mesh = *dst_mesh;
    mesh.visible = visible;
    if (!visible) {
        return;
    }

    auto vertices = (const xm_vertex1*)data.data();
    if (weld_vertices) {
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
    mesh.refine_settings.flags.swap_faces = true;
    mesh.refine_settings.flags.gen_tangents = 1;
    mesh.refine_settings.flags.invert_v = 1;
}

bool BufferRecord::isModelData() const
{
    return stride == sizeof(xm_vertex1) && triangle;
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
        return;
    }

    if (force) {
        m_material_manager.makeDirtyAll();
        m_entity_manager.makeDirtyAll();
    }

    for (auto& pair : m_buffer_records) {
        auto& buf = pair.second;
        if (buf.isModelData() && (buf.dirty || force)) {
            m_mesh_buffers.push_back(&buf);
            buf.dirty = false;
        }
    }
    if (m_mesh_buffers.empty() && m_meshes_deleted.empty() && (!m_settings.sync_camera || !m_camera_dirty)) {
        // nothing to send
        return;
    }

    // build material list
    {
        m_material_data.clear();
        auto findOrAddMaterial = [this](const MaterialRecord& md) {
            auto it = std::find(m_material_data.begin(), m_material_data.end(), md);
            if (it != m_material_data.end()) {
                return (int)std::distance(m_material_data.begin(), it);
            }
            else {
                int ret = (int)m_material_data.size();
                m_material_data.push_back(md);
                return ret;
            }
        };

        for (auto& pair : m_buffer_records) {
            auto& buf = pair.second;
            if (buf.isModelData()) {
                buf.material_id = findOrAddMaterial(buf.material);
            }
        }

        char name[128];
        int material_index = 0;
        for (auto& md : m_material_data) {
            int mid = material_index++;
            auto mat = ms::Material::create();
            sprintf(name, "XismoMaterial:ID[%04x]", mid);
            mat->id = mid;
            mat->name = name;
            mat->setColor(md.diffuse);
            m_material_manager.add(mat);
        }
    }

    // camera
    if (m_settings.sync_camera) {
        if (!m_camera) {
            m_camera = ms::Camera::create();
            m_camera->path = "/Main Camera";
        }
        m_camera->position = m_camera_pos;
        m_camera->rotation = m_camera_rot;
        m_camera->fov = m_camera_fov;
        m_camera->near_plane = m_camera_near;
        m_camera->far_plane = m_camera_far;
        m_camera_dirty = false;
    }


    if (!m_sender.on_prepare) {
        m_sender.on_prepare = [this]() {
            // add camera
            if (m_camera)
                m_entity_manager.add(m_camera);

            // gen welded meshes
            mu::parallel_for_each(m_mesh_buffers.begin(), m_mesh_buffers.end(), [&](BufferRecord *v) {
                v->buildMeshData(m_settings.weld_vertices);
                m_entity_manager.add(v->dst_mesh);
            });
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
            t.deleted = m_entity_manager.getDeleted();
        };
        m_sender.on_succeeded = [this]() {
            m_texture_manager.clearDirtyFlags();
            m_material_manager.clearDirtyFlags();
            m_entity_manager.clearDirtyFlags();
        };
    }
    m_sender.kick();
}

void msxmContext::onActiveTexture(GLenum texture)
{
    m_texture_slot = texture - GL_TEXTURE0;
}

void msxmContext::onBindTexture(GLenum target, GLuint texture)
{
    if (m_texture_slot == 0) {
        m_material_records.texture = texture;
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


void msxmContext::onGenBuffers(GLsizei n, GLuint * handles)
{
    for (int i = 0; i < (int)n; ++i) {
        auto it = std::find(m_meshes_deleted.begin(), m_meshes_deleted.end(), handles[i]);
        if (it != m_meshes_deleted.end()) {
            m_meshes_deleted.erase(it);
        }
    }
}

void msxmContext::onDeleteBuffers(GLsizei n, const GLuint * handles)
{
    for (int i = 0; i < n; ++i) {
        auto it = m_buffer_records.find(handles[i]);
        if (it != m_buffer_records.end()) {
            if (it->second.isModelData()) {
                m_meshes_deleted.push_back(it->second.handle);
            }
            m_buffer_records.erase(it);
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
                std::swap(buf->data, buf->tmp_data);
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

void msxmContext::onUniform4fv(GLint location, GLsizei count, const GLfloat * value)
{
    if (location == 3) {
        // diffuse
        m_material_records.diffuse.assign(value);
    }
}

void msxmContext::onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    if (location == 0) {
        // modelview matrix
        m_modelview.assign(value);
    }
    else if (location == 1) {
        // projection matrix
        m_proj.assign(value);
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
        if (buf->material != m_material_records) {
            buf->material = m_material_records;
            buf->dirty = true;
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

