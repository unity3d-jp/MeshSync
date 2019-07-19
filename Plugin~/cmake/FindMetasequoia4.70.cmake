# * MQSDK_DIR

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/External")
set(MQSDK_ARCHIVE "${EXTERNAL_DIR}/mqsdk470.zip")
set(MQSDK_DIR "${EXTERNAL_DIR}/mqsdk470/mqsdk")
if(NOT EXISTS "${MQSDK_ARCHIVE}")
    file(DOWNLOAD "http://www.metaseq.net/metaseq/mqsdk470.zip" "${MQSDK_ARCHIVE}" SHOW_PROGRESS)
    execute_process(WORKING_DIRECTORY "${EXTERNAL_DIR}" COMMAND 7za x -aos -o"mqsdk470" "${MQSDK_ARCHIVE}")
endif()
