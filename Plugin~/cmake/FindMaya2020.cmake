# * MAYA2020_INCLUDE_DIR
# * MAYA2020_LIBRARIES

set(CMAKE_PREFIX_PATH
    "/Applications/Autodesk/maya2020"
    "/Applications/Autodesk/maya2020/Maya.app/Contents"
    "/usr/autodesk/maya2020"
    "/opt/autodesk/maya2020"
)

find_path(MAYA2020_INCLUDE_DIR maya/MGlobal.h)
mark_as_advanced(MAYA2020_INCLUDE_DIR)
foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation)
    find_file(MAYA2020_${MAYA_LIB}_LIBRARY lib${MAYA_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX} PATH_SUFFIXES lib MacOS)
    mark_as_advanced(MAYA2020_${MAYA_LIB}_LIBRARY)
    if(MAYA2020_${MAYA_LIB}_LIBRARY)
        list(APPEND MAYA2020_LIBRARIES ${MAYA2020_${MAYA_LIB}_LIBRARY})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Maya2020"
    DEFAULT_MSG
    MAYA2020_INCLUDE_DIR
    MAYA2020_LIBRARIES
)
