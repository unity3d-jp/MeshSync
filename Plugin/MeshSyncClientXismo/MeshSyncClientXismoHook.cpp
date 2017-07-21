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

struct BufferState
{
    RawVector<char> data;
    void    *mapped_data = nullptr;
    int     stride = 0;
    bool    dirty = false;
};

struct SendContext
{
    ms::Mesh mesh;
    std::future<void> future;
};

struct XismoSyncContext
{
    ms::ClientSettings settings;
    std::map<uint32_t, ms::Mesh> msmeshes;

    std::map<uint32_t, BufferState> buffers;
    uint32_t current_vb = 0;
} g_ctx;


static PFNGLGENBUFFERSPROC      _glGenBuffers;
static PFNGLDELETEBUFFERSPROC   _glDeleteBuffers;
static PFNGLBINDBUFFERPROC      _glBindBuffer;
static PFNGLBUFFERDATAPROC      _glBufferData;
static PFNGLMAPBUFFERPROC       _glMapBuffer;
static PFNGLUNMAPBUFFERPROC     _glUnmapBuffer;
static PFNGLVERTEXATTRIBPOINTERPROC _glVertexAttribPointer;
static void(*GLAPIENTRY _glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);
static void(*GLAPIENTRY _glFlush)(void);
static void* (*GLAPIENTRY _wglGetProcAddress)(const char* name);

static BufferState* GetActiveBuffer(GLenum target)
{
    uint32_t bid = 0;
    if (target == GL_ARRAY_BUFFER) {
        bid = g_ctx.current_vb;
    }
    return bid != 0 ? &g_ctx.buffers[bid] : nullptr;
}

static void SendMeshData(uint32_t id, const vertex_t *vertices, size_t num_vertices)
{

}


static void WINAPI glGenBuffers_hook(GLsizei n, GLuint* buffers)
{
    _glGenBuffers(n, buffers);
}

static void WINAPI glDeleteBuffers_hook(GLsizei n, const GLuint* buffers)
{
    for (int i = 0; i < n; ++i) {
        auto it = g_ctx.buffers.find(buffers[i]);
        if (it != g_ctx.buffers.end()) {
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
        buf->data.resize_discard(size);
        if (data) {
            memcpy(buf->data.data(), data, buf->data.size());
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

static void WINAPI glDrawElements_hook(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
    if (mode == GL_TRIANGLES) {
        auto *buf = GetActiveBuffer(GL_ARRAY_BUFFER);
        if (buf && buf->stride == sizeof(vertex_t) && buf->dirty) {
            buf->dirty = false;
            SendMeshData(g_ctx.current_vb, (vertex_t*)buf->data.data(), buf->data.size() / sizeof(vertex_t));
        }
    }
    _glDrawElements(mode, count, type, indices);
}

static void WINAPI glFlush_hook(void)
{
    _glFlush();
}

struct HookData
{
    const char *name;
    void **hook;
    void **orig;
};

static void* WINAPI wglGetProcAddress_hook(const char* name)
{
    char buf[1024];
    sprintf(buf, "%s\n", name);
    ::OutputDebugStringA(buf);

    static HookData s_hooks[] = {
#define Import(name) {#name, (void**)&name##_hook, (void**)&_##name}
    Import(glGenBuffers),
    Import(glDeleteBuffers),
    Import(glBindBuffer),
    Import(glBufferData),
    Import(glMapBuffer),
    Import(glUnmapBuffer),
    Import(glVertexAttribPointer),
    Import(glDrawElements),
    Import(glFlush),
#undef Import
    };

    for (auto& hook : s_hooks) {
        if (strcmp(hook.name, name) == 0) {
            auto sym = _wglGetProcAddress(name);
            if (sym) {
                *hook.orig = sym;
                return hook.hook;
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
        // 0xe9 RVA
        from[0] = 0xe9;
        from += 1;
        *((DWORD*)from) = (DWORD)(to - jump_from);
        from += sizeof(DWORD);
    }
    else {
        // 0xff 0x25 RVA TargetAddr
        from[0] = 0xff;
        from[1] = 0x25;
        from += 2;
#ifdef _M_IX86
        *((DWORD*)from) = (DWORD)(from + 4);
#elif defined(_M_X64)
        *((DWORD*)from) = (DWORD)0;
#endif
        from += 4;
        *((DWORD_PTR*)from) = (DWORD_PTR)(to);
        from += sizeof(DWORD_PTR);
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

#define Override(name) (void*&)_##name = OverrideDLLExport(opengl32, #name, name##_hook, jumptable);
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
