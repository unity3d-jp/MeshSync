# * BLENDER2.80_PYTHON_VERSION
# * BLENDER2.80_PYTHON_INCLUDE_DIR

set(BLENDER2.80_PYTHON_VERSION 37 CACHE STRING "")
mark_as_advanced(BLENDER2.80_PYTHON_VERSION)

find_path(BLENDER2.80_PYTHON_INCLUDE_DIR
    NAMES
        "Python.h"
    PATHS
        "/opt/rh/rh-python37/root/usr/include"
        "/usr/local/include"
    PATH_SUFFIXES
        "python3.7m"
    NO_DEFAULT_PATH
)
mark_as_advanced(BLENDER2.80_PYTHON_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Blender"
    DEFAULT_MSG
    BLENDER2.80_PYTHON_INCLUDE_DIR
)
