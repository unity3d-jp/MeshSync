CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(MeshSyncTest)

set(src_root   "${CMAKE_SOURCE_DIR}/Src")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")

file(GLOB sources *.cpp *.h)
add_executable(MeshSyncTest ${sources})
add_dependencies(MeshSyncTest MeshSync MeshUtils)

#includes
target_include_directories(MeshSyncTest PUBLIC 
    "${CMAKE_SOURCE_DIR}"
    ${src_root}
)

target_link_libraries(MeshSyncTest MeshSync MeshUtils ${Poco_LIBRARIES})
if(ENABLE_ZSTD)
    target_link_libraries(MeshSyncTest ${ZSTD_LIBRARY})
endif()
if(LINUX)
    target_link_libraries(MeshSyncTest pthread "-Wl,--no-undefined")
endif()
