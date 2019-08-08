#include "pch.h"
#include "msDebug.h"

namespace ms {

#ifdef msDbgEnableTimer
DbgTimerScope::DbgTimerScope(const char *mes)
{
    m_message = mes;
    m_begin = mu::Now();
}

DbgTimerScope::~DbgTimerScope()
{
    float elapsed = mu::NS2MS(mu::Now() - m_begin);
    mu::Print("%s - %.2fms\n", m_message, elapsed);
}
#endif // msDbgEnableTimer

} // namespace ms
