# * MQSDK_DIR

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/External")
set(MQSDK_ARCHIVE "${EXTERNAL_DIR}/mqsdk464.zip")
set(MQSDK_DIR "${EXTERNAL_DIR}/mqsdk464/mqsdk")
if(NOT EXISTS "${MQSDK_ARCHIVE}")
    file(DOWNLOAD "http://www.metaseq.net/metaseq/mqsdk464.zip" "${MQSDK_ARCHIVE}" SHOW_PROGRESS)
    execute_process(WORKING_DIRECTORY "${EXTERNAL_DIR}" COMMAND 7za x "${MQSDK_ARCHIVE}")
endif()
