# * MODO_QT_LIBRARIES

set(CMAKE_PREFIX_PATH
    "~/Modo12.2v2"
    "~/Modo13.0v1"
)

mark_as_advanced(MAYA2019_INCLUDE_DIR)
foreach(QT_LIB QtCore QtGui)
    find_file(MODO_${QT_LIB}_LIBRARY lib${QT_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
    mark_as_advanced(MODO_${QT_LIB}_LIBRARY)
    if(MODO_${QT_LIB}_LIBRARY)
        list(APPEND MODO_QT_LIBRARIES ${MODO_${QT_LIB}_LIBRARY})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Modo"
    DEFAULT_MSG
    MODO_QT_LIBRARIES
)
