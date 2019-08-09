#include "pch.h"
#include "muDebugTimer.h"

namespace mu {

ScopedTimer::ScopedTimer(const char *mes, ...)
{
    va_list args;
    va_start(args, mes);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), mes, args);
    va_end(args);

    m_message = buf;
    m_begin = Now();
}

ScopedTimer::~ScopedTimer()
{
    float elapsed = NS2MS(Now() - m_begin);
    Print("%s - %.2fms\n", m_message.c_str(), elapsed);
}

} // namespace mu
