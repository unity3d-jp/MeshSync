include(CMakeParseArguments)

option(ENABLE_ISPC "Use Intel ISPC to generate SIMDified code. It can significantly boost performance." ON)
set(ISPC "/usr/local/bin/ispc" CACHE PATH "Path to Intel ISPC")
mark_as_advanced(FORCE ISPC)

function(setup_ispc)
    if(EXISTS ${ISPC})
        return()
    endif()

    set(ISPC_VERSION 1.14.1)

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(ispc_root "${CMAKE_BINARY_DIR}/ispc-v${ISPC_VERSION}-linux") 
        set(ispc_extract_path "${CMAKE_BINARY_DIR}")
        set(ISPC_ARCHIVE_FILE "ispc-v${ISPC_VERSION}-linux.tar.gz")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(ispc_root "${CMAKE_BINARY_DIR}/ispc-v${ISPC_VERSION}-macOS") 
        set(ispc_extract_path "${CMAKE_BINARY_DIR}")
        set(ISPC_ARCHIVE_FILE "ispc-v${ISPC_VERSION}-macOS.tar.gz")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(ispc_root "${CMAKE_BINARY_DIR}/ispc-v${ISPC_VERSION}-windows") 
        set(ispc_extract_path "${ispc_root}")
        set(ISPC_ARCHIVE_FILE "ispc-v${ISPC_VERSION}-windows.zip")
    endif()
    set(ISPC "${ispc_root}/bin/ispc" CACHE PATH "" FORCE)   

    set(ISPC_ARCHIVE_URL "https://github.com/ispc/ispc/releases/download/v${ISPC_VERSION}/${ISPC_ARCHIVE_FILE}")
    set(ISPC_ARCHIVE_PATH "${CMAKE_BINARY_DIR}/${ISPC_ARCHIVE_FILE}")
    if(NOT EXISTS ${ISPC_ARCHIVE_PATH})
        file(DOWNLOAD ${ISPC_ARCHIVE_URL} ${ISPC_ARCHIVE_PATH} SHOW_PROGRESS)
    endif()
    
    message("   Extracting ${ISPC_ARCHIVE_FILE}")
    file(MAKE_DIRECTORY ${ispc_extract_path})        
    execute_process(
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND tar -xf ${ISPC_ARCHIVE_PATH} -C ${ispc_extract_path}
    )
endfunction()


# e.g:
#add_ispc_targets(
#    SOURCES "src1.ispc" "src2.ispc"
#    HEADERS "header1.h" "header2.h"
#    OUTDIR "path/to/outputs")
function(add_ispc_targets)
    cmake_parse_arguments(arg "" "OUTDIR" "SOURCES;HEADERS" ${ARGN})

    foreach(source ${arg_SOURCES})
        get_filename_component(name ${source} NAME_WE)
        set(header "${arg_OUTDIR}/${name}.h")
        set(object "${arg_OUTDIR}/${name}${CMAKE_CXX_OUTPUT_EXTENSION}")
        if(ANDROID)
            # target: arm64
            set(target --target=neon-i32x4 --arch=aarch64)
            set(objects 
                ${object}
                "${arg_OUTDIR}/${name}_neon${CMAKE_CXX_OUTPUT_EXTENSION}"
            )
        else()
            # target: x86-64
            set(target --target=sse4-i32x4,avx1-i32x8,avx512skx-i32x16 --arch=x86-64)
            set(objects 
                ${object}
                "${arg_OUTDIR}/${name}_sse4${CMAKE_CXX_OUTPUT_EXTENSION}"
                "${arg_OUTDIR}/${name}_avx${CMAKE_CXX_OUTPUT_EXTENSION}"
                "${arg_OUTDIR}/${name}_avx512skx${CMAKE_CXX_OUTPUT_EXTENSION}"
            )
        endif()
        set(outputs ${header} ${objects})
        add_custom_command(
            OUTPUT ${outputs}
            COMMAND ${ISPC} ${source} -o ${object} -h ${header} --pic ${target} --opt=fast-masked-vload --opt=fast-math --wno-perf
            DEPENDS ${source} ${arg_HEADERS}
        )

        list(APPEND _ispc_headers ${header})
        list(APPEND _ispc_objects ${objects})
        list(APPEND _ispc_outputs ${outputs})
    endforeach()

    set(_ispc_headers ${_ispc_headers} PARENT_SCOPE)
    set(_ispc_objects ${_ispc_objects} PARENT_SCOPE)
    set(_ispc_outputs ${_ispc_outputs} PARENT_SCOPE)

    execute_process(COMMAND mkdir -p ${arg_OUTDIR})
    foreach(f ${_ispc_outputs})
        if(NOT EXISTS ${f})
            execute_process(COMMAND touch -t 200001010000 ${f})
        endif()
    endforeach()
endfunction()
