find_package(Modo)
set(MESHSYNC_CLIENT_MODO "MeshSyncClientModo")

file(GLOB sources *.cpp *.h "${MODO_SDK_DIR}/common/*.cpp")
list(FILTER sources EXCLUDE REGEX "initialize.cpp") 
list(FILTER sources EXCLUDE REGEX "clean.cpp") 
add_library(${MESHSYNC_CLIENT_MODO} SHARED ${sources})
add_dependencies(${MESHSYNC_CLIENT_MODO} MeshUtils MeshSync)
target_include_directories(${MESHSYNC_CLIENT_MODO} PUBLIC
    "${CMAKE_SOURCE_DIR}"
    "${MODO_SDK_DIR}/include"
    ${MODO_QT_INCLUDE_DIRS}
)
target_link_libraries(${MESHSYNC_CLIENT_MODO} MeshSync MeshUtils ${Poco_LIBRARIES} ${MODO_QT_LIBRARIES})
if(LINUX)
    target_link_libraries(${MESHSYNC_CLIENT_MODO} pthread "-Wl,--no-undefined")
endif()
set_target_properties(${MESHSYNC_CLIENT_MODO} PROPERTIES PREFIX "")
set_target_properties(${MESHSYNC_CLIENT_MODO} PROPERTIES SUFFIX ".lx")
set_target_properties(${MESHSYNC_CLIENT_MODO} PROPERTIES OUTPUT_NAME "MeshSyncClientModo")

set(DIST_DIR "${CMAKE_INSTALL_PREFIX}/UnityMeshSync_Modo_${PLATFORM}")
install(TARGETS ${MESHSYNC_CLIENT_MODO} DESTINATION "${DIST_DIR}/")
