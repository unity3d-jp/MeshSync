#pragma once

#ifdef _WIN32

#include <windows.h>
#include <dbghelp.h>
void ReportMiniDump(EXCEPTION_POINTERS* pep);

#define CrashReportScopeBegin __try {
#define CrashReportScopeEnd } __except (ReportMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER) { DebugBreak(); }

#else

#define CrashReportScopeBegin
#define CrashReportScopeEnd

#endif
