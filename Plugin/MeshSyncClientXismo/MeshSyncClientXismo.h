#pragma once

struct msxmSettings
{
    ms::ClientSettings client_settings;
    bool auto_sync = true;
    bool weld_vertices = true;
    bool sync_camera = false;
    float scale_factor = 100.0f;
};

class msxmIContext
{
public:
    virtual ~msxmIContext() {}
    virtual msxmSettings& getSettings() = 0;
    virtual void send(bool force = false) = 0;

    virtual void onGenBuffers(GLsizei n, GLuint* buffers) = 0;
    virtual void onDeleteBuffers(GLsizei n, const GLuint* buffers) = 0;
    virtual void onBindBuffer(GLenum target, GLuint buffer) = 0;
    virtual void onBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) = 0;
    virtual void onMapBuffer(GLenum target, GLenum access, void *mapped_data) = 0;
    virtual void onUnmapBuffer(GLenum target) = 0;
    virtual void onVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) = 0;
    virtual void onUniform4fv(GLint location, GLsizei count, const GLfloat* value) = 0;
    virtual void onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = 0;
    virtual void onDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices) = 0;
    virtual void onFlush() = 0;
};

int msxmGetAppVersion();
msxmIContext* msxmGetContext();
void msxmInitializeWidget();
