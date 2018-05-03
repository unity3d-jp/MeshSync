# * BLENDER2.8_PYTHON_VERSION
# * BLENDER2.8_PYTHON_INCLUDE_DIR

set(BLENDER2.8_PYTHON_VERSION 36 CACHE STRING "")
mark_as_advanced(BLENDER2.8_PYTHON_VERSION)
find_path(BLENDER2.8_PYTHON_INCLUDE_DIR
    NAMES
        "Python.h"
    PATHS
        "/opt/rh/rh-python36/root/usr/include"
        "/usr/local/include"
    PATH_SUFFIXES
        "python3.6m"
    NO_DEFAULT_PATH
)
mark_as_advanced(BLENDER2.8_PYTHON_INCLUDE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Blender"
    DEFAULT_MSG
    BLENDER2.8_PYTHON_INCLUDE_DIR
)
