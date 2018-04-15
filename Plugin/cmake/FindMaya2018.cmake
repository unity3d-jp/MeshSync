# * MAYA2018_INCLUDE_DIR
# * MAYA2018_LIBRARIES

set(CMAKE_PREFIX_PATH
    "/Applications/Autodesk/maya2018/Maya.app/Contents"
    "/usr/autodesk/maya2018"
    "/opt/autodesk/maya2018"
)

find_path(MAYA2018_INCLUDE_DIR NAMES maya/MGlobal.h)
mark_as_advanced(MAYA2018_INCLUDE_DIR)
foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation tbb)
    find_library(MAYA2018_${MAYA_LIB}_LIBRARY NAMES ${MAYA_LIB} PATHS ${LIBRARY_PATHS} PATH_SUFFIXES lib MacOS)
    mark_as_advanced(MAYA2018_${MAYA_LIB}_LIBRARY)
    if(MAYA2018_${MAYA_LIB}_LIBRARY)
        list(APPEND MAYA2018_LIBRARIES ${MAYA2018_${MAYA_LIB}_LIBRARY})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Maya2018"
    DEFAULT_MSG
    MAYA2018_INCLUDE_DIR
    MAYA2018_LIBRARIES
)
