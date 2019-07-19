#include "pch.h"
#include "CrashReporter.h"

#ifdef _WIN32
static BOOL CALLBACK cbMiniDump(
    PVOID                            /*pParam*/,
    const PMINIDUMP_CALLBACK_INPUT   pInput,
    PMINIDUMP_CALLBACK_OUTPUT        pOutput
)
{
    if (!pInput || !pOutput)
        return FALSE;

    BOOL bRet = FALSE;
    switch (pInput->CallbackType)
    {
    case IncludeModuleCallback:
        // Include the module into the dump 
        bRet = TRUE;
        break;

    case IncludeThreadCallback:
        // Include the thread into the dump 
        bRet = TRUE;
        break;

    case ModuleCallback:
        // Does the module have ModuleReferencedByMemory flag set ? 
        if (!(pOutput->ModuleWriteFlags & ModuleReferencedByMemory))
        {
            // No, it does not - exclude it 
            // pInput->Module.FullPath
            pOutput->ModuleWriteFlags &= (~ModuleWriteModule);
        }
        bRet = TRUE;
        break;

    case ThreadCallback:
        // Include all thread information into the minidump 
        bRet = TRUE;
        break;

    case ThreadExCallback:
        // Include this information 
        bRet = TRUE;
        break;

    case MemoryCallback:
        // We do not include any information here -> return FALSE 
        bRet = FALSE;
        break;

    case CancelCallback:
        break;
    }

    return bRet;
}

void ReportMiniDump(EXCEPTION_POINTERS* pep)
{
    char filename[128];
    sprintf(filename, "%x.dmp", (uint32_t)::time(nullptr));

    HANDLE hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != nullptr && hFile != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = pep;
        mdei.ClientPointers = FALSE;

        MINIDUMP_CALLBACK_INFORMATION mci;
        mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)cbMiniDump;
        mci.CallbackParam = 0;

        MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);

        // create minidump 
        BOOL rv = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (pep ? &mdei : nullptr), nullptr, &mci);
        CloseHandle(hFile);

        if (rv) {
            // todo: send dmp file
        }
    }
}

#endif
