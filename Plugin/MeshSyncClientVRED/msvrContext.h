#pragma once

#include <GL/glew.h>
#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"

using ms::float2;
using ms::float3;
using ms::float4;
using ms::quatf;
using ms::float4x4;

class msvrContext;

msvrContext* msvrGetContext();
void msvrInitializeWidget();


struct vr_vertex
{
    float3 vertex;
    float3 normal;
    float2 uv;
    float4 color;
};

struct TextureRecord
{
    ms::TexturePtr dst;
    bool dirty = true;
    bool used = false;
};

struct FramebufferRecord
{
    GLuint colors[16] = {};
    GLuint depth_stencil = 0;

    bool isMainTarget() const;
};

struct MaterialRecord
{
    int id = ms::InvalidID;
    GLuint program = 0;
    float4 diffuse = float4::one();
    int color_tex = ms::InvalidID;
    int normal_tex = ms::InvalidID;

    bool operator==(const MaterialRecord& v) const
    {
        return memcmp(this, &v, sizeof(MaterialRecord));
    }
    bool operator!=(const MaterialRecord& v) const
    {
        return !operator==(v);
    }
};

struct BufferRecord : public mu::noncopyable
{
    RawVector<char> data, tmp_data;
    void        *mapped_data = nullptr;
    int         stride = 0;
    bool        dirty = false;
    bool        visible = true;
    int         material_id = -1;

    ms::MeshPtr dst_mesh;
};

struct ProgramRecord
{
    GLuint program = 0;
    std::map<GLuint, ms::MaterialProperty> uniforms;
};



struct msvrSettings
{
    ms::ClientSettings client_settings;
    float scale_factor = 100.0f;
    bool auto_sync = false;
    bool sync_delete = true;
    bool sync_camera = true;
    bool sync_textures = true;
    std::string camera_path = "/Main Camera";
};

class msvrContext
{
public:
    msvrContext();
    ~msvrContext();
    msvrSettings& getSettings();
    void send(bool force);

    void onGenTextures(GLsizei n, GLuint * textures);
    void onDeleteTextures(GLsizei n, const GLuint * textures);
    void onActiveTexture(GLenum texture);
    void onBindTexture(GLenum target, GLuint texture);
    void onTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * data);

    void onGenFramebuffers(GLsizei n, GLuint *ids);
    void onBindFramebuffer(GLenum target, GLuint framebuffer);
    void onDeleteFramebuffers(GLsizei n, GLuint *framebuffers);
    void onFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
    void onFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

    void onGenBuffers(GLsizei n, GLuint* buffers);
    void onDeleteBuffers(GLsizei n, const GLuint* buffers);
    void onBindBuffer(GLenum target, GLuint buffer);
    void onBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    void onBindBufferBase(GLenum target, GLuint index, GLuint buffer);
    void onBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    void onNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, const void *data);
    void onMapBuffer(GLenum target, GLenum access, void *&mapped_data);
    void onMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access, void *&mapped_data);
    void onUnmapBuffer(GLenum target);
    void onFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);

    void onGenVertexArrays(GLsizei n, GLuint *buffers);
    void onDeleteVertexArrays(GLsizei n, const GLuint *buffers);
    void onBindVertexArray(GLuint buffer);
    void onEnableVertexAttribArray(GLuint index);
    void onDisableVertexAttribArray(GLuint index);
    void onVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

    void onLinkProgram(GLuint program);
    void onDeleteProgram(GLuint program);
    void onUseProgram(GLuint program);
    void onUniform1fv(GLint location, GLsizei count, const GLfloat *value);
    void onUniform2fv(GLint location, GLsizei count, const GLfloat *value);
    void onUniform3fv(GLint location, GLsizei count, const GLfloat *value);
    void onUniform4fv(GLint location, GLsizei count, const GLfloat *value);
    void onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

    void onDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
    void onFlush();

protected:
    BufferRecord* getActiveBuffer(GLenum target);

protected:
    msvrSettings m_settings;

    std::map<GLuint, BufferRecord> m_buffer_records;
    std::vector<GLuint> m_meshes_deleted;

    GLuint m_vb_handle = 0;
    GLuint m_ib_handle = 0;
    GLuint m_ub_handle = 0;
    GLuint m_ub_handles[16] = {};

    int m_active_texture = 0;
    GLuint m_texture_slots[32] = {};
    std::map<GLuint, TextureRecord> m_texture_records;

    GLuint m_fb_handle = 0;
    std::map<GLuint, FramebufferRecord> m_framebuffer_records;

    GLuint m_program_handle = 0;
    std::map<GLuint, ProgramRecord> m_program_records;
    std::vector<MaterialRecord> m_material_records;

    bool m_camera_dirty = false;
    float3 m_camera_pos = float3::zero();
    quatf m_camera_rot = quatf::identity();
    float m_camera_fov = 60.0f;
    float m_camera_near = 0.01f;
    float m_camera_far = 100.0f;

    ms::CameraPtr m_camera;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;

    ms::AsyncSceneSender m_sender;
};