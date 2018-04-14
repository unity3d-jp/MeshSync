file(GLOB sources *.cpp *.h)
add_library(MeshSyncClientMQ4 SHARED ${sources})

add_dependencies(MeshSyncClientMQ4 MeshUtils MeshSync)
target_include_directories(MeshSyncClientMQ4 PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(MeshSyncClientMQ4 MeshSync MeshUtils ${Poco_LIBRARIES})
install(TARGETS MeshSyncClientMQ4 DESTINATION .)

