autodesk/# * PYTHON_INCLUDE_DIR
# * PYTHON_LIBRARY

set(CMAKE_PREFIX_PATH
    "/opt/autodesk/maya2018"
)

find_path(MAYA_INCLUDE_DIR NAMES maya/MGlobal.h)
mark_as_advanced(MAYA_INCLUDE_DIR)

find_library(MAYA_LIBRARIES NAMES OpenMaya OpenMayaAnim OpenMayaFx OpenMayaRender OpenMayaUI PATHS ${LIBRARY_PATHS} PATH_SUFFIXES lib64 lib)
mark_as_advanced(MAYA_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Maya2018"
    DEFAULT_MSG
    MAYA_INCLUDE_DIR
    MAYA_LIBRARIES
)
