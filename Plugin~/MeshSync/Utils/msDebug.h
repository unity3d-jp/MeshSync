#pragma once
#include "MeshUtils/MeshUtils.h"

#ifdef msDebug
//#define msDbgEnableProfile
#endif

namespace ms {

#ifdef msDbgEnableProfile
    #define msProfileScope(Message, ...) mu::ProfileTimer _prof_timer(Message, __VA_ARGS__)
#else // msDbgEnableProfile
    #define msProfileScope(Message, ...)
#endif // msDbgEnableProfile

} // namespace ms
