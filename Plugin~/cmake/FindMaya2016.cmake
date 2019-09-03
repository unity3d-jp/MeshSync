# * MAYA2016_INCLUDE_DIR
# * MAYA2016_LIBRARIES

set(CMAKE_PREFIX_PATH
    "/Applications/Autodesk/maya2016/Maya.app/Contents"
    "/usr/autodesk/maya2016"
    "/opt/autodesk/maya2016"
)

find_path(MAYA2016_INCLUDE_DIR NAMES maya/MGlobal.h)
mark_as_advanced(MAYA2016_INCLUDE_DIR)
foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation)
    find_file(MAYA2016_${MAYA_LIB}_LIBRARY lib${MAYA_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX} PATH_SUFFIXES lib MacOS)
    mark_as_advanced(MAYA2016_${MAYA_LIB}_LIBRARY)
    if(MAYA2016_${MAYA_LIB}_LIBRARY)
        list(APPEND MAYA2016_LIBRARIES ${MAYA2016_${MAYA_LIB}_LIBRARY})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Maya2016"
    DEFAULT_MSG
    MAYA2016_INCLUDE_DIR
    MAYA2016_LIBRARIES
)
