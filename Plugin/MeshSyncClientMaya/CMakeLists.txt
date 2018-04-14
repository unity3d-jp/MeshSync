file(GLOB sources *.cpp *.h)
add_library(MeshSyncClientMaya SHARED ${sources})

add_dependencies(MeshSyncClientMaya MeshUtils MeshSync)
target_include_directories(MeshSyncClientMaya PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(MeshSyncClientMaya
    MeshSync MeshUtils
    ${Poco_Net_LIBRARY} ${Poco_Util_LIBRARY} ${Poco_Foundation_LIBRARY}
)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(MeshSyncClientMaya pthread)
endif()
install(TARGETS MeshSyncClientMaya DESTINATION .)
