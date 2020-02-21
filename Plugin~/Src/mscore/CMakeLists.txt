CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(mscore)
file(GLOB_RECURSE sources *.cpp *.h)
set(plugins_dir "${CMAKE_SOURCE_DIR}/../Runtime/Plugins/x86_64")
set(src_root   "${CMAKE_SOURCE_DIR}/Src")
set(mscore_dir "${CMAKE_CURRENT_SOURCE_DIR}")

add_plugin(mscore SOURCES ${sources} PLUGINS_DIR ${plugins_dir})

#Include, pch
target_include_directories(mscore PRIVATE 
    ${src_root}
    ${mscore_dir}
)
target_include_directories(mscore PRIVATE )
target_precompile_headers(mscore PRIVATE "${mscore_dir}/pch.h")

# Library dependencies
target_link_libraries(
    mscore 
    MeshSync 
    MeshUtils 
    ${Poco_LIBRARIES}
)

if(ENABLE_ZSTD)
    target_link_libraries(mscore ${ZSTD_LIBRARY})
endif()

#if(LINUX)
#    target_link_libraries(mscore pthread "-Wl,--no-undefined")
#endif()
