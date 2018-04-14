string(REGEX MATCH "([0-9.]+)$" MAYA_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_MAYA "MeshSyncClientMaya${MAYA_VERSION}")

file(GLOB sources *.cpp *.h)
add_library(${MESHSYNC_CLIENT_MAYA} SHARED ${sources})
add_dependencies(${MESHSYNC_CLIENT_MAYA} MeshUtils MeshSync)
target_include_directories(${MESHSYNC_CLIENT_MAYA} PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(${MESHSYNC_CLIENT_MAYA}
    MeshSync MeshUtils
    ${Poco_Net_LIBRARY} ${Poco_Util_LIBRARY} ${Poco_Foundation_LIBRARY}
)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(${MESHSYNC_CLIENT_MAYA} pthread)
endif()
install(TARGETS ${MESHSYNC_CLIENT_MAYA} DESTINATION .)
