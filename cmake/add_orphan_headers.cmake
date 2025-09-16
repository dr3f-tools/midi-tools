# Add a cpp for each orphaned header in the given directory.
function(add_orphan_headers project_name)
    # get include directory
    get_target_property(target_include_dirs ${project_name} INTERFACE_INCLUDE_DIRECTORIES)

    # get build directory
    get_target_property(target_build_dir ${project_name} BINARY_DIR)

    set(source_list)
    foreach(header IN ITEMS ${ARGN})
        set(filename ${target_build_dir}/autogen/${header}.cpp)
        
        # Extract filename after "include/"
        string(FIND ${header} "include/" include_pos)
        if(include_pos GREATER -1)
            math(EXPR start_pos "${include_pos} + 8") # 8 is length of "include/"
            string(SUBSTRING ${header} ${start_pos} -1 header_path)
        else()
            message(FATAL_ERROR "Header ${header} does not contain 'include/' in its path.")
        endif()

        file(WRITE ${filename} "#include \"${header_path}\"\n")
        list(APPEND source_list ${filename})
        message(STATUS "Generating ${filename}")
    endforeach()
    add_library(${project_name}_orphan_headers OBJECT ${source_list})
    target_include_directories(${project_name}_orphan_headers PRIVATE ${target_include_dirs})

endfunction()