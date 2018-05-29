# * MAYA2015_INCLUDE_DIR
# * MAYA2015_LIBRARIES

set(CMAKE_PREFIX_PATH
    "/Applications/Autodesk/maya2015/Maya.app/Contents"
    "/Applications/Autodesk/maya2015/devkit"
    "/usr/autodesk/maya2015-x64"
)

find_path(MAYA2015_INCLUDE_DIR NAMES maya/MGlobal.h)
mark_as_advanced(MAYA2015_INCLUDE_DIR)
foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation)
    find_file(MAYA2015_${MAYA_LIB}_LIBRARY lib${MAYA_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX} PATH_SUFFIXES lib MacOS)
    mark_as_advanced(MAYA2015_${MAYA_LIB}_LIBRARY)
    if(MAYA2015_${MAYA_LIB}_LIBRARY)
        list(APPEND MAYA2015_LIBRARIES ${MAYA2015_${MAYA_LIB}_LIBRARY})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Maya2015"
    DEFAULT_MSG
    MAYA2015_INCLUDE_DIR
    MAYA2015_LIBRARIES
)
