#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncClientXismo.h"
using namespace mu;


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


struct MaterialData
{
    GLuint program = 0;
    float4 difuse = float4::one();

    bool operator==(const MaterialData& v) const
    {
        return program == v.program && difuse == v.difuse;
    }
    bool operator!=(const MaterialData& v) const
    {
        return !operator==(v);
    }
};


struct VertexData
{
    RawVector<char> data;
    void        *mapped_data = nullptr;
    GLuint      handle = 0;
    int         num_elements = 0;
    int         stride = 0;
    bool        triangle = false;
    bool        dirty = false;
    bool        visible = true;
    MaterialData material;
    float4x4    transform = float4x4::identity();

    struct SendTaskData
    {
        RawVector<char>     data;
        GLuint              handle = 0;
        int                 num_elements = 0;
        bool                visible = true;
        int                 material_id = 0;
        float4x4            transform = float4x4::identity();
        RawVector<ms_vertex> vertices_welded;
        ms::MeshPtr         ms_mesh;

        void buildMeshData(bool weld_vertices);
    };
    using SendTaskPtr = std::shared_ptr<SendTaskData>;
    SendTaskPtr send_data;

    void updateSendData();
};


class msxmContext : public msxmIContext
{
public:
    msxmContext();
    ~msxmContext() override;
    msxmSettings& getSettings() override;
    void send(bool force) override;

    void onGenBuffers(GLsizei n, GLuint* buffers) override;
    void onDeleteBuffers(GLsizei n, const GLuint* buffers) override;
    void onBindBuffer(GLenum target, GLuint buffer) override;
    void onBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) override;
    void onMapBuffer(GLenum target, GLenum access, void *mapped_data) override;
    void onUnmapBuffer(GLenum target) override;
    void onVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) override;
    void onUniform4fv(GLint location, GLsizei count, const GLfloat* value) override;
    void onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
    void onDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices) override;
    void onFlush() override;

protected:
    VertexData* getActiveBuffer(GLenum target);

protected:
    msxmSettings m_settings;

    std::map<uint32_t, VertexData> m_buffers;
    std::future<void> m_send_future;

    std::vector<GLuint> m_meshes_deleted;
    uint32_t m_vertex_attributes = 0;
    uint32_t m_vb_handle = 0;
    MaterialData m_material;
    float4x4 m_proj = float4x4::identity();
    float4x4 m_modelview = float4x4::identity();
    float4x4 m_rotation = float4x4::identity();

    bool m_camera_dirty = false;
    float3 m_camera_pos = float3::zero();
    quatf m_camera_rot = quatf::identity();
    float m_camera_fov = 60.0f;

    struct SendTaskData
    {
        msxmSettings settings;
        ms::SetMessage message;
        std::vector<MaterialData> materials;
        std::vector<VertexData::SendTaskPtr> meshes;
        std::vector<GLuint> meshes_deleted;

        void send();
    };
    SendTaskData m_send_data;
};


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

