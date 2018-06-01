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
#include <modstack.h>
#include <triobj.h>
#include <notify.h>
#include <ISceneEventManager.h>
#include <systemutilities.h>
#include <maxscript/maxscript.h>
#include <maxscript/macros/define_instantiation_functions.h>

#ifdef PI
    #undef PI
#endif
