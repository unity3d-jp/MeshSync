if(APPLE)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_SKIP_RPATH ON)

elseif(LINUX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
endif()

# ----------------------------------------------------------------------------------------------------------------------

function(add_plugin name)
    cmake_parse_arguments(arg "" "PLUGINS_DIR" "SOURCES" ${ARGN})
    file(TO_NATIVE_PATH ${arg_PLUGINS_DIR} native_plugins_dir)
    

    if(APPLE)
        add_library(${name} MODULE ${arg_SOURCES})
        set_target_properties(${name} PROPERTIES BUNDLE ON)
        SET(target_filename \${TARGET_BUILD_DIR}/${name}.bundle)
        add_custom_command(TARGET ${name} POST_BUILD
            COMMAND rm -rf ${arg_PLUGINS_DIR}/${target_filename}
            COMMAND cp -r ${target_filename} ${native_plugins_dir}               
            COMMENT "Copying ${name} to ${native_plugins_dir}"
        )
    else()
        # Linux/Windows
        add_library(${name} SHARED ${arg_SOURCES})                
        set_target_properties(${name} PROPERTIES PREFIX "" ) 

        SET(target_filename $<TARGET_FILE:${name}>)
        add_custom_command(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
               $<TARGET_FILE:${name}>
               ${native_plugins_dir}              
            COMMENT "Copying ${name} to ${native_plugins_dir}"
        )        
    
    endif()

endfunction()



