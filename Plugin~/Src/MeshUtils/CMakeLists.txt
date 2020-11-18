set(MeshUtils_dir "${CMAKE_CURRENT_SOURCE_DIR}")
option(ENABLE_TBB "Use Intel TBB." OFF)

project(MeshUtils)

if(ENABLE_ISPC)
    setup_ispc()
    set(MUISPC_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/ISPC)
    set(MUISPC_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/ispcmath.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/muSIMDConfig.h"
    )
    file(GLOB MUISPC_SOURCES *.ispc)
    add_ispc_targets(SOURCES ${MUISPC_SOURCES} HEADERS ${MUISPC_HEADERS} OUTDIR ${MUISPC_OUTDIR})
    set(MUISPC_OUTPUTS ${_ispc_outputs})
endif()

file(GLOB_RECURSE sources *.cpp *.c *.h)
add_library(MeshUtils STATIC ${sources} ${MUISPC_OUTPUTS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

#pch
target_precompile_headers(MeshUtils PRIVATE "${MeshUtils_dir}/pch.h")

list(APPEND MeshUtils_include 
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

target_compile_definitions(MeshUtils
    PRIVATE
        muEnableISPC
)

set(src_root "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories(MeshUtils 
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
        "${src_root}"        
    PRIVATE 
        ${MUISPC_OUTDIR}
)


if(LINUX)
    target_link_libraries(MeshUtils INTERFACE pthread "-Wl,--no-undefined")
endif()

if(ENABLE_TBB)
    find_package(TBB QUIET)
    add_definitions(-DmuEnableTBB)
    include_directories(${TBB_INCLUDE_DIRS})
    list(APPEND EXTERNAL_LIBS ${TBB_LIBRARIES})
endif()
