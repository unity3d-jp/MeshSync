#pragma once

#include "MeshUtils/MeshUtils.h"
#include "msConfig.h"
#include "SceneGraph/msAsset.h"
#include "SceneGraph/msAnimation.h"
#include "SceneGraph/msMaterial.h"
#include "SceneGraph/msTexture.h"
#include "SceneGraph/msAudio.h"
#include "SceneGraph/msEntity.h"
#include "SceneGraph/msMesh.h"
#include "SceneGraph/msConstraints.h"
#include "SceneGraph/msSceneGraph.h"
#include "SceneCache/msSceneCache.h"
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
