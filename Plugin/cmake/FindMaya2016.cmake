# * MAYA2016_INCLUDE_DIR
# * MAYA2016_LIBRARIES

set(CMAKE_PREFIX_PATH
    "/usr/autodesk/maya2016"
    "/opt/autodesk/maya2016"
)

find_path(MAYA2016_INCLUDE_DIR NAMES maya/MGlobal.h)
mark_as_advanced(MAYA2016_INCLUDE_DIR)
foreach(MAYA_LIB OpenMayaAnim OpenMayaFX OpenMayaRender OpenMayaUI OpenMaya Foundation tbb)
    find_library(MAYA2016_${MAYA_LIB}_LIBRARY NAMES ${MAYA_LIB} PATHS ${LIBRARY_PATHS} PATH_SUFFIXES lib)
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
