string(REGEX MATCH "([0-9.]+)$" BLENDER_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MESHSYNC_CLIENT_BLENDER "MeshSyncClientBlender${BLENDER_VERSION}")

find_package(Blender${BLENDER_VERSION})
set(BLENDER_PYTHON_VERSION ${BLENDER${BLENDER_VERSION}_PYTHON_VERSION})
set(BLENDER_PYTHON_INCLUDE_DIR ${BLENDER${BLENDER_VERSION}_PYTHON_INCLUDE_DIR})
string(REPLACE "." "_" BLENDER_MODULE_BLENDER_VERSION ${BLENDER_VERSION})

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/External")
set(EXTERNAL_ARCHIVE "${EXTERNAL_DIR}/External.7z")
if(NOT EXISTS "${EXTERNAL_ARCHIVE}")
    file(DOWNLOAD "https://github.com/unity3d-jp/MeshSync/releases/download/20171228/External.7z" "${EXTERNAL_ARCHIVE}" SHOW_PROGRESS)
    execute_process(WORKING_DIRECTORY "${EXTERNAL_DIR}" COMMAND 7za x "${EXTERNAL_ARCHIVE}")
endif()

set(BLENDER_INCLUDE_DIRS
    "${EXTERNAL_DIR}/blender-${BLENDER_VERSION}/include/blenkernel"
    "${EXTERNAL_DIR}/blender-${BLENDER_VERSION}/include/blenlib"
    "${EXTERNAL_DIR}/blender-${BLENDER_VERSION}/include/bmesh"
    "${EXTERNAL_DIR}/blender-${BLENDER_VERSION}/include/makesdna"
    "${EXTERNAL_DIR}/blender-${BLENDER_VERSION}/include/makesrna"
    "${EXTERNAL_DIR}/blender-${BLENDER_VERSION}/include/python"
    "${BLENDER_PYTHON_INCLUDE_DIR}"
)

file(GLOB sources *.cpp *.h *.c)
add_library(${MESHSYNC_CLIENT_BLENDER}  SHARED ${sources})

add_dependencies(${MESHSYNC_CLIENT_BLENDER} MeshUtils MeshSync)
target_include_directories(${MESHSYNC_CLIENT_BLENDER} PUBLIC
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/PyBind11"
    ${BLENDER_INCLUDE_DIRS}
)
target_link_libraries(${MESHSYNC_CLIENT_BLENDER} MeshSync MeshUtils ${Poco_LIBRARIES})
if(LINUX)
    target_link_libraries(${MESHSYNC_CLIENT_BLENDER} pthread)
    set_target_properties(${MESHSYNC_CLIENT_BLENDER} PROPERTIES SUFFIX ".cpython-${BLENDER_PYTHON_VERSION}m-x86_64-linux-gnu.so")
elseif(MAC)
    target_link_libraries(${MESHSYNC_CLIENT_BLENDER} "-undefined dynamic_lookup")
    set_target_properties(${MESHSYNC_CLIENT_BLENDER} PROPERTIES SUFFIX ".cpython-${BLENDER_PYTHON_VERSION}m-darwin.so")
endif()
set_target_properties(${MESHSYNC_CLIENT_BLENDER} PROPERTIES PREFIX "")
set_target_properties(${MESHSYNC_CLIENT_BLENDER} PROPERTIES OUTPUT_NAME "MeshSyncClientBlender")

set(DIST_DIR "${CMAKE_INSTALL_PREFIX}/UnityMeshSync_Blender_${PLATFORM}/blender-${BLENDER_VERSION}")
set(PYD_DIR "${DIST_DIR}/MeshSyncClientBlender")
install(FILES
    python/unity_mesh_sync_common.py
    python/${BLENDER_VERSION}/unity_mesh_sync.py
    DESTINATION ${DIST_DIR})
install(FILES python/__init__.py DESTINATION ${PYD_DIR})
install(TARGETS ${MESHSYNC_CLIENT_BLENDER} DESTINATION ${PYD_DIR})
