file(GLOB sources *.cpp *.c *.h)
add_library(MeshSync STATIC ${sources} ${MUISPC_OUTPUTS})
target_include_directories(MeshSync PUBLIC "${CMAKE_SOURCE_DIR}" ${Poco_INCLUDE_DIRS})
