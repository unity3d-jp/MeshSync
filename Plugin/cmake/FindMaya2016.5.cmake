# * MAYA2016.5_INCLUDE_DIR
# * MAYA2016.5_LIBRARIES

set(CMAKE_PREFIX_PATH
    "/Applications/Autodesk/maya2016.5/Maya.app/Contents"
    "/usr/autodesk/maya2016.5"
    "/opt/autodesk/maya2016.5"
)

find_path(MAYA2016.5_INCLUDE_DIR NAMES maya/MGlobal.h)
mark_as_advanced(MAYA2016.5_INCLUDE_DIR)
foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation)
    find_file(MAYA2016.5_${MAYA_LIB}_LIBRARY lib${MAYA_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX} PATH_SUFFIXES lib MacOS)
    mark_as_advanced(MAYA2016.5_${MAYA_LIB}_LIBRARY)
    if(MAYA2016.5_${MAYA_LIB}_LIBRARY)
        list(APPEND MAYA2016.5_LIBRARIES ${MAYA2016.5_${MAYA_LIB}_LIBRARY})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Maya2016.5"
    DEFAULT_MSG
    MAYA2016.5_INCLUDE_DIR
    MAYA2016.5_LIBRARIES
)
