file(GLOB sources *.cpp *.h Utils/*.cpp Utils/*.h Cache/*.cpp Cache/*.h)
add_library(MeshSync STATIC ${sources} ${MUISPC_OUTPUTS})
target_include_directories(MeshSync PUBLIC "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/MeshSync" ${Poco_INCLUDE_DIRS})
