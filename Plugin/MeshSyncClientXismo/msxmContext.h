#pragma once

#include <GL/glew.h>
#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"

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

struct MaterialRecord
{
    GLuint program = 0;
    GLuint texture = 0;
    float4 diffuse = float4::zero();

    bool operator==(const MaterialRecord& v) const
    {
        return program == v.program && texture == v.texture && diffuse == v.diffuse;
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
    GLuint      handle = 0;
    int         num_elements = 0;
    int         stride = 0;
    bool        triangle = false;
    bool        dirty = false;
    bool        visible = true;
    int         material_id = -1;
    MaterialRecord material;
    float4x4    transform = float4x4::identity();

    RawVector<ms_vertex> vertices_welded;
    ms::MeshPtr dst_mesh;

    bool isModelData() const;
    void buildMeshData(const msxmSettings& settings);
};


struct msxmSettings
{
    ms::ClientSettings client_settings;
    float scale_factor = 100.0f;
    bool auto_sync = true;
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

    void onActiveTexture(GLenum texture);
    void onBindTexture(GLenum target, GLuint texture);
    void onGenBuffers(GLsizei n, GLuint* buffers);
    void onDeleteBuffers(GLsizei n, const GLuint* buffers);
    void onBindBuffer(GLenum target, GLuint buffer);
    void onBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
    void onMapBuffer(GLenum target, GLenum access, void *&mapped_data);
    void onUnmapBuffer(GLenum target);
    void onVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
    void onUniform4fv(GLint location, GLsizei count, const GLfloat* value);
    void onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
    void onDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
    void onFlush();

protected:
    BufferRecord* getActiveBuffer(GLenum target);

protected:
    msxmSettings m_settings;

    std::map<uint32_t, BufferRecord> m_buffer_records;
    std::vector<GLuint> m_meshes_deleted;
    GLuint m_texture_slot = 0;

    uint32_t m_vertex_attributes = 0;
    uint32_t m_vb_handle = 0;
    MaterialRecord m_material_records;
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

    ms::CameraPtr m_camera;
    ms::TextureManager m_texture_manager;
    ms::MaterialManager m_material_manager;
    ms::EntityManager m_entity_manager;

    ms::AsyncSceneSender m_sender;
};
