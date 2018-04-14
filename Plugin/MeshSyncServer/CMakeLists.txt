file(GLOB sources *.cpp *.h)
set(plugins_dir "${CMAKE_SOURCE_DIR}/../MeshSync/Assets/UTJ/MeshSync/Plugins/x86_64")
add_plugin(MeshSyncServer SOURCES ${sources} PLUGINS_DIR ${plugins_dir})

add_dependencies(MeshSyncServer MeshUtils)
target_include_directories(MeshSyncServer PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(MeshSyncServer
    MeshSync MeshUtils
    ${Poco_Net_LIBRARY} ${Poco_Util_LIBRARY} ${Poco_Foundation_LIBRARY}
)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(MeshSyncServer pthread)
endif()
install(TARGETS MeshSyncServer DESTINATION .)
