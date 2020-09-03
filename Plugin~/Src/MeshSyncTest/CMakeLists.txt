CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project(MeshSyncTest)

set(MeshSyncTest_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(src_root        "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(MeshSyncTest ${sources})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})
add_dependencies(MeshSyncTest MeshSync MeshUtils)

#includes
target_include_directories(MeshSyncTest PUBLIC 
    "${CMAKE_SOURCE_DIR}"
    ${MeshSyncTest_dir}    
    ${src_root}
    ${Poco_INCLUDE_DIRS}
)

target_link_libraries(MeshSyncTest 
    MeshSync 
    MeshUtils 
    ${Poco_LIBRARIES}
)

if(ENABLE_ZSTD)
    target_link_libraries(MeshSyncTest ${ZSTD_LIBRARY})
endif()
if(LINUX)
    target_link_libraries(MeshSyncTest pthread "-Wl,--no-undefined")
endif()

# Copy test data
file(GLOB_RECURSE test_data ${MeshSyncTest_dir}/Data/*.* )
file(COPY ${test_data} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
