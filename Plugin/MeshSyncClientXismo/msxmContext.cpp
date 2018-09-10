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
    GLuint texture = 0;
    float4 diffuse = float4::zero();

    bool operator==(const MaterialData& v) const
    {
        return program == v.program && texture == v.texture && diffuse == v.diffuse;
    }
    bool operator!=(const MaterialData& v) const
    {
        return !operator==(v);
    }
};


struct BufferData : public mu::noncopyable
{
    RawVector<char> data, tmp_data;
    void        *mapped_data;
    GLuint      handle = 0;
    int         num_elements = 0;
    int         stride = 0;
    bool        triangle = false;
    bool        dirty = false;
    bool        visible = true;
    int         material_id = -1;
    MaterialData material;
    float4x4    transform = float4x4::identity();

    struct SendTaskData : public mu::noncopyable
    {
        RawVector<char>     data;
        GLuint              handle = 0;
        int                 num_elements = 0;
        bool                visible = true;
        int                 material_id = -1;
        float4x4            transform = float4x4::identity();
        RawVector<ms_vertex> vertices_welded;
        ms::MeshPtr         dst_mesh;

        void buildMeshData(bool weld_vertices);
    };
    using SendTaskPtr = std::shared_ptr<SendTaskData>;
    SendTaskPtr send_data;

    bool isModelData() const;
    void updateSendData();
};


class msxmContext : public msxmIContext
{
public:
    msxmContext();
    ~msxmContext() override;
    msxmSettings& getSettings() override;
    void send(bool force) override;

    void onActiveTexture(GLenum texture) override;
    void onBindTexture(GLenum target, GLuint texture) override;
    void onGenBuffers(GLsizei n, GLuint* buffers) override;
    void onDeleteBuffers(GLsizei n, const GLuint* buffers) override;
    void onBindBuffer(GLenum target, GLuint buffer) override;
    void onBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) override;
    void onMapBuffer(GLenum target, GLenum access, void *&mapped_data) override;
    void onUnmapBuffer(GLenum target) override;
    void onVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) override;
    void onUniform4fv(GLint location, GLsizei count, const GLfloat* value) override;
    void onUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) override;
    void onDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices) override;
    void onFlush() override;

protected:
    BufferData* getActiveBuffer(GLenum target);

protected:
    msxmSettings m_settings;

    std::map<uint32_t, BufferData> m_buffers;
    std::future<void> m_send_future;

    std::vector<GLuint> m_meshes_deleted;
    GLuint m_texture_slot = 0;

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
    float m_camera_near = 0.01f;
    float m_camera_far = 100.0f;

    std::vector<MaterialData> m_material_data;
    std::vector<BufferData::SendTaskPtr> m_mesh_data;
    std::vector<GLuint> m_deleted;

    ms::CameraPtr m_camera;
    std::vector<ms::MaterialPtr> m_materials;
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

void BufferData::SendTaskData::buildMeshData(bool weld_vertices)
{
    if (!data.data()) { return; }
    int num_indices = num_elements;
    int num_triangles = num_indices / 3;

    if (!dst_mesh) {
        dst_mesh = ms::Mesh::create();

        char path[128];
        sprintf(path, "/XismoMesh:ID[%08x]", handle);
        dst_mesh->path = path;
    }
    auto& mesh = *dst_mesh;
    mesh.visible = visible;
    if (!visible) {
        return;
    }

    mesh.flags.has_points = 1;
    mesh.flags.has_normals = 1;
    mesh.flags.has_uv0 = 1;
    mesh.flags.has_counts = 1;
    mesh.flags.has_indices = 1;
    mesh.flags.has_material_ids = 1;
    mesh.flags.has_refine_settings = 1;
    mesh.refine_settings.flags.swap_faces = true;
    mesh.refine_settings.flags.gen_tangents = 1;
    mesh.refine_settings.flags.invert_v = 1;

    auto vertices = (const xm_vertex1*)data.data();
    if (weld_vertices) {
        Weld(vertices, num_indices, vertices_welded, mesh.indices);

        int num_vertices = (int)vertices_welded.size();
        mesh.points.resize_discard(num_vertices);
        mesh.normals.resize_discard(num_vertices);
        mesh.uv0.resize_discard(num_vertices);
        mesh.colors.resize_discard(num_vertices);

        for (int vi = 0; vi < num_vertices; ++vi) {
            auto& v = vertices_welded[vi];
            mesh.points[vi] = v.vertex;
            mesh.normals[vi] = v.normal;
            mesh.uv0[vi] = v.uv;
            mesh.colors[vi] = v.color;
        }
    }
    else {
        int num_vertices = num_indices;
        mesh.points.resize_discard(num_vertices);
        mesh.normals.resize_discard(num_vertices);
        mesh.uv0.resize_discard(num_vertices);
        mesh.colors.resize_discard(num_vertices);
        mesh.indices.resize_discard(num_vertices);

        for (int vi = 0; vi < num_vertices; ++vi) {
            mesh.points[vi] = vertices[vi].vertex;
            mesh.normals[vi] = vertices[vi].normal;
            mesh.uv0[vi] = vertices[vi].uv;
            mesh.colors[vi] = vertices[vi].color;
            mesh.indices[vi] = vi;
        }
    }

    mesh.counts.resize_discard(num_triangles);
    mesh.material_ids.resize_discard(num_triangles);
    for (int ti = 0; ti < num_triangles; ++ti) {
        mesh.counts[ti] = 3;
        mesh.material_ids[ti] = material_id;
    }
}

bool BufferData::isModelData() const
{
    return stride == sizeof(xm_vertex1) && triangle;
}

