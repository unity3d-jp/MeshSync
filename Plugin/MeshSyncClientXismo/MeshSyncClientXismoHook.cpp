#include "pch.h"
#include "MeshSync/MeshSync.h"


void* (* WINAPI wglGetProcAddress_orig)(const char* name);

void* WINAPI wglGetProcAddress_hook(const char* name)
{
    ::OutputDebugStringA(name);
    ::OutputDebugStringA("\n");

    auto ret = wglGetProcAddress_orig(name);
    return ret;
}


template<class T>
inline void ForceWrite(T &dst, const T &src)
{
    DWORD old_flag;
    ::VirtualProtect(&dst, sizeof(T), PAGE_EXECUTE_READWRITE, &old_flag);
    dst = src;
    ::VirtualProtect(&dst, sizeof(T), old_flag, &old_flag);
}

void* OverrideDLLExport(HMODULE module, const char *funcname, void *replacement)
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

void SetHooks()
{
    auto opengl32 = ::LoadLibraryA("opengl32.dll");
    (void*&)wglGetProcAddress_orig = OverrideDLLExport(opengl32, "wglGetProcAddress", wglGetProcAddress_hook);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        SetHooks();
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
        //StopWebControllerServer();
    }
    return TRUE;
}
