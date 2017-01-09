#include "pch.h"
#include "giInternal.h"

#ifdef giSupportOpenGL
#if defined(_WIN32)
    #pragma comment(lib, "opengl32.lib")
    #define GLEW_STATIC
    #include <GL/glew.h>
#elif defined(__APPLE__)
    #include <GL/glew.h>
#else // linux
    #include <GL/glxew.h>
#endif

#ifdef _WIN32
    static PFNGLGENBUFFERSPROC      _glGenBuffers;
    static PFNGLDELETEBUFFERSPROC   _glDeleteBuffers;
    static PFNGLBINDBUFFERPROC      _glBindBuffer;
    static PFNGLBUFFERDATAPROC      _glBufferData;
    static PFNGLMAPBUFFERPROC       _glMapBuffer;
    static PFNGLUNMAPBUFFERPROC     _glUnmapBuffer;
#else
    #define _glGenBuffers    glGenBuffers
    #define _glDeleteBuffers glDeleteBuffers
    #define _glBindBuffer    glBindBuffer
    #define _glBufferData    glBufferData
    #define _glMapBuffer     glMapBuffer
    #define _glUnmapBuffer   glUnmapBuffer
#endif // _WIN32

static void InitializeOpenGL()
{
#ifdef _WIN32
#define Import(name) (void*&)_##name = (void*)wglGetProcAddress(#name)
    Import(glGenBuffers);
    Import(glDeleteBuffers);
    Import(glBindBuffer);
    Import(glBufferData);
    Import(glMapBuffer);
    Import(glUnmapBuffer);
#undef Import
#else
    glewInit();
#endif // _WIN32
}

namespace gi {

class GraphicsInterfaceOpenGL : public GraphicsInterface
{
public:
    GraphicsInterfaceOpenGL(void *device);
    ~GraphicsInterfaceOpenGL() override;
    void release() override;

    void* getDevicePtr() override;
    DeviceType getDeviceType() override;
    void sync() override;

    Result createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags flags) override;
    void   releaseTexture2D(void *tex) override;
    Result readTexture2D(void *dst, size_t read_size, void *src_tex, int width, int height, TextureFormat format) override;
    Result writeTexture2D(void *dst_tex, int width, int height, TextureFormat format, const void *src, size_t write_size) override;

    Result createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags) override;
    void   releaseBuffer(void *buf) override;
    Result mapBuffer(MapContext& ctx) override;
    Result unmapBuffer(MapContext& ctx) override;
};


GraphicsInterface* CreateGraphicsInterfaceOpenGL(void *device)
{
    InitializeOpenGL();
    return new GraphicsInterfaceOpenGL(device);
}


void* GraphicsInterfaceOpenGL::getDevicePtr() { return nullptr; }
DeviceType GraphicsInterfaceOpenGL::getDeviceType() { return DeviceType::OpenGL; }

GraphicsInterfaceOpenGL::GraphicsInterfaceOpenGL(void * /*device*/)
{
}

GraphicsInterfaceOpenGL::~GraphicsInterfaceOpenGL()
{
}

void GraphicsInterfaceOpenGL::release()
{
    delete this;
}


static void GetGLTextureType(TextureFormat format, GLenum &glfmt, GLenum &gltype, GLenum &glifmt)
{
    switch (format)
    {
    case TextureFormat::Ru8:     glfmt = GL_RED;  gltype = GL_UNSIGNED_BYTE; glifmt = GL_R8; return;
    case TextureFormat::RGu8:    glfmt = GL_RG;   gltype = GL_UNSIGNED_BYTE; glifmt = GL_RG8; return;
    case TextureFormat::RGBAu8:  glfmt = GL_RGBA; gltype = GL_UNSIGNED_BYTE; glifmt = GL_RGBA8; return;

    case TextureFormat::Rf16:    glfmt = GL_RED;  gltype = GL_HALF_FLOAT; glifmt = GL_R16F; return;
    case TextureFormat::RGf16:   glfmt = GL_RG;   gltype = GL_HALF_FLOAT; glifmt = GL_RG16F; return;
    case TextureFormat::RGBAf16: glfmt = GL_RGBA; gltype = GL_HALF_FLOAT; glifmt = GL_RGBA16F; return;

    case TextureFormat::Ri16:    glfmt = GL_RED;  gltype = GL_SHORT; glifmt = GL_R16_SNORM; return;
    case TextureFormat::RGi16:   glfmt = GL_RG;   gltype = GL_SHORT; glifmt = GL_RG16_SNORM; return;
    case TextureFormat::RGBAi16: glfmt = GL_RGBA; gltype = GL_SHORT; glifmt = GL_RGBA16_SNORM; return;

    case TextureFormat::Rf32:    glfmt = GL_RED;  gltype = GL_FLOAT; glifmt = GL_R32F; return;
    case TextureFormat::RGf32:   glfmt = GL_RG;   gltype = GL_FLOAT; glifmt = GL_RG32F; return;
    case TextureFormat::RGBAf32: glfmt = GL_RGBA; gltype = GL_FLOAT; glifmt = GL_RGBA32F; return;

    case TextureFormat::Ri32:    glfmt = GL_RED_INTEGER;   gltype = GL_INT;  glifmt = GL_R32I; return;
    case TextureFormat::RGi32:   glfmt = GL_RG_INTEGER;    gltype = GL_INT;  glifmt = GL_RG32I; return;
    case TextureFormat::RGBAi32: glfmt = GL_RGBA_INTEGER;  gltype = GL_INT;  glifmt = GL_RGBA32I; return;
    default: break;
    }
}

