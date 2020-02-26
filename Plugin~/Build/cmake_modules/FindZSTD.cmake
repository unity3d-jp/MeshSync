# * ZSTD_INCLUDE_DIR
# * ZSTD_LIBRARY

find_path(ZSTD_INCLUDE_DIR
    NAMES
        "zstd.h"
    PATHS
        "/usr/include"
        "/usr/local/include"
        ${CMAKE_SOURCE_DIR}/External/zstd/include
)

mark_as_advanced(ZSTD_INCLUDE_DIR)

# Decide the name of the zstd lib based on platform
if(WIN32) 
    set(zstd_lib_filename "libzstd_static.lib")
else()
    set(zstd_lib_filename "libzstd.a")
endif()        


find_file(
    ZSTD_LIBRARY 
    NAMES
        ${zstd_lib_filename}
    PATHS
        ${CMAKE_SOURCE_DIR}
    PATH_SUFFIXES 
        External/zstd/lib/win64
        lib
)

mark_as_advanced(ZSTD_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("ZSTD"
    DEFAULT_MSG
    ZSTD_INCLUDE_DIR
	ZSTD_LIBRARY
)
