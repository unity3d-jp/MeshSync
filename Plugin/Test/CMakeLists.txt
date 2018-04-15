set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}")

file(GLOB sources *.cpp *.h)
add_executable(Test ${sources})
add_dependencies(Test MeshUtils)
target_include_directories(Test PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(Test MeshUtils ${EXTERNAL_LIBS})
