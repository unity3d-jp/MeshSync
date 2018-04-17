# * BLENDER2.8_PYTHON_VERSION
# * BLENDER2.8_PYTHON_INCLUDE_DIR

set(CMAKE_PREFIX_PATH
    "/opt/rh/rh-python36/root/usr"
    "/usr/local/include"
)

set(BLENDER2.8_PYTHON_VERSION 36 CACHE STRING "")
find_path(BLENDER2.8_PYTHON_INCLUDE_DIR NAMES Python.h PATH_SUFFIXES python3.6m)
mark_as_advanced(BLENDER2.8_PYTHON_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Blender"
    DEFAULT_MSG
    BLENDER2.8_PYTHON_INCLUDE_DIR
)
