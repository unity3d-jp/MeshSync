#include "pch.h"
#include "msvrContext.h"


#pragma warning(push)
#pragma warning(disable:4229)
static void(*WINAPI _glGenTextures)(GLsizei n, GLuint * textures);
static void(*WINAPI _glDeleteTextures)(GLsizei n, const GLuint * textures);
static void(*WINAPI _glActiveTexture)(GLenum texture);
static void(*WINAPI _glBindTexture)(GLenum target, GLuint texture);
static void(*WINAPI _glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * data);

static void(*WINAPI _glGenFramebuffers)(GLsizei n, GLuint *ids);
static void(*WINAPI _glBindFramebuffer)(GLenum target, GLuint framebuffer);
static void(*WINAPI _glDeleteFramebuffers)(GLsizei n, GLuint *framebuffers);
static void(*WINAPI _glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);

static void(*WINAPI _glGenBuffers)(GLsizei n, GLuint* buffers);
static void(*WINAPI _glDeleteBuffers) (GLsizei n, const GLuint* buffers);
static void(*WINAPI _glBindBuffer) (GLenum target, GLuint buffer);
static void(*WINAPI _glBindVertexBuffer) (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
static void(*WINAPI _glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);
static void(*WINAPI _glBufferData) (GLenum target, GLsizeiptr size, const void* data, GLenum usage);
static void(*WINAPI _glNamedBufferData)(GLuint buffer, GLsizei size, const void *data, GLenum usage);
static void(*WINAPI _glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
static void(*WINAPI _glNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizei size, const void *data);
static void* (*WINAPI _glMapBuffer) (GLenum target, GLenum access);
static void* (*WINAPI _glMapNamedBuffer)(GLuint buffer, GLenum access);
static GLboolean(*WINAPI _glUnmapBuffer) (GLenum target);
static GLboolean(*WINAPI _glUnmapNamedBuffer)(GLuint buffer);
static void* (*WINAPI _glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
static void* (*WINAPI _glMapNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access);
static void(*WINAPI _glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
static void(*WINAPI _glFlushMappedNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizei length);

static void(*WINAPI _glDeleteProgram)(GLuint program);
static void(*WINAPI _glUseProgram)(GLuint program);
/*static*/ void(*WINAPI _glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
/*static*/ void(*WINAPI _glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
/*static*/ GLint(*WINAPI _glGetUniformLocation)(GLuint program, const GLchar *name);

static void(*WINAPI _glUniform1i)(GLint location, GLint v0);
static void(*WINAPI _glUniform1f)(GLint location, GLfloat v0);
static void(*WINAPI _glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
static void(*WINAPI _glUniform1fv)(GLint location, GLsizei count, const GLfloat *value);
static void(*WINAPI _glUniform2fv)(GLint location, GLsizei count, const GLfloat *value);
static void(*WINAPI _glUniform3fv)(GLint location, GLsizei count, const GLfloat *value);
static void(*WINAPI _glUniform4fv)(GLint location, GLsizei count, const GLfloat *value);
static void(*WINAPI _glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
static void(*WINAPI _glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
static void(*WINAPI _glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

static void(*WINAPI _glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices);
static void(*WINAPI _glClear)(GLbitfield mask);
static void* (*WINAPI _wglGetProcAddress)(const char* name);

#pragma warning(pop)

static void WINAPI glGenTextures_hook(GLsizei n, GLuint * textures)
{
    _glGenTextures(n, textures);
    msvrGetContext()->onGenTextures(n, textures);
}
static void WINAPI glDeleteTextures_hook(GLsizei n, const GLuint * textures)
{
    msvrGetContext()->onDeleteTextures(n, textures);
    _glDeleteTextures(n, textures);
}
static void WINAPI glActiveTexture_hook(GLenum texture)
{
    msvrGetContext()->onActiveTexture(texture);
    _glActiveTexture(texture);
}
static void WINAPI glBindTexture_hook(GLenum target, GLuint texture)
{
    msvrGetContext()->onBindTexture(target, texture);
    _glBindTexture(target, texture);
}
static void WINAPI glTexImage2D_hook(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * data)
{
    msvrGetContext()->onTexImage2D(target, level, internalformat, width, height, border, format, type, data);
    _glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
}

static void WINAPI glGenFramebuffers_hook(GLsizei n, GLuint *ids)
{
    _glGenFramebuffers(n, ids);
    msvrGetContext()->onGenFramebuffers(n, ids);
}
static void WINAPI glBindFramebuffer_hook(GLenum target, GLuint framebuffer)
{
    msvrGetContext()->onBindFramebuffer(target, framebuffer);
    _glBindFramebuffer(target, framebuffer);
}
static void WINAPI glDeleteFramebuffers_hook(GLsizei n, GLuint *framebuffers)
{
    msvrGetContext()->onDeleteFramebuffers(n, framebuffers);
    _glDeleteFramebuffers(n, framebuffers);
}
static void WINAPI glFramebufferTexture_hook(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    msvrGetContext()->onFramebufferTexture(target, attachment, texture, level);
    _glFramebufferTexture(target, attachment, texture, level);
}

static void WINAPI glGenBuffers_hook(GLsizei n, GLuint* buffers)
{
    _glGenBuffers(n, buffers);
    msvrGetContext()->onGenBuffers(n, buffers);
}
static void WINAPI glDeleteBuffers_hook(GLsizei n, const GLuint* buffers)
{
    msvrGetContext()->onDeleteBuffers(n, buffers);
    _glDeleteBuffers(n, buffers);
}
static void WINAPI glBindBuffer_hook(GLenum target, GLuint buffer)
{
    msvrGetContext()->onBindBuffer(target, buffer);
    _glBindBuffer(target, buffer);
}
static void WINAPI glBindVertexBuffer_hook(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    msvrGetContext()->onBindVertexBuffer(bindingindex, buffer, offset, stride);
    _glBindVertexBuffer(bindingindex, buffer, offset, stride);
}
static void WINAPI glBindBufferBase_hook(GLenum target, GLuint index, GLuint buffer)
{
    msvrGetContext()->onBindBufferBase(target, index, buffer);
    _glBindBufferBase(target, index, buffer);
}
static void WINAPI glBufferData_hook(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    msvrGetContext()->onBufferData(target, size, data, usage);
    _glBufferData(target, size, data, usage);
}
static void WINAPI glNamedBufferData_hook(GLuint buffer, GLsizei size, const void *data, GLenum usage)
{
    _glNamedBufferData(buffer, size, data, usage);
}
static void WINAPI glBufferSubData_hook(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data)
{
    _glBufferSubData(target, offset, size, data);
}
static void WINAPI glNamedBufferSubData_hook(GLuint buffer, GLintptr offset, GLsizei size, const void *data)
{
    msvrGetContext()->onNamedBufferSubData(buffer, offset, size, data);
    _glNamedBufferSubData(buffer, offset, size, data);
}
static void* WINAPI glMapBuffer_hook(GLenum target, GLenum access)
{
    auto ret = _glMapBuffer(target, access);
    msvrGetContext()->onMapBuffer(target, access, ret);
    return ret;
}
static void* WINAPI glMapNamedBuffer_hook(GLuint buffer, GLenum access)
{
    auto ret = _glMapNamedBuffer(buffer, access);
    return ret;
}
static GLboolean WINAPI glUnmapBuffer_hook(GLenum target)
{
    msvrGetContext()->onUnmapBuffer(target);
    return _glUnmapBuffer(target);
}
static GLboolean WINAPI glUnmapNamedBuffer_hook(GLuint buffer)
{
    return _glUnmapNamedBuffer(buffer);
}
static void* WINAPI glMapBufferRange_hook(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    auto ret = _glMapBufferRange(target, offset, length, access);
    msvrGetContext()->onMapBufferRange(target, offset, length, access, ret);
    return ret;
}
static void* WINAPI glMapNamedBufferRange_hook(GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access)
{
    auto ret = _glMapNamedBufferRange(buffer, offset, length, access);
    return ret;
}
static void WINAPI glFlushMappedBufferRange_hook(GLenum target, GLintptr offset, GLsizeiptr length)
{
    msvrGetContext()->onFlushMappedBufferRange(target, offset, length);
    _glFlushMappedBufferRange(target, offset, length);
}
static void WINAPI glFlushMappedNamedBufferRange_hook(GLuint buffer, GLintptr offset, GLsizei length)
{
    _glFlushMappedNamedBufferRange(buffer, offset, length);
}

static void WINAPI glDeleteProgram_hook(GLuint program)
{
    msvrGetContext()->onDeleteProgram(program);
    _glDeleteProgram(program);
}
static void WINAPI glUseProgram_hook(GLuint program)
{
    _glUseProgram(program);
    msvrGetContext()->onUseProgram(program);
}

static void WINAPI glUniform1i_hook(GLint location, GLint v0)
{
    _glUniform1i(location, v0);
    msvrGetContext()->onUniform1i(location, v0);
}
static void WINAPI glUniform1f_hook(GLint location, GLfloat v0)
{
    _glUniform1f(location, v0);
    msvrGetContext()->onUniform1f(location, v0);
}
static void WINAPI glUniform2fv_hook(GLint location, GLsizei count, const GLfloat *value)
{
    _glUniform2fv(location, count, value);
    msvrGetContext()->onUniform2fv(location, count, value);
}
static void WINAPI glUniform3fv_hook(GLint location, GLsizei count, const GLfloat *value)
{
    _glUniform3fv(location, count, value);
    msvrGetContext()->onUniform3fv(location, count, value);
}
static void WINAPI glUniform4fv_hook(GLint location, GLsizei count, const GLfloat *value)
{
    _glUniform4fv(location, count, value);
}


bool g_color_pass = false;

static void WINAPI glDrawRangeElements_hook(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices)
{
    if (g_color_pass) {
        msvrInitializeWidget();
        msvrGetContext()->onDrawRangeElements(mode, start, end, count, type, indices);
    }
    _glDrawRangeElements(mode, start, end, count, type, indices);
}

static void WINAPI glClear_hook(GLbitfield mask)
{
    _glClear(mask);

    static const GLbitfield s_steps[] = {
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        GL_DEPTH_BUFFER_BIT,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        GL_COLOR_BUFFER_BIT,
    };
    static int s_pos = 0;
    if (mask == s_steps[s_pos]) {
        ++s_pos;
        if (s_pos == 2) {
            g_color_pass = true;
        }
        else if (s_pos == 3) {
            msvrGetContext()->onFlush();
            g_color_pass = false;
            s_pos = 0;
        }
    }
    else {
        s_pos = 0;
    }
    msvrGetContext()->onClear();
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
#define Get(Name) {#Name, nullptr, (void**)&_##Name}
    Hook(glGenTextures),
    Hook(glDeleteTextures),
    Hook(glActiveTexture),
    Hook(glBindTexture),
    //Hook(glTexImage2D),

    Hook(glGenFramebuffers),
    Hook(glBindFramebuffer),
    Hook(glDeleteFramebuffers),
    Hook(glFramebufferTexture),

    Hook(glGenBuffers),
    Hook(glDeleteBuffers),
    Hook(glBindBuffer),
    Hook(glBindVertexBuffer),
    Hook(glBindBufferBase),
    Hook(glBufferData),
    Hook(glNamedBufferData),
    Hook(glBufferSubData),
    Hook(glNamedBufferSubData),
    Hook(glMapBuffer),
    Hook(glMapNamedBuffer),
    Hook(glUnmapBuffer),
    Hook(glUnmapNamedBuffer),
    Hook(glMapBufferRange),
    Hook(glMapNamedBufferRange),
    Hook(glFlushMappedBufferRange),
    Hook(glFlushMappedNamedBufferRange),

    Hook(glDeleteProgram),
    Hook(glUseProgram),
    Get(glGetProgramiv),
    Get(glGetActiveUniform),
    Get(glGetUniformLocation),

    Hook(glUniform1i),
    Hook(glUniform1f),
    Hook(glUniform2fv),
    Hook(glUniform3fv),
    Hook(glUniform4fv),

    Hook(glDrawRangeElements),
#undef Get
#undef Hook
    };

    for (auto& hook : s_hooks) {
        if (strcmp(hook.name, name) == 0) {
            auto sym = _wglGetProcAddress(name);
            if (sym) {
                *hook.func_orig = sym;
                return hook.func_hook ? hook.func_hook : sym;
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
        Override(glClear);
        Override(glTexImage2D);
        //Override(glTexSubImage2D);
#undef Override
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
