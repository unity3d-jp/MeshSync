#pragma once

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
#include <regex>
#include <future>
#include <mutex>
#include <memory>
#include <cassert>

#ifdef _WIN32
#pragma warning(push, 0)
#include <lxversion.h>
#include <lxvector.h>
#include <lxidef.h>
#include <lx_plugin.hpp>
#include <lx_item.hpp>
#include <lx_mesh.hpp>
#include <lx_locator.hpp>
#include <lx_trisurf.hpp>
#include <lx_envelope.hpp>
#include <lx_package.hpp>
#include <lx_io.hpp>
#include <lx_value.hpp>
#include <lx_log.hpp>
#include <lxu_command.hpp>
#pragma warning(pop)
#endif

#include "MeshSync/MeshSync.h"
#include "MeshSync/MeshSyncUtils.h"
