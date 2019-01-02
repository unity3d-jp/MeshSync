#include "pch.h"

#define HookDllName "MeshSyncClientVREDHook.dll"
#define VREDExeName "VREDPro.exe"

static bool InjectDLL(HANDLE hProcess, const std::string& dllname)
{
    SIZE_T bytesRet = 0;
    DWORD oldProtect = 0;
    LPVOID remote_addr = nullptr;
    HANDLE hThread = nullptr;
    size_t len = dllname.size() + 1;

    remote_addr = ::VirtualAllocEx(hProcess, 0, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (remote_addr == nullptr) { return false; }
    ::VirtualProtectEx(hProcess, remote_addr, len, PAGE_EXECUTE_READWRITE, &oldProtect);
    ::WriteProcessMemory(hProcess, remote_addr, dllname.c_str(), len, &bytesRet);
    ::VirtualProtectEx(hProcess, remote_addr, len, oldProtect, &oldProtect);

    hThread = ::CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)((void*)&LoadLibraryA), remote_addr, 0, nullptr);
    ::WaitForSingleObject(hThread, INFINITE);
    ::VirtualFreeEx(hProcess, remote_addr, 0, MEM_RELEASE);
    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR cmd, int show)
{
    std::string module_path;
    {
        char buf[2048];
        GetModuleFileNameA(nullptr, buf, sizeof(buf));
        module_path = buf;
        auto spos = module_path.find_last_of("\\");
        if (spos != std::string::npos) {
            module_path.resize(spos);
        }
    }

    std::string exe_path;
    if (__argc > 1) {
        exe_path = __argv[1];
    }
    else {
        exe_path = module_path + "\\" + VREDExeName;
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ::ZeroMemory(&si, sizeof(si));
    ::ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    BOOL ret = ::CreateProcessA(exe_path.c_str(), nullptr, nullptr, nullptr, FALSE,
        NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED, nullptr, nullptr, &si, &pi);
    if (ret) {
        std::string hook_path;
        hook_path += module_path + "\\" + HookDllName;

        //::Sleep(10000); // for debug
        InjectDLL(pi.hProcess, hook_path);
        ::ResumeThread(pi.hThread);
    }
    return 0;
}
