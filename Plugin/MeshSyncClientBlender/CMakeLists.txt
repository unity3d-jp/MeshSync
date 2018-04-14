string(REGEX MATCH "([0-9.]+)$" BLENDER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_BLENDER "MeshSyncClientBlender${BLENDER_VERSION}")

set(LIBRARIES )
if(${BLENDER_VERSION} STREQUAL "2.79")
    find_package(Blender2.79)
elseif(${BLENDER_VERSION} STREQUAL "2.8")
    find_package(Blender2.8)
endif()

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/External")
set(EXTERNAL_ARCHIVE "${EXTERNAL_DIR}/External.7z")
if(NOT EXISTS "${EXTERNAL_ARCHIVE}")
    file(DOWNLOAD "https://github.com/unity3d-jp/MeshSync/releases/download/20171228/External.7z" "${EXTERNAL_ARCHIVE}" SHOW_PROGRESS)
    execute_process(WORKING_DIRECTORY "${EXTERNAL_DIR}" COMMAND 7za x "${EXTERNAL_ARCHIVE}")
endif()

file(GLOB sources *.cpp *.h)
add_library(${MESHSYNC_CLIENT_BLENDER}  SHARED ${sources})

add_dependencies(${MESHSYNC_CLIENT_BLENDER} MeshUtils MeshSync)
target_include_directories(${MESHSYNC_CLIENT_BLENDER} PUBLIC
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/PyBind11"
    "${PYTHON_INCLUDE_DIR}"
    "${EXTERNAL_DIR}/blender-${BLENDER_VERSION}"
)
target_link_libraries(${MESHSYNC_CLIENT_BLENDER} MeshSync MeshUtils ${Poco_LIBRARIES} ${PYTHON_LIBRARY})
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    target_link_libraries(${MESHSYNC_CLIENT_BLENDER} pthread)
endif()
install(TARGETS ${MESHSYNC_CLIENT_BLENDER} DESTINATION .)
