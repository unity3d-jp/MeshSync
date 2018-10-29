# * MAYALT2018_INCLUDE_DIR
# * MAYALT2018_LIBRARIES

set(CMAKE_PREFIX_PATH
    "/Applications/Autodesk/mayaLT2018/Maya.app/Contents"
    "/usr/autodesk/maya2018"
    "/opt/autodesk/maya2018"
)

find_path(MAYALT2018_INCLUDE_DIR maya/MGlobal.h)
mark_as_advanced(MAYALT2018_INCLUDE_DIR)
foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation)
    find_file(MAYALT2018_${MAYA_LIB}_LIBRARY lib${MAYA_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX} PATH_SUFFIXES lib MacOS)
    mark_as_advanced(MAYALT2018_${MAYA_LIB}_LIBRARY)
    if(MAYALT2018_${MAYA_LIB}_LIBRARY)
        list(APPEND MAYALT2018_LIBRARIES ${MAYALT2018_${MAYA_LIB}_LIBRARY})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("MayaLT2018"
    DEFAULT_MSG
    MAYALT2018_INCLUDE_DIR
    MAYALT2018_LIBRARIES
)
