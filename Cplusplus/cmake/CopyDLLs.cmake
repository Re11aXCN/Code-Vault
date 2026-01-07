# DLL copy function
function(copy_dlls_to_target target_name dll_list search_path)
    foreach(DLL_PATTERN ${dll_list})
        FILE(GLOB DLL_FILES "${search_path}/${DLL_PATTERN}")
        foreach(DLL ${DLL_FILES})
            if(EXISTS ${DLL})
                add_custom_command(TARGET ${target_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${DLL}"
                        $<TARGET_FILE_DIR:${target_name}>
                    COMMENT "Copying ${DLL} to output directory"
                )
            endif()
        endforeach()
    endforeach()
endfunction()

