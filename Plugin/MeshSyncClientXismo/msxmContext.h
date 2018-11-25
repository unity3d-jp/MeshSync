#pragma once

#include <GL/glew.h>
#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"

#define msxmMaxTextureSlots 32

using ms::float2;
using ms::float3;
using ms::float4;
using ms::quatf;
using ms::float4x4;

struct msxmSettings;
class msxmContext;

int msxmGetXismoVersion();
msxmContext* msxmGetContext();
void msxmInitializeWidget();


struct ms_vertex
{
    float3 vertex;
    float3 normal;
    float4 color;
    float2 uv;
};

struct xm_vertex1
{
    float3 vertex;
    float3 normal;
    float4 color;
    float2 uv;
    float state;

    bool operator==(const ms_vertex& v) const
    {
        return vertex == v.vertex && normal == v.normal && color == v.color && uv == v.uv;
    }
    operator ms_vertex() const
    {
        return { vertex, normal, color, uv };
    }
};

struct TextureRecord
{
    ms::TexturePtr dst;
    bool dirty = true;
    bool used = false;
};

struct MaterialRecord
{
    int id = ms::InvalidID;
    GLuint program = 0;
    float4 diffuse_color = float4::zero();
    float4 emission_color = float4::zero();
    int color_map = ms::InvalidID;
    int bump_map = ms::InvalidID;

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
    void        *mapped_data = nullptr;
    GLuint      handle = 0;
    int         num_elements = 0;
    int         stride = 0;
    bool        triangle = false;
    bool        dirty = false;
    int         material_id = ms::InvalidID;
    float4x4    transform = float4x4::identity();

    RawVector<xm_vertex1> vertices_tmp;
    RawVector<ms_vertex> vertices_welded;
    ms::MeshPtr dst_mesh;
    std::future<void> task;

    ~BufferRecord();
    bool isModelData() const;
    void startBuildMeshData(const msxmSettings& settings);
    void wait();
private:
    void buildMeshDataBody(const msxmSettings& settings);
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


struct msxmSettings
{
    ms::ClientSettings client_settings;
    float scale_factor = 100.0f;
    bool auto_sync = false;
    bool weld_vertices = true;
    bool make_double_sided = false;
    bool sync_delete = true;
    bool sync_camera = true;
    bool sync_textures = true;
    std::string camera_path = "/Main Camera";
};

class msxmContext
{
public:
    msxmContext();
    ~msxmContext();
    msxmSettings& getSettings();
    void send(bool force);

    void onGenTextures(GLsizei n, GLuint *textures);
    void onDeleteTextures(GLsizei n, const GLuint *textures);
    void onActiveTexture(GLenum texture);
    void onBindTexture(GLenum target, GLuint texture);
    void onTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);

    void onGenBuffers(GLsizei n, GLuint* buffers);
    void onDeleteBuffers(GLsizei n, const GLuint* buffers);
    void onBindBuffer(GLenum target, GLuint buffer);
    void onBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    void onMapBuffer(GLenum target, GLenum access, void *&mapped_data);
    void onUnmapBuffer(GLenum target);
    void onVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

    void onLinkProgram(GLuint program);
    void onDeleteProgram(GLuint program);
    void onUseProgram(GLuint program);
    void onUniform1i(GLint location, GLint v0);
    void onUniform4fv(GLint location, GLsizei count, const GLfloat* value);
    void onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

    void onDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
    void onFlush();

protected:
    BufferRecord* getActiveBuffer(GLenum target);
    ProgramRecord::Uniform* findUniform(GLint location);

protected:
    msxmSettings m_settings;

    std::map<uint32_t, BufferRecord> m_buffer_records;
    std::vector<GLuint> m_meshes_deleted;

    GLuint m_active_texture = 0;
    GLuint m_texture_slots[msxmMaxTextureSlots] = {};
    std::map<GLuint, TextureRecord> m_texture_records;

    GLuint m_program_handle = 0;
    std::map<GLuint, ProgramRecord> m_program_records;
    std::vector<MaterialRecord> m_material_records;

    uint32_t m_vertex_attributes = 0;
    uint32_t m_vb_handle = 0;
    float4x4 m_proj = float4x4::identity();
    float4x4 m_modelview = float4x4::identity();

    bool m_camera_dirty = false;
    float3 m_camera_pos = float3::zero();
    quatf m_camera_rot = quatf::identity();
    float m_camera_fov = 60.0f;
    float m_camera_near = 0.01f;
    float m_camera_far = 100.0f;

    std::vector<MaterialRecord> m_material_data;
    std::vector<BufferRecord*> m_mesh_buffers;

    ms::IDGenerator<MaterialRecord> m_material_ids;
    ms::CameraPtr m_camera;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;

    ms::AsyncSceneSender m_sender;
};