msxmIContext* msxmGetContext()
{
    static std::unique_ptr<msxmIContext> s_ctx;
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

void VertexData::SendTaskData::buildMeshData(bool weld_vertices)
{
    if (!data.data()) { return; }
    int num_indices = num_elements;
    int num_triangles = num_indices / 3;

    if (!ms_mesh) {
        ms_mesh.reset(new ms::Mesh());

        char path[128];
        sprintf(path, "/XismoMesh:ID[%08x]", handle);
        ms_mesh->path = path;
        ms_mesh->id = handle;
    }
    auto& mesh = *ms_mesh;
    mesh.visible = visible;
    if (!visible) {
        return;
    }

    mesh.flags.has_points = 1;
    mesh.flags.has_normals = 1;
    mesh.flags.has_uv = 1;
    mesh.flags.has_counts = 1;
    mesh.flags.has_indices = 1;
    mesh.flags.has_materialIDs = 1;
    mesh.flags.has_refine_settings = 1;
    mesh.refine_settings.flags.swap_faces = true;

    auto vertices = (const xm_vertex1*)data.data();
    if (weld_vertices) {
        Weld(vertices, num_indices, vertices_welded, mesh.indices);

        int num_vertices = (int)vertices_welded.size();
        mesh.points.resize_discard(num_vertices);
        mesh.normals.resize_discard(num_vertices);
        mesh.uv.resize_discard(num_vertices);
        mesh.colors.resize_discard(num_vertices);

        for (int vi = 0; vi < num_vertices; ++vi) {
            auto& v = vertices_welded[vi];
            mesh.points[vi] = v.vertex;
            mesh.normals[vi] = v.normal;
            mesh.uv[vi] = v.uv;
            mesh.colors[vi] = v.color;
        }
    }
    else {
        int num_vertices = num_indices;
        mesh.points.resize_discard(num_vertices);
        mesh.normals.resize_discard(num_vertices);
        mesh.uv.resize_discard(num_vertices);
        mesh.colors.resize_discard(num_vertices);
        mesh.indices.resize_discard(num_vertices);

        for (int vi = 0; vi < num_vertices; ++vi) {
            mesh.points[vi] = vertices[vi].vertex;
            mesh.normals[vi] = vertices[vi].normal;
            mesh.uv[vi] = vertices[vi].uv;
            mesh.colors[vi] = vertices[vi].color;
            mesh.indices[vi] = vi;
        }
    }

    mesh.counts.resize_discard(num_triangles);
    mesh.materialIDs.resize_discard(num_triangles);
    for (int ti = 0; ti < num_triangles; ++ti) {
        mesh.counts[ti] = 3;
        mesh.materialIDs[ti] = material_id;
    }
}

void VertexData::updateSendData()
{
    if (!send_data) {
        send_data.reset(new SendTaskData());
    }
    auto& dst = *send_data;
    dst.data = data;
    dst.handle = handle;
    dst.num_elements = num_elements;
    dst.transform = transform;
    dst.visible = visible;
    dirty = false;
}



msxmContext::msxmContext()
{
}

msxmContext::~msxmContext()
{
    if (m_send_future.valid()) {
        m_send_future.wait();
    }
}

msxmSettings& msxmContext::getSettings()
{
    return m_settings;
}

void msxmContext::send(bool force)
{
    if (m_send_future.valid() && m_send_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
    {
        // previous request is not completed yet
        return;
    }

    auto& scene = m_send_data.message.scene;

    parallel_invoke([&]() {
        // build mesh data to send
        std::vector<VertexData*> buffers_to_send;
        for (auto& pair : m_buffers) {
            auto& buf = pair.second;
            if (buf.stride == sizeof(xm_vertex1) && buf.triangle && (buf.dirty || force)) {
                buffers_to_send.push_back(&buf);
            }
        }
        if (buffers_to_send.empty() && m_meshes_deleted.empty() && (!m_settings.sync_camera || !m_camera_dirty)) {
            // nothing to send
            return;
        }

        parallel_for_each(buffers_to_send.begin(), buffers_to_send.end(), [](VertexData *buf) {
            buf->updateSendData();
        });
        m_send_data.meshes.resize(buffers_to_send.size());
        for (size_t i = 0; i < buffers_to_send.size(); ++i) {
            m_send_data.meshes[i] = buffers_to_send[i]->send_data;
        }
    },
    [&]() {
        // build material list
        {
            m_send_data.materials.clear();
            auto findOrAddMaterial = [this](const MaterialData& md) {
                auto it = std::find(m_send_data.materials.begin(), m_send_data.materials.end(), md);
                if (it != m_send_data.materials.end()) {
                    return (int)std::distance(m_send_data.materials.begin(), it);
                }
                else {
                    int ret = (int)m_send_data.materials.size();
                    m_send_data.materials.push_back(md);
                    return ret;
                }
            };

            for (auto& pair : m_buffers) {
                auto& buf = pair.second;
                if (buf.send_data) {
                    buf.send_data->material_id = findOrAddMaterial(buf.material);
                }
            }

            scene.materials.resize(m_send_data.materials.size());
            for (int i = 0; i < (int)scene.materials.size(); ++i) {
                auto& mat = scene.materials[i];
                if (!mat) mat.reset(new ms::Material());

                char name[128];
                sprintf(name, "XismoMaterial:ID[%04x]", i);
                mat->id = i;
                mat->name = name;
                mat->color = m_send_data.materials[i].difuse;
            }
        }

        // camera
        if (m_settings.sync_camera) {
            if (scene.cameras.empty()) {
                auto c = new ms::Camera();
                c->path = "/Main Camera";
                scene.cameras.emplace_back(c);
            }
            auto& cam = *scene.cameras.back();
            cam.transform.position = m_camera_pos;
            cam.transform.rotation = m_camera_rot;
            cam.fov = m_camera_fov;
            m_camera_dirty = false;
        }
        else {
            scene.cameras.clear();
        }

        m_send_data.meshes_deleted.swap(m_meshes_deleted);
        m_meshes_deleted.clear();
        m_send_data.settings = m_settings;
    });

    // kick async send
    m_send_future = std::async(std::launch::async, [this]() { m_send_data.send(); });
}

void msxmContext::SendTaskData::send()
{
    ms::Client client(settings.client_settings);

    ms::SceneSettings scene_settings;
    scene_settings.handedness = ms::Handedness::Left;
    scene_settings.scale_factor = settings.scale_factor;


    // notify scene begin
    {
        ms::FenceMessage fence;
        fence.type = ms::FenceMessage::FenceType::SceneBegin;
        client.send(fence);
    }

    // send camera & materials
    message.scene.settings = scene_settings;
    client.send(message);

    // send deleted
    if (!meshes_deleted.empty()) {
        ms::DeleteMessage del;
        for (auto h : meshes_deleted) {
            auto it = std::find_if(meshes.begin(), meshes.end(), [h](VertexData::SendTaskPtr& v) {
                return v->handle == h;
            });
            if (it == meshes.end()) {
                char path[128];
                sprintf(path, "/XismoMesh:ID[%08x]", h);
                del.targets.push_back({ path, (int)h });
            }
        }
        if (!del.targets.empty()) {
            client.send(del);
        }
    }

    // send meshes
    parallel_for_each(meshes.begin(), meshes.end(), [&](VertexData::SendTaskPtr& v) {
        v->buildMeshData(settings.weld_vertices);

        ms::SetMessage set;
        set.scene.settings = scene_settings;
        set.scene.meshes = { v->ms_mesh };
        client.send(set);
    });
    meshes.clear();

    // notify scene end
    {
        ms::FenceMessage fence;
        fence.type = ms::FenceMessage::FenceType::SceneEnd;
        client.send(fence);
    }
}


VertexData* msxmContext::getActiveBuffer(GLenum target)
{
    uint32_t bid = 0;
    if (target == GL_ARRAY_BUFFER) {
        bid = m_vb_handle;
    }
    return bid != 0 ? &m_buffers[bid] : nullptr;
}


void msxmContext::onGenBuffers(GLsizei n, GLuint * handles)
{
}

void msxmContext::onDeleteBuffers(GLsizei n, const GLuint * handles)
{
    for (int i = 0; i < n; ++i) {
        auto it = m_buffers.find(handles[i]);
        if (it != m_buffers.end()) {
            if (it->second.send_data) {
                m_meshes_deleted.push_back(it->second.handle);
            }
            m_buffers.erase(it);
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

void msxmContext::onMapBuffer(GLenum target, GLenum access, void *mapped_data)
{
    if (auto *buf = getActiveBuffer(target)) {
        buf->mapped_data = mapped_data;
    }
}

void msxmContext::onUnmapBuffer(GLenum target)
{
    if (auto *buf = getActiveBuffer(target)) {
        if (buf->mapped_data) {
            memcpy(buf->data.data(), buf->mapped_data, buf->data.size());
            buf->mapped_data = nullptr;
            buf->dirty = true;
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
        m_material.difuse.assign(value);
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
        buf->triangle = true;
        buf->num_elements = (int)count;
        if (buf->material != m_material) {
            buf->material = m_material;
            buf->dirty = true;
        }
        {
            // modelview matrix -> camera pos & rot
            float3 pos, forward, up, right;
            view_to_camera(m_modelview, pos, forward, up, right);
            quatf rot = to_quat(look33(forward, float3{ 0.0f, 1.0f, 0.0f }));
            if (pos != m_camera_pos ||
                rot != m_camera_rot)
            {
                m_camera_dirty = true;
                m_camera_pos = pos;
                m_camera_rot = rot;
            }
        }
        {
            // projection matrix -> camera fov
            float thf = 1.0f / m_proj[1][1];
            float fov = std::atan(thf) *  Rad2Deg;
            if (fov != m_camera_fov) {
                m_camera_dirty = true;
                m_camera_fov = fov;
            }
        }
    }

    m_vertex_attributes = 0;
}

void msxmContext::onFlush()
{
    if (m_settings.auto_sync) {
        send(false);
    }
}