void BufferData::updateSendData()
{
    if (!send_data) {
        send_data.reset(new SendTaskData());
    }
    auto& dst = *send_data;
    dst.data = data;
    dst.handle = handle;
    dst.num_elements = num_elements;
    dst.material_id = material_id;
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

    std::vector<BufferData*> buffers_to_send;
    for (auto& pair : m_buffers) {
        auto& buf = pair.second;
        if (buf.isModelData() && (buf.dirty || force)) {
            buffers_to_send.push_back(&buf);
        }
    }
    if (buffers_to_send.empty() && m_meshes_deleted.empty() && (!m_settings.sync_camera || !m_camera_dirty)) {
        // nothing to send
        return;
    }

    // build material list
    {
        m_material_data.clear();
        auto findOrAddMaterial = [this](const MaterialData& md) {
            auto it = std::find(m_material_data.begin(), m_material_data.end(), md);
            if (it != m_material_data.end()) {
                return (int)std::distance(m_material_data.begin(), it);
            }
            else {
                int ret = (int)m_material_data.size();
                m_material_data.push_back(md);
                return ret;
            }
        };

        for (auto& pair : m_buffers) {
            auto& buf = pair.second;
            if (buf.isModelData()) {
                buf.material_id = findOrAddMaterial(buf.material);
            }
        }

        m_materials.resize(m_material_data.size());
        for (int i = 0; i < (int)m_materials.size(); ++i) {
            auto& mat = m_materials[i];
            if (!mat)
                mat = ms::Material::create();

            char name[128];
            sprintf(name, "XismoMaterial:ID[%04x]", i);
            mat->id = i;
            mat->name = name;
            mat->setColor(m_material_data[i].diffuse);
        }
    }

    // camera
    if (m_settings.sync_camera) {
        if (!m_camera) {
            m_camera = ms::Camera::create();
            m_camera->path = "/Main Camera";
        }
        m_camera->position = m_camera_pos;
        m_camera->rotation = m_camera_rot;
        m_camera->fov = m_camera_fov;
        m_camera->near_plane = m_camera_near;
        m_camera->far_plane = m_camera_far;
        m_camera_dirty = false;
    }

    m_deleted.swap(m_meshes_deleted);
    m_meshes_deleted.clear();

    // build send data
    parallel_for_each(buffers_to_send.begin(), buffers_to_send.end(), [](BufferData *buf) {
        buf->updateSendData();
    });
    m_mesh_data.resize(buffers_to_send.size());
    for (size_t i = 0; i < buffers_to_send.size(); ++i) {
        m_mesh_data[i] = buffers_to_send[i]->send_data;
    }

    // kick async send
    m_send_future = std::async(std::launch::async, [this]() {
        ms::Client client(m_settings.client_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Left;
        scene_settings.scale_factor = m_settings.scale_factor;


        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

        // send deleted
        if (m_settings.sync_delete && !m_deleted.empty()) {
            ms::DeleteMessage del;
            for (auto h : m_deleted) {
                char path[128];
                sprintf(path, "/XismoMesh:ID[%08x]", h);
                del.targets.push_back({ path, (int)h });
            }
            client.send(del);
        }

        // send camera & materials
        {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            if (m_settings.sync_camera) {
                set.scene.objects.push_back(std::static_pointer_cast<ms::Transform>(m_camera));
            }
            set.scene.materials = m_materials;
            client.send(set);
        }

        // send meshes
        mu::parallel_for_each(m_mesh_data.begin(), m_mesh_data.end(), [&](BufferData::SendTaskPtr& v) {
            v->buildMeshData(m_settings.weld_vertices);
        });
        for (auto& v : m_mesh_data) {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.objects = { v->dst_mesh };
            client.send(set);
        }
        m_mesh_data.clear();

        // notify scene end
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneEnd;
            client.send(fence);
        }
    });
}

void msxmContext::onActiveTexture(GLenum texture)
{
    m_texture_slot = texture - GL_TEXTURE0;
}

void msxmContext::onBindTexture(GLenum target, GLuint texture)
{
    if (m_texture_slot == 0) {
        m_material.texture = texture;
    }
}

BufferData* msxmContext::getActiveBuffer(GLenum target)
{
    uint32_t bid = 0;
    if (target == GL_ARRAY_BUFFER) {
        bid = m_vb_handle;
    }
    return bid != 0 ? &m_buffers[bid] : nullptr;
}


void msxmContext::onGenBuffers(GLsizei n, GLuint * handles)
{
    for (int i = 0; i < (int)n; ++i) {
        auto it = std::find(m_meshes_deleted.begin(), m_meshes_deleted.end(), handles[i]);
        if (it != m_meshes_deleted.end()) {
            m_meshes_deleted.erase(it);
        }
    }
}

void msxmContext::onDeleteBuffers(GLsizei n, const GLuint * handles)
{
    for (int i = 0; i < n; ++i) {
        auto it = m_buffers.find(handles[i]);
        if (it != m_buffers.end()) {
            if (it->second.isModelData()) {
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

void msxmContext::onMapBuffer(GLenum target, GLenum access, void *&mapped_data)
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

void msxmContext::onUnmapBuffer(GLenum target)
{
    if (auto *buf = getActiveBuffer(target)) {
        if (buf->mapped_data) {
            parallel_invoke([buf]() {
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
        m_material.diffuse.assign(value);
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
        {
            // projection matrix -> camera fov & clippling planes
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

void msxmContext::onFlush()
{
    if (m_settings.auto_sync) {
        send(false);
    }
}

