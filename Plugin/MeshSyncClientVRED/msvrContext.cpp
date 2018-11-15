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

void msvrContext::onGenBuffers(GLsizei n, GLuint * buffers)
{
}

void msvrContext::onDeleteBuffers(GLsizei n, const GLuint * buffers)
{
}

void msvrContext::onBindBuffer(GLenum target, GLuint buffer)
{
}

void msvrContext::onBufferData(GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
}

void msvrContext::onMapBuffer(GLenum target, GLenum access, void *& mapped_data)
{
}

void msvrContext::onUnmapBuffer(GLenum target)
{
}

void msvrContext::onVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer)
{
}

void msvrContext::onUniform4fv(GLint location, GLsizei count, const GLfloat * value)
{
}

void msvrContext::onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
}

void msvrContext::onDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
}

void msvrContext::onFlush()
{
}

BufferData* msvrContext::getActiveBuffer(GLenum target)
{
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
