#pragma once

#include <GL/glew.h>
#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"

using ms::float2;
using ms::float3;
using ms::float4;
using ms::quatf;
using ms::float2x2;
using ms::float3x3;
using ms::float4x4;

#define msvrMaxTextureSlots 32

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
    float4 diffuse_color = float4::zero();
    float4 specular_color = float4::zero();
    float bump_scale = 0.0f;

    int color_map = ms::InvalidID;
    int bump_map = ms::InvalidID;
    int specular_map = ms::InvalidID;

    float2 texture_scale = float2::one();
    float2 texture_offset = float2::zero();


    bool operator==(const MaterialRecord& v) const;
    bool operator!=(const MaterialRecord& v) const;
    uint64_t checksum() const;
};

namespace ms {

template<>
class IDGenerator<MaterialRecord> : public IDGenerator<void*>
{
public:
    int getID(const MaterialRecord& o)
    {
        auto ck = o.checksum();
        return getIDImpl((void*&)ck);
    }
};

} // namespace ms

struct BufferRecord : public mu::noncopyable
{
    RawVector<char> data, tmp_data;
    void *mapped_data = nullptr;
    int stride = 0;
    bool dirty = false;
    bool visible = true;

    bool valid_vertex_buffer = false;
    bool valid_index_buffer = false;
    int draw_count = 0;
};

struct ProgramRecord
{
    struct Uniform
    {
        std::string name;
        ms::MaterialProperty::Type type;
        int size;
    };
    std::map<GLuint, Uniform> uniforms;
    MaterialRecord mrec;
};

struct DrawcallRecord
{
    ms::MeshPtr mesh;
    int material_id = 0;

    GLuint vb = 0;
    GLuint ib = 0;
    int nth = 0;

    int hash() const;
};


struct msvrSettings
{
    ms::ClientSettings client_settings;
    float scale_factor = 100.0f;
    bool flip_u = false;
    bool flip_v = false;
    bool make_double_sided  = false;
    bool sync_delete = true;
    bool sync_textures = true;
    bool sync_camera = true;
    bool auto_sync = false;
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

    void onDeleteProgram(GLuint program);
    void onUseProgram(GLuint program);
    void onUniform1i(GLint location, GLint v0);
    void onUniform1f(GLint location, GLfloat v0);
    void onUniform2fv(GLint location, GLsizei count, const GLfloat *value);
    void onUniform3fv(GLint location, GLsizei count, const GLfloat *value);

    void onDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
    void onFlush();
    void onClear();


    void flipU(bool v);
    void flipV(bool v);
    void makeDoubleSided(bool v);

protected:
    BufferRecord* getActiveBuffer(GLenum target);
    ProgramRecord::Uniform* findUniform(GLint location);

protected:
    msvrSettings m_settings;

    std::map<GLuint, BufferRecord> m_buffer_records;
    std::vector<GLuint> m_meshes_deleted;

    GLuint m_vb_handle = 0;
    GLuint m_ib_handle = 0;
    GLuint m_ub_handle = 0;
    GLuint m_ub_handles[16] = {};

    int m_active_texture = 0;
    GLuint m_texture_slots[msvrMaxTextureSlots] = {};
    std::map<GLuint, TextureRecord> m_texture_records;

    GLuint m_fb_handle = 0;
    std::map<GLuint, FramebufferRecord> m_framebuffer_records;

    GLuint m_program_handle = 0;
    std::map<GLuint, ProgramRecord> m_program_records;
    std::vector<MaterialRecord> m_material_records;

    std::map<int, DrawcallRecord> m_drawcalls;

    bool m_camera_dirty = false;
    float3 m_camera_pos = float3::zero();
    quatf m_camera_rot = quatf::identity();
    float m_camera_fov = 60.0f;
    float m_camera_near = 0.01f;
    float m_camera_far = 100.0f;

    ms::IDGenerator<MaterialRecord> m_material_ids;
    ms::CameraPtr m_camera;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;

    ms::AsyncSceneSender m_sender;
};