string(REGEX MATCH "([0-9.]+)$" MAYA_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_MAYA "MeshSyncClientMaya${MAYA_VERSION}")

if(${MAYA_VERSION} STREQUAL "2018")
    find_package(Maya2018)
    set(MAYA_INCLUDE_DIR ${MAYA2018_INCLUDE_DIR})
    set(MAYA_LIBRARIES ${MAYA2018_LIBRARIES})
elseif(${MAYA_VERSION} STREQUAL "2017")
    find_package(Maya2017)
    set(MAYA_INCLUDE_DIR ${MAYA2017_INCLUDE_DIR})
    set(MAYA_LIBRARIES ${MAYA2017_LIBRARIES})
elseif(${MAYA_VERSION} STREQUAL "2016.5")
    find_package(Maya2016.5)
    set(MAYA_INCLUDE_DIR ${MAYA2016_5_INCLUDE_DIR})
    set(MAYA_LIBRARIES ${MAYA2016_5_LIBRARIES})
endif()

file(GLOB sources *.cpp *.h)
add_library(${MESHSYNC_CLIENT_MAYA} SHARED ${sources})
add_dependencies(${MESHSYNC_CLIENT_MAYA} MeshUtils MeshSync)
target_include_directories(${MESHSYNC_CLIENT_MAYA} PUBLIC "${CMAKE_SOURCE_DIR}" "${MAYA_INCLUDE_DIR}")
target_link_libraries(${MESHSYNC_CLIENT_MAYA}
    MeshSync MeshUtils
    ${Poco_Net_LIBRARY} ${Poco_Util_LIBRARY} ${Poco_Foundation_LIBRARY}
    ${MAYA_LIBRARIES}
)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(${MESHSYNC_CLIENT_MAYA} pthread)
endif()
install(TARGETS ${MESHSYNC_CLIENT_MAYA} DESTINATION .)
