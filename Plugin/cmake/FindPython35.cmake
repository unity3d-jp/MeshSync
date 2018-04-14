# * PYTHON_INCLUDE_DIR
# * PYTHON_LIBRARY

set(CMAKE_PREFIX_PATH
    "/opt/rh/rh-python35/root/usr"
)

find_path(PYTHON_INCLUDE_DIR NAMES Python.h)
mark_as_advanced(PYTHON_INCLUDE_DIR)

find_library(PYTHON_LIBRARY NAMES libpython3.5m PATHS ${LIBRARY_PATHS} PATH_SUFFIXES lib64 lib)
mark_as_advanced(PYTHON_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Python"
    DEFAULT_MSG
    PYTHON_LIBRARY
    PYTHON_INCLUDE_DIR
)
