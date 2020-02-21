#include "muDebugTimer.h"
#include "pch.h"
#include "muDebugTimer.h"

namespace mu {

ScopedTimer::ScopedTimer()
{
    m_begin = Now();
}

float ScopedTimer::elapsed() const
{
    return NS2MS(Now() - m_begin);
}


ProfileTimer::ProfileTimer(const char *mes, ...)
{
    va_list args;
    va_start(args, mes);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), mes, args);
    va_end(args);

    m_message = buf;
}

ProfileTimer::~ProfileTimer()
{
    float t = elapsed();
    Print("%s - %.2fms\n", m_message.c_str(), t);
}

} // namespace mu
