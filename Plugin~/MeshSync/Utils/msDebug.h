#pragma once
#include "MeshUtils/MeshUtils.h"

#ifdef msDebug
//#define msDbgEnableProfile
#endif

namespace ms {

#ifdef msDbgEnableProfile
    #define msDbgTimer(Message, ...) mu::ScopedTimer _dbg_timer(Message, __VA_ARGS__)
#else // msDbgEnableProfile
    #define msDbgTimer(Message, ...)
#endif // msDbgEnableProfile

} // namespace ms
