# * ZSTD_INCLUDE_DIR
# * ZSTD_LIBRARY

find_path(ZSTD_INCLUDE_DIR
    NAMES
        "zstd.h"
    PATHS
        "/usr/include"
        "/usr/local/include"
)
mark_as_advanced(ZSTD_INCLUDE_DIR)

find_file(ZSTD_LIBRARY libzstd.a PATH_SUFFIXES lib)
mark_as_advanced(ZSTD_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("ZSTD"
    DEFAULT_MSG
    ZSTD_INCLUDE_DIR
	ZSTD_LIBRARY
)
