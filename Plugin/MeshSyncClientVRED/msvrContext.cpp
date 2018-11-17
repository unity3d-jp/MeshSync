#include "pch.h"
#include "msvrContext.h"


msvrContext::msvrContext()
{
}

msvrContext::~msvrContext()
{
}

msvrSettings& msvrContext::getSettings()
{
    return m_settings;
}

void msvrContext::send(bool force)
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
        if (buf.dst_mesh)
            m_entity_manager.add(buf.dst_mesh);
    }

    // build material list
    {
        m_material_records.clear();
        auto findOrAddMaterial = [this](const MaterialRecord& md) {
            auto it = std::find(m_material_records.begin(), m_material_records.end(), md);
            if (it != m_material_records.end()) {
                return (int)std::distance(m_material_records.begin(), it);
            }
            else {
                int ret = (int)m_material_records.size();
                m_material_records.push_back(md);
                return ret;
            }
        };

        for (auto& pair : m_buffer_records) {
            auto& buf = pair.second;
            if (buf.dst_mesh) {
                buf.material_id = findOrAddMaterial(buf.material);
            }
        }

        char name[128];
        int material_index = 0;
        for (auto& md : m_material_records) {
            int mid = material_index++;
            auto mat = ms::Material::create();
            sprintf(name, "VREDMaterial:ID[%04x]", mid);
            mat->id = mid;
            mat->name = name;

            auto& stdmat = ms::AsStandardMaterial(*mat);
            stdmat.setColor(md.diffuse);
            stdmat.setColorMap(md.color_tex);
            stdmat.setNormalMap(md.normal_tex);
            m_material_manager.add(mat);
        }
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
        m_entity_manager.add(m_camera);
    }

    for (auto& kvp : m_texture_records) {
        auto& rec = kvp.second;
        if (rec.dst && rec.dirty && rec.used) {
            m_texture_manager.add(rec.dst);
            rec.dirty = false;
        }
    }


    if (!m_sender.on_prepare) {
        m_sender.on_prepare = [this]() {
            // handle deleted objects
            for (auto h : m_meshes_deleted) {
                char path[128];
                sprintf(path, "/VREDMesh:ID[%08x]", h);
                m_entity_manager.erase(ms::Identifier(path, (int)h));
            }
            m_meshes_deleted.clear();


            auto& t = m_sender;
            t.client_settings = m_settings.client_settings;
            t.scene_settings.handedness = ms::Handedness::LeftZUp;
            t.scene_settings.scale_factor = m_settings.scale_factor;

            t.textures = m_texture_manager.getDirtyTextures();
            t.materials = m_material_manager.getDirtyMaterials();
            t.transforms = m_entity_manager.getDirtyTransforms();
            t.geometries = m_entity_manager.getDirtyGeometries();
            t.deleted_entities = m_entity_manager.getDeleted();
        };
        m_sender.on_succeeded = [this]() {
            m_texture_manager.clearDirtyFlags();
            m_material_manager.clearDirtyFlags();
            m_entity_manager.clearDirtyFlags();
        };
    }
    m_sender.kick();
}


void msvrContext::onGenTextures(GLsizei n, GLuint * textures)
{
    for (int i = 0; i < (int)n; ++i) {
        m_texture_records[textures[i]];
    }
}

void msvrContext::onDeleteTextures(GLsizei n, const GLuint * textures)
{
    for (int i = 0; i < (int)n; ++i) {
        m_texture_records.erase(textures[i]);
    }
}

void msvrContext::onActiveTexture(GLenum texture)
{
    m_active_texture = texture - GL_TEXTURE0;
}

void msvrContext::onBindTexture(GLenum target, GLuint texture)
{
    if (target == GL_TEXTURE_2D) {
        m_texture_slots[m_active_texture] = texture;
    }
}

void msvrContext::onTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * data)
{
    if (data && target == GL_TEXTURE_2D && level == 0) {
        auto handle = m_texture_slots[m_active_texture];
        auto& rec = m_texture_records[handle];

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
            sprintf(name, "VREDTexture_ID%08x", handle);
            dst->name = name;
            dst->id = (int)handle;
            dst->format = msf;
            dst->width = width;
            dst->height = height;

            int size = width * height * pixel_size;
            dst->data.assign((char*)data, (char*)data + size);
            rec.dst = dst;
            rec.dirty = true;
        }
        else {
            msLogWarning("unsupported texture format\n");
        }
    }
}

void msvrContext::onGenFramebuffers(GLsizei n, GLuint * ids)
{
}

void msvrContext::onBindFramebuffer(GLenum target, GLuint framebuffer)
{
    if (target != GL_FRAMEBUFFER)
        return;
    m_fb_handle = framebuffer;
}

