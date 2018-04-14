file(GLOB sources *.cpp *.h)
add_library(MeshSyncClientBlender  SHARED ${sources})

add_dependencies(MeshSyncClientBlender MeshUtils MeshSync)
target_include_directories(MeshSyncClientBlender PUBLIC "${CMAKE_SOURCE_DIR}" PyBind11)
target_link_libraries(MeshSyncClientBlender
    MeshSync MeshUtils
    ${Poco_Net_LIBRARY} ${Poco_Util_LIBRARY} ${Poco_Foundation_LIBRARY}
)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(MeshSyncClientBlender pthread)
endif()
install(TARGETS MeshSyncClientBlender DESTINATION .)
