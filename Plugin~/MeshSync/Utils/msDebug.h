#pragma once
#include "MeshUtils/MeshUtils.h"

#ifdef msDebug
//#define msDbgEnableTimer
#endif

namespace ms {

#ifdef msDbgEnableTimer
    #define msDbgTimer(Message, ...) mu::ScopedTimer _dbg_timer(Message, __VA_ARGS__)
#else // msDbgEnableTimer
    #define msDbgTimer(Message, ...)
#endif // msDbgEnableTimer

} // namespace ms
