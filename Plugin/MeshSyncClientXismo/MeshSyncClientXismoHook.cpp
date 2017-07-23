#include "pch.h"
#include "MeshSync/MeshSync.h"
#include "MeshSyncClientXismo.h"

using namespace mu;

struct vertex_t
{
    float3 vertex;
    float3 normal;
    float4 color;
    float2 uv;
    float state;

    bool operator==(const vertex_t& v) const
    {
        return
            vertex == v.vertex &&
            normal == v.normal &&
            color == v.color &&
            uv == v.uv;
            // no need to compare state
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

struct SendTaskData
{
    RawVector<char>     data;
    GLuint              handle = 0;
    int                 num_elements = 0;
    bool                drawn = false;
    int                 material_id = 0;
    float4x4            transform = float4x4::identity();
    RawVector<vertex_t> vertices_welded;
    ms::MeshPtr         ms_mesh;
};
using SendTaskPtr = std::shared_ptr<SendTaskData>;

struct VertexData
{
    RawVector<char> data;
    void        *mapped_data = nullptr;
    GLuint      handle = 0;
    int         num_elements = 0;
    int         stride = 0;
    bool        triangle = false;
    bool        dirty = false;
    bool        drawn = false;
    MaterialData material;
    float4x4    transform = float4x4::identity();

    SendTaskPtr send_task;

    void updateTaskData()
    {
        if (!send_task) {
            send_task.reset(new SendTaskData());
        }
        auto& dst = *send_task;
        dst.data = data;
        dst.handle = handle;
        dst.num_elements = num_elements;
        dst.drawn = drawn;
        dst.transform = transform;
    }
};


struct XismoSyncContext
{
    XismoSyncSettings settings;

    std::map<uint32_t, VertexData> buffers;

    ms::SetMessage send_scene;
    std::vector<MaterialData> materials;
    std::vector<SendTaskPtr> send_tasks;
    std::vector<GLuint> meshes_deleted;
    std::future<void> send_future;

    uint32_t current_vb = 0;
    MaterialData current_material;
    float4x4 current_transform = float4x4::identity();
    float4x4 current_proj = float4x4::identity();
    float4x4 current_modelview = float4x4::identity();

    bool camera_dirty = false;
    float3 current_camera_pos = float3::zero();
    quatf current_camera_rot = quatf::identity();
    float current_camera_fov = 60.0f;

    int findOrAddMaterial(const MaterialData& md);
};

int XismoSyncContext::findOrAddMaterial(const MaterialData& md)
{
    auto it = std::find(materials.begin(), materials.end(), md);
    if (it != materials.end()) {
        return (int)std::distance(materials.begin(), it);
    }
    else {
        int ret = (int)materials.size();
        materials.push_back(md);
        return ret;
    }
}

#pragma warning(push)
#pragma warning(disable:4229)  
static void(*WINAPI _glGenBuffers)(GLsizei n, GLuint* buffers);
static void(*WINAPI _glDeleteBuffers) (GLsizei n, const GLuint* buffers);
static void(*WINAPI _glBindBuffer) (GLenum target, GLuint buffer);
static void(*WINAPI _glBufferData) (GLenum target, GLsizeiptr size, const void* data, GLenum usage);
static void* (*WINAPI _glMapBuffer) (GLenum target, GLenum access);
static GLboolean(*WINAPI _glUnmapBuffer) (GLenum target);
static void(*WINAPI _glVertexAttribPointer) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
static void(*WINAPI _glUniform4fv) (GLint location, GLsizei count, const GLfloat* value);
static void(*WINAPI _glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
static void(*WINAPI _glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
static void(*WINAPI _glFlush)(void);
static void* (*WINAPI _wglGetProcAddress)(const char* name);
#pragma warning(pop)

static XismoSyncContext g_ctx;

XismoSyncSettings& msxmGetSettings()
{
    return g_ctx.settings;
}

void msxmForceSetDirty()
{
    for (auto& pair : g_ctx.buffers) {
        auto& buf = pair.second;
        buf.dirty = true;
    }
}


static VertexData* GetActiveBuffer(GLenum target)
{
    uint32_t bid = 0;
    if (target == GL_ARRAY_BUFFER) {
        bid = g_ctx.current_vb;
    }
    return bid != 0 ? &g_ctx.buffers[bid] : nullptr;
}

static void Weld(const vertex_t src[], int num_vertices, RawVector<vertex_t>& dst_vertices, RawVector<int>& dst_indices)
{
    dst_vertices.clear();
    dst_vertices.reserve_discard(num_vertices / 2);
    dst_indices.resize_discard(num_vertices);

    for (int vi = 0; vi < num_vertices; ++vi) {
        auto tmp = src[vi];
        auto it = std::find_if(dst_vertices.begin(), dst_vertices.end(), [&](const vertex_t& v) { return v == tmp; });
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

void msxmSend(bool force)
{
    if (g_ctx.send_future.valid() && g_ctx.send_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
    {
        // previous request is not completed yet
        return;
    }

    auto& scene = g_ctx.send_scene.scene;
    std::vector<VertexData*> buffers_to_send;
    for (auto& pair : g_ctx.buffers) {
        auto& buf = pair.second;
        if (buf.stride == sizeof(vertex_t) && (force || buf.dirty) && buf.triangle) {
            buffers_to_send.push_back(&buf);
        }
    }
    if (buffers_to_send.empty() && g_ctx.meshes_deleted.empty() && (!g_ctx.settings.sync_camera || !g_ctx.camera_dirty)) {
        return;
    }

    // make copy for worker thread
    parallel_for_each(buffers_to_send.begin(), buffers_to_send.end(), [&](VertexData *buf) {
        buf->updateTaskData();
        buf->dirty = false;
        buf->drawn = false;
    });
    g_ctx.send_tasks.resize(buffers_to_send.size());
    for (size_t i = 0; i < buffers_to_send.size(); ++i) {
        g_ctx.send_tasks[i] = buffers_to_send[i]->send_task;
    }

    // build material list
    {
        g_ctx.materials.clear();
        for (auto& pair : g_ctx.buffers) {
            auto& buf = pair.second;
            if (buf.send_task) {
                buf.send_task->material_id = g_ctx.findOrAddMaterial(buf.material);
            }
        }

        scene.materials.resize(g_ctx.materials.size());
        for (int i = 0; i < (int)scene.materials.size(); ++i) {
            auto& mat = scene.materials[i];
            if (!mat) mat.reset(new ms::Material());

            char name[128];
            sprintf(name, "XismoMaterial:ID[%04x]", i);
            mat->name = name;
            mat->color = g_ctx.materials[i].difuse;
        }
    }

    // camera
    if (g_ctx.settings.sync_camera) {
        if (scene.cameras.empty()) {
            auto c = new ms::Camera();
            c->path = "/Main Camera";
            scene.cameras.emplace_back(c);
        }
        auto& cam = *scene.cameras.back();
        cam.transform.position = g_ctx.current_camera_pos;
        cam.transform.rotation = g_ctx.current_camera_rot;
        cam.fov = g_ctx.current_camera_fov;
        g_ctx.camera_dirty = false;
    }
    else {
        scene.cameras.clear();
    }

    g_ctx.send_future = std::async(std::launch::async, []() {
        ms::Client client(g_ctx.settings.client_settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Left;
        scene_settings.scale_factor = g_ctx.settings.scale_factor;


        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

        // send material
        g_ctx.send_scene.scene.settings = scene_settings;
        client.send(g_ctx.send_scene);

        // send deleted
        if (!g_ctx.meshes_deleted.empty()) {
            ms::DeleteMessage del;
            for (auto h : g_ctx.meshes_deleted) {
                auto it = std::find_if(g_ctx.send_tasks.begin(), g_ctx.send_tasks.end(), [h](SendTaskPtr& v) {
                    return v->handle == h;
                });
                if (it == g_ctx.send_tasks.end()) {
                    del.targets.push_back({ "", (int)h });
                }
            }
            if (!del.targets.empty()) {
                client.send(del);
            }
            g_ctx.meshes_deleted.clear();
        }

        // send meshes
        parallel_for_each(g_ctx.send_tasks.begin(), g_ctx.send_tasks.end(), [&](SendTaskPtr& ptask) {
            auto& task = *ptask;
            auto vertices = (const vertex_t*)task.data.data();
            if (!vertices) { return; }
            int num_indices = task.num_elements;
            int num_triangles = num_indices / 3;

            if (!task.ms_mesh) {
                task.ms_mesh.reset(new ms::Mesh());

                char name[128];
                sprintf(name, "/XismoMesh:ID[%08x]", task.handle);
                task.ms_mesh->path = name;
                task.ms_mesh->id = task.handle;
            }
            auto& mesh = *task.ms_mesh;
            mesh.flags.has_points = 1;
            mesh.flags.has_normals = 1;
            mesh.flags.has_uv = 1;
            mesh.flags.has_counts = 1;
            mesh.flags.has_indices = 1;
            mesh.flags.has_materialIDs = 1;
            mesh.flags.has_refine_settings = 1;
            mesh.refine_settings.flags.swap_faces = true;

            if (g_ctx.settings.weld) {
                Weld(vertices, num_indices, task.vertices_welded, mesh.indices);

                int num_vertices = (int)task.vertices_welded.size();
                mesh.points.resize_discard(num_vertices);
                mesh.normals.resize_discard(num_vertices);
                mesh.uv.resize_discard(num_vertices);
                mesh.colors.resize_discard(num_vertices);
                for (int vi = 0; vi < num_vertices; ++vi) {
                    auto& v = task.vertices_welded[vi];
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
                mesh.materialIDs[ti] = task.material_id;
            }

            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.meshes = { task.ms_mesh };
            client.send(set);
        });
        g_ctx.send_tasks.clear();

        // notify scene end
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneEnd;
            client.send(fence);
        }
    });
}


static void WINAPI glGenBuffers_hook(GLsizei n, GLuint* buffers)
{
    _glGenBuffers(n, buffers);
}

static void WINAPI glDeleteBuffers_hook(GLsizei n, const GLuint* buffers)
{
    if (g_ctx.send_future.valid()) {
        g_ctx.send_future.wait();
    }
    for (int i = 0; i < n; ++i) {
        auto it = g_ctx.buffers.find(buffers[i]);
        if (it != g_ctx.buffers.end()) {
            if (it->second.send_task) {
                g_ctx.meshes_deleted.push_back(it->second.handle);
            }
            g_ctx.buffers.erase(it);
        }
    }
    _glDeleteBuffers(n, buffers);
}

static void WINAPI glBindBuffer_hook(GLenum target, GLuint buffer)
{
    if (target == GL_ARRAY_BUFFER) {
        g_ctx.current_vb = buffer;
    }
    _glBindBuffer(target, buffer);
}

static void WINAPI glBufferData_hook(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    if (auto *buf = GetActiveBuffer(target)) {
        buf->handle = g_ctx.current_vb;
        buf->data.resize_discard(size);
        if (data) {
            memcpy(buf->data.data(), data, buf->data.size());
            buf->dirty = true;
        }
    }
    _glBufferData(target, size, data, usage);
}

static void* WINAPI glMapBuffer_hook(GLenum target, GLenum access)
{
    auto ret = _glMapBuffer(target, access);
    if (auto *buf = GetActiveBuffer(target)) {
        buf->mapped_data = ret;
    }
    return ret;
}

static GLboolean WINAPI glUnmapBuffer_hook(GLenum target)
{
    if (auto *buf = GetActiveBuffer(target)) {
        if (buf->mapped_data) {
            memcpy(buf->data.data(), buf->mapped_data, buf->data.size());
            buf->mapped_data = nullptr;
            buf->dirty = true;
        }
    }

    auto ret = _glUnmapBuffer(target);
    return ret;
}

static void WINAPI glVertexAttribPointer_hook(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
{
    if (auto *buf = GetActiveBuffer(GL_ARRAY_BUFFER)) {
        buf->stride = stride;
    }
    _glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

static void WINAPI glUniform4fv_hook(GLint location, GLsizei count, const GLfloat* value)
{
    if (location == 3) {
        // diffuse
        g_ctx.current_material.difuse.assign(value);
    }
    _glUniform4fv(location, count, value);
}

static void WINAPI glUniformMatrix4fv_hook(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    if (location == 0) {
        // modelview matrix
        g_ctx.current_modelview.assign(value);
    }
    else if (location == 1) {
        // projection matrix
        g_ctx.current_proj.assign(value);
    }

    _glUniformMatrix4fv(location, count, transpose, value);
}

static void WINAPI glDrawElements_hook(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
    if (mode == GL_TRIANGLES) {
        auto *buf = GetActiveBuffer(GL_ARRAY_BUFFER);
        if (buf && buf->stride == sizeof(vertex_t)) {
            buf->triangle = true;
            buf->drawn = true;
            buf->num_elements = (int)count;
            if (buf->material != g_ctx.current_material) {
                buf->material = g_ctx.current_material;
                buf->dirty = true;
            }
            {
                float3 pos, forward, up, right;
                view_to_camera(g_ctx.current_modelview, pos, forward, up, right);
                quatf rot = to_quat(look33(forward, float3{ 0.0f, 1.0f, 0.0f }));
                if (pos != g_ctx.current_camera_pos ||
                    rot != g_ctx.current_camera_rot)
                {
                    g_ctx.camera_dirty = true;
                    g_ctx.current_camera_pos = pos;
                    g_ctx.current_camera_rot = rot;
                }
            }
            {
                float thf = 1.0f / g_ctx.current_proj[1][1];
                float fov = std::atan(thf) *  Rad2Deg;
                if (fov != g_ctx.current_camera_fov) {
                    g_ctx.camera_dirty = true;
                    g_ctx.current_camera_fov = fov;
                }
            }

            buf->transform = g_ctx.current_transform;
        }
    }
    _glDrawElements(mode, count, type, indices);
}

static void WINAPI glFlush_hook(void)
{
    msxmInitializeWidget();
    if (g_ctx.settings.auto_sync) {
        msxmSend();
    }
    _glFlush();
}

static void* WINAPI wglGetProcAddress_hook(const char* name)
{
    static struct HookData
    {
        const char *name;
        void **func_hook;
        void **func_orig;
    }
    s_hooks[] = {
#define Hook(Name) {#Name, (void**)&Name##_hook, (void**)&_##Name}
    Hook(glGenBuffers),
    Hook(glDeleteBuffers),
    Hook(glBindBuffer),
    Hook(glBufferData),
    Hook(glMapBuffer),
    Hook(glUnmapBuffer),
    Hook(glVertexAttribPointer),
    Hook(glUniform4fv),
    Hook(glUniformMatrix4fv),
    Hook(glDrawElements),
    Hook(glFlush),
#undef Hook
    };

    for (auto& hook : s_hooks) {
        if (strcmp(hook.name, name) == 0) {
            auto sym = _wglGetProcAddress(name);
            if (sym) {
                *hook.func_orig = sym;
                return hook.func_hook;
            }
        }
    }
    return _wglGetProcAddress(name);
}



template<class T>
static inline void ForceWrite(T &dst, const T &src)
{
    DWORD old_flag;
    ::VirtualProtect(&dst, sizeof(T), PAGE_EXECUTE_READWRITE, &old_flag);
    dst = src;
    ::VirtualProtect(&dst, sizeof(T), old_flag, &old_flag);
}

static void* AllocateExecutableMemoryForward(size_t size, void *location)
{
    static size_t base = (size_t)location;

    void *ret = nullptr;
    const size_t step = 0x10000; // 64kb
    for (size_t i = 0; ret == nullptr; ++i) {
        // increment address until VirtualAlloc() succeed
        // (MSDN says "If the memory is already reserved and is being committed, the address is rounded down to the next page boundary".
        //  https://msdn.microsoft.com/en-us/library/windows/desktop/aa366887.aspx
        //  but it seems VirtualAlloc() return nullptr if memory is already reserved and is being committed)
        ret = ::VirtualAlloc((void*)((size_t)base + (step*i)), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }
    return ret;
}

static void* EmitJmpInstruction(void* from_, void* to_)
{
    BYTE *from = (BYTE*)from_;
    BYTE *to = (BYTE*)to_;
    BYTE *jump_from = from + 5;
    size_t distance = jump_from > to ? jump_from - to : to - jump_from;
    if (distance <= 0x7fff0000) {
        // 0xe9 [RVA]
        *(from++) = 0xe9;
        *(((DWORD*&)from)++) = (DWORD)(to - jump_from);
    }
    else {
        // 0xff 0x25 [RVA] [TargetAddr]
        *(from++) = 0xff;
        *(from++) = 0x25;
#ifdef _M_IX86
        *(((DWORD*&)from)++) = (DWORD)(from + 4);
#elif defined(_M_X64)
        *(((DWORD*&)from)++) = (DWORD)0;
#endif
        *(((DWORD_PTR*&)from)++) = (DWORD_PTR)(to);
    }
    return from;
}

static void* OverrideDLLExport(HMODULE module, const char *funcname, void *replacement, void *&jump_table)
{
    if (!module) { return nullptr; }

    size_t ImageBase = (size_t)module;
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) { return nullptr; }

    PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(ImageBase + pDosHeader->e_lfanew);
    DWORD RVAExports = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    if (RVAExports == 0) { return nullptr; }

    IMAGE_EXPORT_DIRECTORY *pExportDirectory = (IMAGE_EXPORT_DIRECTORY *)(ImageBase + RVAExports);
    DWORD *RVANames = (DWORD*)(ImageBase + pExportDirectory->AddressOfNames);
    WORD *RVANameOrdinals = (WORD*)(ImageBase + pExportDirectory->AddressOfNameOrdinals);
    DWORD *RVAFunctions = (DWORD*)(ImageBase + pExportDirectory->AddressOfFunctions);
    for (DWORD i = 0; i < pExportDirectory->NumberOfFunctions; ++i) {
        char *pName = (char*)(ImageBase + RVANames[i]);
        if (strcmp(pName, funcname) == 0) {
            void *before = (void*)(ImageBase + RVAFunctions[RVANameOrdinals[i]]);
            DWORD RVAJumpTable = (DWORD)((size_t)jump_table - ImageBase);
            ForceWrite<DWORD>(RVAFunctions[RVANameOrdinals[i]], RVAJumpTable);
            jump_table = EmitJmpInstruction(jump_table, replacement);
            return before;
        }
    }
    return nullptr;
}



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        // setup hooks
        auto opengl32 = ::LoadLibraryA("opengl32.dll");
        auto jumptable = AllocateExecutableMemoryForward(1024, opengl32);

#define Override(Name) (void*&)_##Name = OverrideDLLExport(opengl32, #Name, Name##_hook, jumptable);
        Override(wglGetProcAddress);
        Override(glDrawElements);
        Override(glFlush);
#undef Override
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
