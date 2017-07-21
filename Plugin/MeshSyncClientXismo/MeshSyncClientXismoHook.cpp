#include "pch.h"
#include "MeshSync/MeshSync.h"
using namespace mu;

struct vertex_t
{
    float3 vertex;
    float3 normal;
    float4 color;
    float2 uv;
    float state;
};

struct VertexData
{
    RawVector<char> data;
    void        *mapped_data = nullptr;
    GLuint      handle = 0;
    int         stride = 0;
    bool        triangle = false;
    bool        dirty = false;
    float4x4    transform = float4x4::identity();

    ms::MeshPtr send_data;
};

struct XismoSyncContext
{
    ms::ClientSettings settings;
    float scale_factor = 100.0f;

    std::map<uint32_t, VertexData> buffers;
    std::vector<VertexData*> buffers_to_send;
    std::vector<ms::MaterialPtr> materials;
    std::vector<ms::MeshPtr> deleted;
    std::future<void> send_future;
    uint32_t current_vb = 0;
    float4x4 current_transform = float4x4::identity();
};


static PFNGLGENBUFFERSPROC      _glGenBuffers;
static PFNGLDELETEBUFFERSPROC   _glDeleteBuffers;
static PFNGLBINDBUFFERPROC      _glBindBuffer;
static PFNGLBUFFERDATAPROC      _glBufferData;
static PFNGLMAPBUFFERPROC       _glMapBuffer;
static PFNGLUNMAPBUFFERPROC     _glUnmapBuffer;
static PFNGLVERTEXATTRIBPOINTERPROC _glVertexAttribPointer;
static PFNGLPROGRAMUNIFORMMATRIX4FVPROC _glProgramUniformMatrix4fv;
static void(*GLAPIENTRY _glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
static void(*GLAPIENTRY _glFlush)(void);
static void* (*GLAPIENTRY _wglGetProcAddress)(const char* name);
static XismoSyncContext g_ctx;


static VertexData* GetActiveBuffer(GLenum target)
{
    uint32_t bid = 0;
    if (target == GL_ARRAY_BUFFER) {
        bid = g_ctx.current_vb;
    }
    return bid != 0 ? &g_ctx.buffers[bid] : nullptr;
}

static void SendMeshes()
{
    if (g_ctx.send_future.valid() && g_ctx.send_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
    {
        // previous request is not completed yet
        return;
    }

    for (auto& pair : g_ctx.buffers) {
        auto& buf = pair.second;
        if (buf.stride == sizeof(vertex_t) && buf.dirty && buf.triangle) {
            g_ctx.buffers_to_send.push_back(&buf);
        }
    }
    if (g_ctx.buffers_to_send.empty() && g_ctx.deleted.empty()) {
        return;
    }

    g_ctx.send_future = std::async(std::launch::async, []() {
        ms::Client client(g_ctx.settings);

        ms::SceneSettings scene_settings;
        scene_settings.handedness = ms::Handedness::Right;
        scene_settings.scale_factor = g_ctx.scale_factor;

        if (g_ctx.materials.empty()) {
            auto mat = new ms::Material();
            mat->name = "Default Material";
            g_ctx.materials.emplace_back(mat);
        }

        // notify scene begin
        {
            ms::FenceMessage fence;
            fence.type = ms::FenceMessage::FenceType::SceneBegin;
            client.send(fence);
        }

        // send material
        {
            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.materials = g_ctx.materials;
            client.send(set);
        }

        // send meshes
        parallel_for_each(g_ctx.buffers_to_send.begin(), g_ctx.buffers_to_send.end(), [&](VertexData *buf) {
            buf->dirty = false;

            auto vertices = (const vertex_t*)buf->data.data();
            int num_vertices = (int)(buf->data.size() / sizeof(vertex_t));
            int num_triangles = num_vertices / 3;

            if (!buf->send_data) {
                buf->send_data.reset(new ms::Mesh());

                char name[128];
                sprintf(name, "/xismo:[%08x]", buf->handle);
                buf->send_data->path = name;
                buf->send_data->id = buf->handle;
            }
            auto& mesh = *buf->send_data;
            mesh.flags.has_points = 1;
            mesh.flags.has_normals = 1;
            mesh.flags.has_uv = 1;
            mesh.flags.has_counts = 1;
            mesh.flags.has_indices = 1;
            mesh.flags.has_materialIDs = 1;
            mesh.flags.has_refine_settings = 1;

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

            mesh.counts.resize_discard(num_triangles);
            mesh.materialIDs.resize_discard(num_triangles);
            for (int ti = 0; ti < num_triangles; ++ti) {
                mesh.counts[ti] = 3;
                mesh.materialIDs[ti] = 0;
            }

            ms::SetMessage set;
            set.scene.settings = scene_settings;
            set.scene.meshes = { buf->send_data };
            client.send(set);
        });
        g_ctx.buffers_to_send.clear();

        // send deleted
        if (!g_ctx.deleted.empty()) {
            ms::DeleteMessage del;
            for (auto& p : g_ctx.deleted) {
                del.targets.push_back({ p->path , p->id });
            }
            client.send(del);
            g_ctx.deleted.clear();
        }

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
            if (it->second.send_data) {
                g_ctx.deleted.push_back(it->second.send_data);
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

static void WINAPI glProgramUniformMatrix4fv_hook(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    _glProgramUniformMatrix4fv(program, location, count, transpose, value);
}

static void WINAPI glDrawElements_hook(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
    if (mode == GL_TRIANGLES) {
        auto *buf = GetActiveBuffer(GL_ARRAY_BUFFER);
        if (buf && buf->stride == sizeof(vertex_t) && buf->dirty) {
            buf->triangle = true;
            buf->transform = g_ctx.current_transform;
        }
    }
    _glDrawElements(mode, count, type, indices);
}

static void WINAPI glFlush_hook(void)
{
    SendMeshes();
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
    Hook(glProgramUniformMatrix4fv),
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
        ret = ::VirtualAlloc((void*)((size_t)base + (step*i)), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }
    return ret;
}

static void* EmitJumpInstruction(void* from_, void* to_)
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
            jump_table = EmitJumpInstruction(jump_table, replacement);
            return before;
        }
    }
    return nullptr;
}

static void SetupHooks()
{
    auto opengl32 = ::LoadLibraryA("opengl32.dll");
    auto jumptable = AllocateExecutableMemoryForward(1024, opengl32);

#define Override(Name) (void*&)_##Name = OverrideDLLExport(opengl32, #Name, Name##_hook, jumptable);
    Override(wglGetProcAddress);
    Override(glDrawElements);
    Override(glFlush);
#undef Override
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        SetupHooks();
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
