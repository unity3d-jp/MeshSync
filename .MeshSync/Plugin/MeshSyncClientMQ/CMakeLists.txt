string(REGEX MATCH "([0-9.]+)$" MQ_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_METASEQUOIA "MeshSyncClientMQ${MQ_VERSION}")
find_package(Metasequoia${MQ_VERSION})

file(GLOB sources *.cpp *.h *.mm)
list(FILTER sources EXCLUDE REGEX "MeshSyncClientMQ3\\.(h|cpp)") 

add_library(${MESHSYNC_CLIENT_METASEQUOIA} MODULE ${sources})
set_target_properties(${MESHSYNC_CLIENT_METASEQUOIA} PROPERTIES BUNDLE ON)
set_target_properties(${MESHSYNC_CLIENT_METASEQUOIA} PROPERTIES OUTPUT_NAME "MeshSyncClientMQ4")

add_dependencies(${MESHSYNC_CLIENT_METASEQUOIA} MeshUtils MeshSync)
target_include_directories(${MESHSYNC_CLIENT_METASEQUOIA} PUBLIC "${CMAKE_SOURCE_DIR}" "${MQSDK_DIR}")
target_link_libraries(${MESHSYNC_CLIENT_METASEQUOIA}
    MeshSync MeshUtils
    ${Poco_LIBRARIES} ${COCOA_LIBRARY})

set(DIST_DIR "${CMAKE_INSTALL_PREFIX}/UnityMeshSync_Metasequoia_${PLATFORM}/Metasequoia${MQ_VERSION}")
install(CODE "execute_process(COMMAND mkdir -p \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
install(CODE "execute_process(COMMAND rm -rf \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
install(CODE "execute_process(COMMAND cp -r \"${CMAKE_CURRENT_BINARY_DIR}/MeshSyncClientMQ4.bundle\" \"${DIST_DIR}/MeshSyncClientMQ4.plugin\")")
