CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(mscore)
file(GLOB_RECURSE sources *.cpp *.h)
set(mscore_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(plugins_dir "${SRC_ROOT}/../../Runtime/Plugins/x86_64")

add_plugin(mscore SOURCES ${sources} PLUGINS_DIR ${plugins_dir})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

#Include, pch
target_include_directories(mscore PRIVATE 
    ${SRC_ROOT}/MeshSync/Include
    ${SRC_ROOT}/MeshUtils/Include
    ${SRC_ROOT}
    ${mscore_dir}
)
target_include_directories(mscore PRIVATE )
target_precompile_headers(mscore PRIVATE "${mscore_dir}/pch.h")

# Library dependencies
target_link_libraries( mscore 
    MeshSync 
    MeshUtils 
    ${Poco_LIBRARIES}
)

if(ENABLE_ZSTD)
    target_link_libraries(mscore ${ZSTD_LIBRARY})
endif()

if(LINUX)
    target_link_libraries(mscore pthread "-Wl,--no-undefined")
endif()
