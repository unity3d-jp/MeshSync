CMAKE_MINIMUM_REQUIRED(VERSION 3.18)

project(MeshSyncTest)

set(MeshSyncTest_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(MeshSyncTest ${sources})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})
add_dependencies(MeshSyncTest MeshSync)

#includes
target_include_directories(MeshSyncTest PUBLIC 
    ${MeshSyncTest_dir}    
)

target_link_libraries(MeshSyncTest 
    MeshSync 
)

 if(LINUX)
    target_link_libraries(MeshSyncTest "-Wl,--no-undefined")
 endif()

# Copy test data
file(GLOB_RECURSE test_data ${MeshSyncTest_dir}/Data/*.* )
file(COPY ${test_data} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


set_target_properties(MeshSyncTest PROPERTIES
    XCODE_GENERATE_SCHEME TRUE
    XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