void msvrContext::onDeleteFramebuffers(GLsizei n, GLuint *framebuffers)
{
    for (int i = 0; i < n; ++i)
        m_framebuffer_records.erase(framebuffers[i]);
}

void msvrContext::onFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    if (target != GL_FRAMEBUFFER)
        return;

    auto& rec = m_framebuffer_records[m_fb_handle];
    if (attachment == GL_DEPTH_ATTACHMENT)
        rec.depth_stencil = texture;
    else
        rec.colors[attachment - GL_COLOR_ATTACHMENT0] = texture;
}

void msvrContext::onFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    if (target != GL_FRAMEBUFFER || textarget != GL_TEXTURE_2D)
        return;

    auto& rec = m_framebuffer_records[m_fb_handle];
    if (attachment == GL_DEPTH_ATTACHMENT)
        rec.depth_stencil = texture;
    else
        rec.colors[attachment - GL_COLOR_ATTACHMENT0] = texture;
}


void msvrContext::onGenBuffers(GLsizei n, GLuint *buffers)
{
    for (int i = 0; i < (int)n; ++i) {
        auto it = std::find(m_meshes_deleted.begin(), m_meshes_deleted.end(), buffers[i]);
        if (it != m_meshes_deleted.end()) {
            m_meshes_deleted.erase(it);
        }
    }
}

void msvrContext::onDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    for (int i = 0; i < n; ++i) {
        auto it = m_buffer_records.find(buffers[i]);
        if (it != m_buffer_records.end()) {
            auto& rec = it->second;
            if (rec.dst_mesh)
                m_entity_manager.erase(rec.dst_mesh->getIdentifier());
            m_buffer_records.erase(it);
        }
    }
}

void msvrContext::onBindBuffer(GLenum target, GLuint buffer)
{
    switch (target) {
    case GL_ARRAY_BUFFER:
        m_vb_handle = buffer;
        break;
    case GL_ELEMENT_ARRAY_BUFFER:
        m_ib_handle = buffer;
        break;
    case GL_UNIFORM_BUFFER:
        m_ub_handle = buffer;
        break;
    }
}

void msvrContext::onBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    m_vb_handle = buffer;
    auto& rec = m_buffer_records[buffer];
    rec.stride = stride;
}

void msvrContext::onBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    switch (target) {
    case GL_UNIFORM_BUFFER:
        m_ub_handles[index] = buffer;
        break;
    }
}

void msvrContext::onBufferData(GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    if (auto *buf = getActiveBuffer(target)) {
        buf->data.resize_discard(size);
        if (data) {
            memcpy(buf->data.data(), data, buf->data.size());
            buf->dirty = true;
        }
    }
}

void msvrContext::onNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, const void * data)
{
    auto *buf = &m_buffer_records[buffer];
    buf->data.resize_discard(offset + size);
    if (data) {
        memcpy(buf->data.data() + offset, data, size);
        buf->dirty = true;
    }
}

void msvrContext::onMapBuffer(GLenum target, GLenum access, void *& mapped_data)
{
    if (access != GL_WRITE_ONLY) {
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

void msvrContext::onMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access, void *& mapped_data)
{
    if ((access & GL_MAP_WRITE_BIT) == 0) {
        return;
    }

    // same as onMapBuffer()
    if (auto *buf = getActiveBuffer(target)) {
        buf->mapped_data = mapped_data;
        buf->tmp_data.resize_discard(buf->data.size());
        mapped_data = buf->tmp_data.data() + offset;
    }
}

void msvrContext::onUnmapBuffer(GLenum target)
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

void msvrContext::onFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
    if (auto *buf = getActiveBuffer(target)) {
        if (buf->mapped_data) {
            memcpy((char*)buf->mapped_data, buf->tmp_data.data() + offset, length);
            if (memcmp(buf->data.data() + offset, buf->tmp_data.data() + offset, length) != 0) {
                memcpy(buf->data.data() + offset, buf->data.data() + offset, length);
                buf->dirty = true;
            }
            buf->mapped_data = nullptr;
        }
    }
}


void msvrContext::onGenVertexArrays(GLsizei n, GLuint *buffers)
{
}

void msvrContext::onDeleteVertexArrays(GLsizei n, const GLuint *buffers)
{
}

void msvrContext::onBindVertexArray(GLuint buffer)
{
}

void msvrContext::onEnableVertexAttribArray(GLuint index)
{
}

void msvrContext::onDisableVertexAttribArray(GLuint index)
{
}

void msvrContext::onVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer)
{
    if (auto *buf = getActiveBuffer(GL_ARRAY_BUFFER)) {
        buf->stride = stride;
    }
}


