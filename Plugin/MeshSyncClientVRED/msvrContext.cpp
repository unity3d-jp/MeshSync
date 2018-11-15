#include "pch.h"
#include "msvrContext.h"


bool BufferData::isModelData() const
{
    return false;
}

void BufferData::buildMeshData(bool weld_vertices)
{
}


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
}

void msvrContext::onActiveTexture(GLenum texture)
{
}

void msvrContext::onBindTexture(GLenum target, GLuint texture)
{
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
        auto it = m_buffers.find(buffers[i]);
        if (it != m_buffers.end()) {
            if (it->second.isModelData()) {
                m_meshes_deleted.push_back(it->second.handle);
            }
            m_buffers.erase(it);
        }
    }
}

void msvrContext::onBindBuffer(GLenum target, GLuint buffer)
{
    if (target == GL_ARRAY_BUFFER) {
        m_vb_handle = buffer;
    }
    else if (target == GL_ELEMENT_ARRAY_BUFFER) {
        m_ib_handle = buffer;
    }
}

void msvrContext::onBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    m_vb_handle = buffer;

    if (buffer != 0)
        m_vertex_attributes |= 1 << bindingindex;
    else
        m_vertex_attributes &= ~(1 << bindingindex);
}

void msvrContext::onBufferData(GLenum target, GLsizeiptr size, const void * data, GLenum usage)
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

void msvrContext::onMapBuffer(GLenum target, GLenum access, void *& mapped_data)
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

void msvrContext::onUnmapBuffer(GLenum target)
{
    if (auto *buf = getActiveBuffer(target)) {
        if (buf->mapped_data) {
            ms::parallel_invoke([buf]() {
                memcpy(buf->mapped_data, buf->tmp_data.data(), buf->tmp_data.size());
            },
            [buf]() {
                if (memcmp(buf->data.data(), buf->tmp_data.data(), buf->data.size()) != 0) {
                    buf->data = buf->tmp_data;
                    buf->dirty = true;
                }
            });
            buf->mapped_data = nullptr;
        }
    }
}

void msvrContext::onUniform4fv(GLint location, GLsizei count, const GLfloat * value)
{
    if (location == 3) {
        // diffuse
        m_material.diffuse.assign(value);
    }
}

void msvrContext::onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    //if (location == 1) {
    //    // projection matrix
    //    m_proj.assign(value);
    //}
    //else if (location == 2) {
    //    // modelview matrix
    //    m_modelview.assign(value);
    //}
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
        m_vertex_attributes |= 1 << index;
    }
}


void msvrContext::onDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices)
{
    auto *buf = getActiveBuffer(GL_ARRAY_BUFFER);
    if (mode == GL_TRIANGLES &&
        ((m_vertex_attributes & 0xffff) == 0xffff) && // model vb has 16 attributes
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
        if (buf->material != m_material) {
            buf->material = m_material;
            buf->dirty = true;
        }
    }
bailout:
    m_vertex_attributes = 0;
}

void msvrContext::onFlush()
{
}

BufferData* msvrContext::getActiveBuffer(GLenum target)
{
    if (target == GL_ARRAY_BUFFER) {
        return &m_buffers[m_vb_handle];
    }
    else if (target == GL_ELEMENT_ARRAY_BUFFER) {
        return &m_buffers[m_ib_handle];
    }
    return nullptr;
}


msvrContext* msvrGetContext()
{
    static std::unique_ptr<msvrContext> s_ctx;
    if (!s_ctx) {
        s_ctx.reset(new msvrContext());
        msvrInitializeWidget();
    }
    return s_ctx.get();
}
