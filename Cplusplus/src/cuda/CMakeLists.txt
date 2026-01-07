file(GLOB_RECURSE CUDA_SOURCES CONFIGURE_DEPENDS "*.cu")

if(CUDA_SOURCES)
    add_executable(cuda_exe ${CUDA_SOURCES})
    
    set_target_properties(cuda_exe PROPERTIES
        CXX_STANDARD 20
        WIN32_EXECUTABLE OFF
        LINK_WHAT_YOU_USE ON
        CUDA_SEPARABLE_COMPILATION ON #启用分离编译，对于复杂CUDA项目为必须
    )
    if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        setup_target_path(cuda_exe)
    endif()

    target_compile_options(cuda_exe PRIVATE 
        $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>)

    target_include_directories(cuda_exe PRIVATE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )

endif()