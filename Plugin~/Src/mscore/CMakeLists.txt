CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(mscore)
file(GLOB_RECURSE sources *.cpp *.h)
set(mscore_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(plugins_dir "${SRC_ROOT}/../../Runtime/Plugins/x86_64")

add_plugin(mscore SOURCES ${sources} PLUGINS_DIR ${plugins_dir})

#Include, pch
target_include_directories(mscore PRIVATE 
    ${SRC_ROOT}
    ${mscore_dir}
)
target_include_directories(mscore PRIVATE )
target_precompile_headers(mscore PRIVATE "${mscore_dir}/pch.h")

# Dependencies on Windows
if(WIN32)
    set(Poco_Win_dependencies "ws2_32.lib;Iphlpapi.lib")
endif()

# Library dependencies
target_link_libraries( mscore 
    MeshSync 
    MeshUtils 
    ${Poco_LIBRARIES}
    ${Poco_Win_dependencies}
)

if(ENABLE_ZSTD)
    target_link_libraries(mscore ${ZSTD_LIBRARY})
endif()

if(LINUX)
    target_link_libraries(mscore pthread "-Wl,--no-undefined")
endif()
