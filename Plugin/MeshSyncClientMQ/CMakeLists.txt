set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/External")
set(MQSDK_ARCHIVE "${EXTERNAL_DIR}/mqsdk464.zip")
set(MQSDK_DIR "${EXTERNAL_DIR}/mqsdk464/mqsdk")
if(NOT EXISTS "${MQSDK_ARCHIVE}")
    file(DOWNLOAD "http://www.metaseq.net/metaseq/mqsdk464.zip" "${MQSDK_ARCHIVE}" SHOW_PROGRESS)
    execute_process(WORKING_DIRECTORY "${EXTERNAL_DIR}" COMMAND 7za x "${MQSDK_ARCHIVE}")
endif()

file(GLOB sources *.cpp *.h)
list(FILTER sources EXCLUDE REGEX "MeshSyncClientMQ3\\.(h|cpp)") 
list(APPEND sources
    "${MQSDK_DIR}/MQBasePlugin.cpp"
    "${MQSDK_DIR}/MQBoneManager.cpp"
    "${MQSDK_DIR}/MQInit.cpp"
    "${MQSDK_DIR}/MQPlugin.cpp"
    "${MQSDK_DIR}/MQSetting.cpp"
    "${MQSDK_DIR}/MQWidget.cpp"
)
if(MAC)
    if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/StringUtil.mm")
        file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/StringUtil.mm"
            "#import <Foundation/Foundation.h>\n"
            "#include \"${MQSDK_DIR}/osx/StringUtil.mm\"\n")
    endif()
    list(APPEND sources "${CMAKE_CURRENT_BINARY_DIR}/StringUtil.mm")
endif()

add_library(MeshSyncClientMQ4 MODULE ${sources})
set_target_properties(MeshSyncClientMQ4 PROPERTIES BUNDLE ON)

add_dependencies(MeshSyncClientMQ4 MeshUtils MeshSync)
target_include_directories(MeshSyncClientMQ4 PUBLIC "${CMAKE_SOURCE_DIR}" "${MQSDK_DIR}")
target_link_libraries(MeshSyncClientMQ4
    MeshSync MeshUtils
    ${Poco_LIBRARIES} ${COCOA_LIBRARY})

set(DIST_DIR "${CMAKE_INSTALL_PREFIX}/UnityMeshSync_Metasequoia_${PLATFORM}")
install(CODE "execute_process(COMMAND mkdir -p \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
install(CODE "execute_process(COMMAND rm -rf \"${DIST_DIR}/MeshSyncClientMQ4.plugin/\")")
install(CODE "execute_process(COMMAND cp -r \"${CMAKE_CURRENT_BINARY_DIR}/MeshSyncClientMQ4.bundle\" \"${DIST_DIR}/MeshSyncClientMQ4.plugin\")")
