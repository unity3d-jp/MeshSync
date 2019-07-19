#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <codecvt>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <future>
#include <mutex>
#include <memory>
#include <cassert>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
// 3ds Max headers
#include <max.h>
#include <modstack.h>
#include <stdmat.h>
#include <imenuman.h>
#include <iskin.h>
#include <iInstanceMgr.h>
#include <triobj.h>
#include <notify.h>
#include <ISceneEventManager.h>
#include <MeshNormalSpec.h>
#include <systemutilities.h>
#include <CS/BipedApi.h>
#include <maxscript/maxscript.h>
#include <maxscript/util/listener.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/macros/define_instantiation_functions.h>
#include <MorpherApi.h>
#include <Scene/IPhysicalCamera.h>
#if MAX_PRODUCT_YEAR_NUMBER >= 2018
    #include <CATAPI/CATClassID.h>
#endif

#ifdef PI
    #undef PI
#endif
