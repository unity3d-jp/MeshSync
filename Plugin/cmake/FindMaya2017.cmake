# * MAYA_INCLUDE_DIR
# * MAYA_LIBRARIES

set(CMAKE_PREFIX_PATH
    "/opt/autodesk/maya2017"
)

find_path(MAYA2017_INCLUDE_DIR NAMES maya/MGlobal.h)
foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation tbb)
    find_library(MAYA2017_${MAYA_LIB}_LIBRARY NAMES ${MAYA_LIB} PATHS ${LIBRARY_PATHS} PATH_SUFFIXES lib)
    list(APPEND MAYA2017_LIBRARIES ${MAYA_${MAYA_LIB}_LIBRARY})
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Maya2017"
    DEFAULT_MSG
    MAYA2017_INCLUDE_DIR
    MAYA2017_LIBRARIES
)