void msvrContext::onDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices)
{
    if (mode != GL_TRIANGLES)
        return;

    auto& fb = m_framebuffer_records[m_fb_handle];
    if (!fb.isMainTarget())
        return;

    auto *vb = getActiveBuffer(GL_ARRAY_BUFFER);
    auto *ib = getActiveBuffer(GL_ELEMENT_ARRAY_BUFFER);
    if (!ib || !vb || vb->stride != sizeof(vr_vertex))
        return;

    auto& camera_buf = m_buffer_records[m_ub_handles[1]];
    auto& obj_buf = m_buffer_records[m_ub_handles[2]];
    if (camera_buf.data.size() != 992 || obj_buf.data.size() != 560)
        return;

    // camera
    {
        auto view = (float4x4&)camera_buf.data[64 * 4];
        auto proj = (float4x4&)camera_buf.data[0];

        // projection matrix -> fov, aspect, clippling planes
        float fov, aspect, near_plane, far_plane;
        extract_projection_data(proj, fov, aspect, far_plane, near_plane);

        // fov >= 180.0f means came is not perspective. capture only when camera is perspective.
        if (fov < 179.0f) {
            if (fov != m_camera_fov) {
                m_camera_dirty = true;
                m_camera_fov = fov;
                m_camera_near = near_plane;
                m_camera_far = far_plane;
            }

            // modelview matrix -> camera pos & rot
            float3 pos;
            quatf rot;
            extract_look_data(view, pos, rot);
            rot *= mu::rotateX(-90.0f * mu::Deg2Rad);

            if (pos != m_camera_pos || rot != m_camera_rot)
            {
                m_camera_dirty = true;
                m_camera_pos = pos;
                m_camera_rot = rot;
            }
        }
    }

    // textures
    for (auto& h : m_texture_slots) {
        auto& rec = m_texture_records[h];
        if (rec.dst) {
            rec.used = true;
        }
    }

    {
        if (vb->material != m_material) {
            vb->material = m_material;
            vb->dirty = true;
        }

        auto transform = (float4x4&)obj_buf.data[0];
        auto task = [this, vb, ib, count, type, transform]() {
            if (!vb->dst_mesh) {
                vb->dst_mesh = ms::Mesh::create();

                char path[128];
                sprintf(path, "/VREDMesh:ID[%08x]", m_vb_handle);
                vb->dst_mesh->path = path;
            }
            auto& dst = *vb->dst_mesh;

            dst.position = extract_position(transform);
            dst.rotation = extract_rotation(transform);
            dst.scale = extract_scale(transform);

            if (vb->dirty) {
                size_t num_indices = count;
                size_t num_triangles = count / 3;
                size_t num_vertices = vb->data.size() / vb->stride;

                // convert vertices
                dst.points.resize_discard(num_vertices);
                dst.normals.resize_discard(num_vertices);
                dst.uv0.resize_discard(num_vertices);
                auto *vtx = (vr_vertex*)vb->data.data();
                for (size_t vi = 0; vi < num_vertices; ++vi) {
                    dst.points[vi] = vtx[vi].vertex;
                    dst.normals[vi] = vtx[vi].normal;
                    dst.uv0[vi] = float2{ 1.0f, 1.0f } - vtx[vi].uv;
                }

                // convert indices
                dst.counts.resize(num_triangles, 3);
                if (type == GL_UNSIGNED_INT) {
                    int *src = (int*)ib->data.data();
                    dst.indices.assign(src, src + num_indices);
                }
                else if (type == GL_UNSIGNED_SHORT) {
                    uint16_t *src = (uint16_t*)ib->data.data();
                    dst.indices.resize_discard(num_indices);
                    for (size_t ii = 0; ii < num_indices; ++ii)
                        dst.indices[ii] = src[ii];
                }

                dst.setupFlags();
                dst.flags.has_refine_settings = 1;
                dst.refine_settings.flags.swap_faces = true;
                dst.refine_settings.flags.gen_tangents = 1;
            }
        };
        task();
    }
    vb->dirty = false;
}

void msvrContext::onFlush()
{
    if (m_settings.auto_sync)
        send(false);
}

BufferRecord* msvrContext::getActiveBuffer(GLenum target)
{
    switch (target) {
    case GL_ARRAY_BUFFER:
        return &m_buffer_records[m_vb_handle];
    case GL_ELEMENT_ARRAY_BUFFER:
        return &m_buffer_records[m_ib_handle];
    case GL_UNIFORM_BUFFER:
        return &m_buffer_records[m_ub_handle];
    }
    return nullptr;
}


msvrContext* msvrGetContext()
{
    static std::unique_ptr<msvrContext> s_ctx;
    if (!s_ctx) {
        s_ctx.reset(new msvrContext());
    }
    return s_ctx.get();
}

bool FramebufferRecord::isMainTarget() const
{
    return colors[0] && colors[1] && depth_stencil;
}
