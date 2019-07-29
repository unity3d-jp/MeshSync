#pragma once
#include "MeshUtils/MeshUtils.h"

#ifdef msDebug
//#define msDbgEnableTimer
#endif

namespace ms {

#ifdef msDbgEnableTimer
    class DbgTimerScope
    {
    public:
        DbgTimerScope(const char *mes);
        ~DbgTimerScope();

    private:
        const char *m_message;
        mu::nanosec m_begin;
    };

    #define msDbgTimer(Message) DbgTimerScope _dbg_timer(Message)

#else // msDbgEnableTimer

    #define msDbgTimer(Message)

#endif // msDbgEnableTimer

} // namespace ms
