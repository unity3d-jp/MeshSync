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
    void            *mapped_data = nullptr;
    GLuint          handle = 0;
    int             stride = 0;
    bool            dirty = false;
};

struct XismoSyncContext
{
    ms::ClientSettings m_settings;

    std::map<uint32_t, BufferState> buffers;
    uint32_t current_vb = 0;
    uint32_t current_cb = 0;
} g_ctx;


static PFNGLGENBUFFERSPROC      _glGenBuffers;
static PFNGLDELETEBUFFERSPROC   _glDeleteBuffers;
static PFNGLBINDBUFFERPROC      _glBindBuffer;
static PFNGLBUFFERDATAPROC      _glBufferData;
static PFNGLMAPBUFFERPROC       _glMapBuffer;
static PFNGLUNMAPBUFFERPROC     _glUnmapBuffer;
static PFNGLVERTEXATTRIBPOINTERPROC _glVertexAttribPointer;
static void* (* WINAPI _wglGetProcAddress)(const char* name);
static void (* WINAPI _glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);

static BufferState* GetActiveBuffer(GLenum target)
{
    uint32_t bid = 0;
    if (target == GL_ARRAY_BUFFER) {
        bid = g_ctx.current_vb;
    }
    else if (target == GL_UNIFORM_BUFFER) {
        bid = g_ctx.current_cb;
    }

    return bid != 0 ? &g_ctx.buffers[bid] : nullptr;
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
    else if (target == GL_UNIFORM_BUFFER) {
        g_ctx.current_cb = buffer;
    }
    _glBindBuffer(target, buffer);
}

static void WINAPI glBufferData_hook(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    if (auto *buf = GetActiveBuffer(target)) {
        buf->data.resize_discard(size);
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


struct HookData
{
    const char *name;
    void *func_hook;
};
static std::vector<HookData> g_hooks;

static void* WINAPI wglGetProcAddress_hook(const char* name)
{
    if (g_hooks.empty()) {
#define Import(name) (void*&)_##name = (void*)_wglGetProcAddress(#name); g_hooks.push_back({#name, name##_hook});
        Import(glGenBuffers);
        Import(glDeleteBuffers);
        Import(glBindBuffer);
        Import(glBufferData);
        Import(glMapBuffer);
        Import(glUnmapBuffer);
        Import(glVertexAttribPointer);
#undef Import
    }

    auto i = std::find_if(g_hooks.begin(), g_hooks.end(), [name](const HookData& h) {
        return strcmp(h.name, name) == 0;
    });

    if (i != g_hooks.end()) {
        return i->func_hook;
    }
    else {
        return _wglGetProcAddress(name);
    }
}

static void WINAPI glDrawElements_hook(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices)
{
    if (mode == GL_TRIANGLES) {

    }
    _glDrawElements(mode, count, type, indices);
}


template<class T>
static inline void ForceWrite(T &dst, const T &src)
{
    DWORD old_flag;
    ::VirtualProtect(&dst, sizeof(T), PAGE_EXECUTE_READWRITE, &old_flag);
    dst = src;
    ::VirtualProtect(&dst, sizeof(T), old_flag, &old_flag);
}

static void* OverrideDLLExport(HMODULE module, const char *funcname, void *replacement)
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
            ForceWrite<DWORD>(RVAFunctions[RVANameOrdinals[i]], (DWORD)replacement - ImageBase);
            return before;
        }
    }
    return nullptr;
}

static void SetupHooks()
{
    auto opengl32 = ::LoadLibraryA("opengl32.dll");
#define Override(name) (void*&)_##name = OverrideDLLExport(opengl32, #name, name##_hook);
    Override(wglGetProcAddress);
    Override(glDrawElements);
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
