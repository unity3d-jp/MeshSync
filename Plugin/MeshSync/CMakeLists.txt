file(GLOB sources *.cpp *.h Utils/*.cpp Utils/*.h SceneCache/*.cpp SceneCache/*.h SceneGraph/*.cpp SceneGraph/*.h)
add_library(MeshSync STATIC ${sources} ${MUISPC_OUTPUTS})
target_include_directories(MeshSync PUBLIC "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/MeshSync" ${Poco_INCLUDE_DIRS})
