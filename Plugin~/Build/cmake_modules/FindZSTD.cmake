# * ZSTD_INCLUDE_DIR
# * ZSTD_LIBRARY

find_path(ZSTD_INCLUDE_DIR
    NAMES
        "zstd.h"
    PATHS
        "/usr/include"
        "/usr/local/include"
        "$ENV{ZSTD_DIR}/include"
)

mark_as_advanced(ZSTD_INCLUDE_DIR)

find_file(
    ZSTD_LIBRARY 
    NAMES
        libzstd_static.lib  # Windows
        libzstd.a           # Others
    HINTS 
        "$ENV{ZSTD_DIR}"
       
    PATH_SUFFIXES 
        lib
        static
)
mark_as_advanced(ZSTD_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("ZSTD"
    DEFAULT_MSG
    ZSTD_INCLUDE_DIR
	ZSTD_LIBRARY
)
