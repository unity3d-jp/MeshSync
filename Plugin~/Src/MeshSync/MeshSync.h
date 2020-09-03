#pragma once

#include "MeshSync/msFoundation.h" //msDeclPtr

namespace ms {

class Transform;
msDeclPtr(Transform);

} // end namespace ms

//#ifdef mscDebug
//    #define mscTrace(...) ::mu::Print("MeshSync trace: " __VA_ARGS__)
//    #define mscTraceW(...) ::mu::Print(L"MeshSync trace: " __VA_ARGS__)
//#else
//    #define mscTrace(...)
//    #define mscTraceW(...)
//#endif