static Result GetGLError()
{
    auto e = glGetError();
    switch (e) {
    case GL_NO_ERROR: return Result::OK;
    case GL_OUT_OF_MEMORY: return Result::OutOfMemory;
    case GL_INVALID_ENUM: return Result::InvalidParameter;
    case GL_INVALID_VALUE: return Result::InvalidParameter;
    case GL_INVALID_OPERATION: return Result::InvalidOperation;
    }
    return Result::Unknown;
}

void GraphicsInterfaceOpenGL::sync()
{
    glFinish();
}

Result GraphicsInterfaceOpenGL::createTexture2D(void **dst_tex, int width, int height, TextureFormat format, const void *data, ResourceFlags /*flags*/)
{
    GLenum gl_format = 0;
    GLenum gl_type = 0;
    GLenum gl_iformat = 0;
    GetGLTextureType(format, gl_format, gl_type, gl_iformat);

    auto ret = Result::OK;
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, gl_iformat, width, height, 0, gl_format, gl_type, data);
    ret = GetGLError();
    glBindTexture(GL_TEXTURE_2D, 0);
    *(GLuint*)dst_tex = tex;

    return ret;
}

void GraphicsInterfaceOpenGL::releaseTexture2D(void *tex_)
{
    GLuint tex = (GLuint)(size_t)tex_;
    glDeleteTextures(1, &tex);

}

Result GraphicsInterfaceOpenGL::readTexture2D(void *dst, size_t /*read_size*/, void *src_tex, int /*width*/, int /*height*/, TextureFormat format)
{
    GLenum gl_format = 0;
    GLenum gl_type = 0;
    GLenum gl_iformat = 0;
    GetGLTextureType(format, gl_format, gl_type, gl_iformat);

    // available OpenGL 4.5 or later
    // glGetTextureImage((GLuint)(size_t)tex, 0, internal_format, internal_type, bufsize, o_buf);

    sync();

    auto ret = Result::OK;
    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)src_tex);
    glGetTexImage(GL_TEXTURE_2D, 0, gl_format, gl_type, dst);
    ret = GetGLError();
    glBindTexture(GL_TEXTURE_2D, 0);
    return ret;
}

Result GraphicsInterfaceOpenGL::writeTexture2D(void *dst_tex, int width, int height, TextureFormat format, const void *src, size_t /*write_size*/)
{
    GLenum gl_format = 0;
    GLenum gl_type = 0;
    GLenum gl_iformat = 0;
    GetGLTextureType(format, gl_format, gl_type, gl_iformat);

    // available OpenGL 4.5 or later
    // glTextureSubImage2D((GLuint)(size_t)o_tex, 0, 0, 0, width, height, internal_format, internal_type, buf);

    auto ret = Result::OK;
    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)dst_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, gl_format, gl_type, src);
    ret = GetGLError();
    glBindTexture(GL_TEXTURE_2D, 0);
    return ret;
}


static GLenum GetGLBufferType(BufferType type)
{
    GLenum gltype = 0;
    switch (type) {
    case BufferType::Index:
        gltype = GL_ELEMENT_ARRAY_BUFFER;
        break;
    case BufferType::Vertex:
        gltype = GL_ARRAY_BUFFER;
        break;
    case BufferType::Constant:
        gltype = GL_UNIFORM_BUFFER;
        break;
    case BufferType::Compute:
        gltype = GL_SHADER_STORAGE_BUFFER;
        break;
    default:
        break;
    }
    return gltype;
}

Result GraphicsInterfaceOpenGL::createBuffer(void **dst_buf, size_t size, BufferType type, const void *data, ResourceFlags flags)
{
    GLenum gltype = GetGLBufferType(type);
    GLenum glusage = GL_STATIC_DRAW;
    if (flags & ResourceFlags::CPU_Write) {
        glusage = GL_DYNAMIC_DRAW;
    }
    if (flags & ResourceFlags::CPU_Read) {
        glusage = GL_STREAM_DRAW;
    }

    auto ret = Result::OK;
    GLuint buf = 0;
    _glGenBuffers(1, &buf);
    _glBindBuffer(gltype, buf);
    _glBufferData(gltype, size, data, glusage);
    ret = GetGLError();
    _glBindBuffer(gltype, 0);

    *(GLuint*)dst_buf = buf;
    return ret;
}

void GraphicsInterfaceOpenGL::releaseBuffer(void *buf_)
{
    GLuint buf = (GLuint)(size_t)buf_;
    _glDeleteBuffers(1, &buf);
}

Result GraphicsInterfaceOpenGL::mapBuffer(MapContext& ctx)
{
    if (!ctx.resource) { return Result::InvalidParameter; }

    GLuint buf = (GLuint)(size_t)ctx.resource;
    GLenum gltype = GetGLBufferType(ctx.type);

    Result ret = Result::OK;
    _glBindBuffer(gltype, buf);
    switch (ctx.mode) {
    case MapMode::Read: ctx.data_ptr = _glMapBuffer(gltype, GL_READ_ONLY); break;
    case MapMode::Write: ctx.data_ptr = _glMapBuffer(gltype, GL_WRITE_ONLY); break;
    default: ret = Result::InvalidParameter; break;
    }

    if (!ctx.data_ptr) {
        ret = GetGLError();
    }
    return ret;
}

Result GraphicsInterfaceOpenGL::unmapBuffer(MapContext& ctx)
{
    if (!ctx.resource || !ctx.data_ptr) { return Result::InvalidParameter; }

    GLuint buf = (GLuint)(size_t)ctx.resource;
    GLenum gltype = GetGLBufferType(ctx.type);

    Result ret = Result::OK;
    _glBindBuffer(gltype, buf);
    _glUnmapBuffer(gltype);
    ctx.data_ptr = nullptr;
    return ret;
}

} // namespace gi
#endif // giSupportOpenGL
