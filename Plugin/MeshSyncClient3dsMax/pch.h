#pragma once

#define _CRT_SECURE_NO_WARNINGS
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
#include <maxapi.h>
#include <guplib.h>
#include <ISceneEventManager.h>
#include <systemutilities.h>
#include <maxscript/maxscript.h>
#include <maxscript/foundation/numbers.h>
#include <maxscript/foundation/3dmath.h>
#include <maxscript/foundation/name.h>
#include <maxscript/foundation/strings.h>
#include <maxscript/maxwrapper/mxsobjects.h>

#ifdef PI
    #undef PI
#endif
