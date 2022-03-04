#include "pch.h"
#include "MeshUtils/muLog.h"

namespace mu { 

void Print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    ::OutputDebugStringA(buf);
#else
    vprintf(fmt, args);
#endif
    va_end(args);
}

void Print(const wchar_t *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    wchar_t buf[1024];
    _vsnwprintf(buf, sizeof(buf), fmt, args);
    ::OutputDebugStringW(buf);
#else
    vwprintf(fmt, args);
#endif
    va_end(args);
}

} // namespace mu
