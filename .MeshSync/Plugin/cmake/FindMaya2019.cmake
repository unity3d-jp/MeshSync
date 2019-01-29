# * MAYA2019_INCLUDE_DIR
# * MAYA2019_LIBRARIES

set(CMAKE_PREFIX_PATH
    "/Applications/Autodesk/maya2019"
    "/Applications/Autodesk/maya2019/Maya.app/Contents"
    "/usr/autodesk/maya2019"
    "/opt/autodesk/maya2019"
)

find_path(MAYA2019_INCLUDE_DIR maya/MGlobal.h)
mark_as_advanced(MAYA2019_INCLUDE_DIR)
foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation)
    find_file(MAYA2019_${MAYA_LIB}_LIBRARY lib${MAYA_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX} PATH_SUFFIXES lib MacOS)
    mark_as_advanced(MAYA2019_${MAYA_LIB}_LIBRARY)
    if(MAYA2019_${MAYA_LIB}_LIBRARY)
        list(APPEND MAYA2019_LIBRARIES ${MAYA2019_${MAYA_LIB}_LIBRARY})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Maya2019"
    DEFAULT_MSG
    MAYA2019_INCLUDE_DIR
    MAYA2019_LIBRARIES
)
