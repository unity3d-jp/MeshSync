set(MODO_SDK_DIR "${CMAKE_SOURCE_DIR}/External/LXSDK_525410" CACHE PATH "Modo SDK directory")
set(MODO_QT_DIR "${CMAKE_SOURCE_DIR}/External/Qt/4.8.5" CACHE PATH "Modo Qt directory")
mark_as_advanced(FORCE MODO_SDK_DIR)
mark_as_advanced(FORCE MODO_QT_DIR)
find_package(Modo)

set(MESHSYNC_CLIENT_MODO "MeshSyncClientModo")

file(GLOB sources *.cpp *.h "${MODO_SDK_DIR}/common/*.cpp")
list(FILTER sources EXCLUDE REGEX "initialize.(h|cpp)") 
add_library(${MESHSYNC_CLIENT_MODO} SHARED ${sources})
add_dependencies(${MESHSYNC_CLIENT_MODO} MeshUtils MeshSync)
target_include_directories(${MESHSYNC_CLIENT_MODO} PUBLIC
    "${CMAKE_SOURCE_DIR}"
    "${MODO_SDK_DIR}/include"
    "${MODO_QT_DIR}/include"
    "${MODO_QT_DIR}/include/QtCore"
    "${MODO_QT_DIR}/include/QtGui"
    "${MODO_QT_DIR}/include/Wedgets"
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
