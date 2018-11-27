#pragma once

#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"
#include "msSceneGraph.h"
#include "msSceneCache.h"
#include "msConstraints.h"
#include "msAnimation.h"
#include "msMaterial.h"
#include "msAudio.h"
#include "msClient.h"
#include "msServer.h"
#include "msMisc.h"

#ifdef mscDebug
    #define mscTrace(...) ::mu::Print("MeshSync trace: " __VA_ARGS__)
    #define mscTraceW(...) ::mu::Print(L"MeshSync trace: " __VA_ARGS__)
#else
    #define mscTrace(...)
    #define mscTraceW(...)
#endif
