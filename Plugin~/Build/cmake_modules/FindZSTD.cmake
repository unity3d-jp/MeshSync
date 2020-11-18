# * ZSTD_INCLUDE_DIR
# * ZSTD_LIBRARY

find_path(ZSTD_INCLUDE_DIR
    NAMES
        "zstd.h"
    PATHS
        ${CMAKE_SOURCE_DIR}/External/zstd/include
    NO_DEFAULT_PATH                        
)

mark_as_advanced(ZSTD_INCLUDE_DIR)

# Decide the name of the zstd lib based on platform
if(WIN32) 
    set(zstd_lib_filename "libzstd_static.lib")
    set(zstd_external_path_suffix "External/zstd/lib/win64")
elseif(APPLE)
    set(zstd_lib_filename "libzstd.a")
    set(zstd_external_path_suffix "External/zstd/lib/osx")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(zstd_lib_filename "libzstd.a")
    set(zstd_external_path_suffix "External/zstd/lib/linux64")
endif()        


find_file(
    ZSTD_LIBRARY 
    NAMES
        ${zstd_lib_filename}
    PATHS
        ${CMAKE_SOURCE_DIR}
    PATH_SUFFIXES 
        ${zstd_external_path_suffix}
    NO_DEFAULT_PATH        
)

mark_as_advanced(ZSTD_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("ZSTD"
    DEFAULT_MSG
    ZSTD_INCLUDE_DIR
	ZSTD_LIBRARY
)
