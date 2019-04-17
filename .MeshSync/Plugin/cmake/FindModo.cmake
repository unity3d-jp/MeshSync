# * MODO_SDK_DIR
# * MODO_QT_INCLUDE_DIR
# * MODO_QT_LIBRARIES

set(CMAKE_PREFIX_PATH
    "~/Modo13.0v1"
    "~/Modo12.2v2"
    "/Applications/Modo13.0v1.app/Contents/Frameworks/QtCore.framework"
    "/Applications/Modo13.0v1.app/Contents/Frameworks/QtGui.framework"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(MODO_QT_LIBRARY_PREFIX "")
    set(MODO_QT_LIBRARY_SUFFIX "")
    set(MODO_QT_INCLUDE_DIR "/Applications/Modo13.0v1.app/Contents/Frameworks/QtGui.framework/Headers" CACHE PATH "Qt include dir")
else()
    set(MODO_QT_LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
    set(MODO_QT_LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(MODO_QT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/External/Qt/4.8.5/include/QtGui" CACHE PATH "Qt include dir")
endif()
mark_as_advanced(MODO_QT_INCLUDE_DIR)

set(MODO_SDK_DIR "${CMAKE_SOURCE_DIR}/External/LXSDK_525410" CACHE PATH "Modo SDK directory")
mark_as_advanced(FORCE MODO_SDK_DIR)

foreach(QT_LIB QtCore QtGui)
    find_file(MODO_${QT_LIB}_LIBRARY ${MODO_QT_LIBRARY_PREFIX}${QT_LIB}${MODO_QT_LIBRARY_SUFFIX})
    mark_as_advanced(MODO_${QT_LIB}_LIBRARY)
    if(MODO_${QT_LIB}_LIBRARY)
        list(APPEND MODO_QT_LIBRARIES ${MODO_${QT_LIB}_LIBRARY})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("Modo"
    DEFAULT_MSG
    MODO_QT_LIBRARIES
    MODO_QT_INCLUDE_DIR
)
