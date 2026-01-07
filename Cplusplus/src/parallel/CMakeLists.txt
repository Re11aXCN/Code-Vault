file(GLOB_RECURSE PARALLEL_SOURCES CONFIGURE_DEPENDS "*.cpp")

if(PARALLEL_SOURCES)
    add_executable(parallel_exe ${PARALLEL_SOURCES})
    
    set_target_properties(parallel_exe PROPERTIES
        WIN32_EXECUTABLE OFF
        LINK_WHAT_YOU_USE ON
    )
    if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        setup_target_path(parallel_exe)
    endif()
    
    target_include_directories(parallel_exe PRIVATE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
    
    find_package(TBB COMPONENTS tbb tbbmalloc tbbmalloc_proxy REQUIRED HINTS ${VCPKG_CMAKE_SHARED_PATH})
    target_link_libraries(parallel_exe PRIVATE
        TBB::tbb
        TBB::tbbmalloc
        TBB::tbbmalloc_proxy
    )
    
    # Parallel项目需要的DLL列表
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(PARALLEL_DLL_LIST "tbb*.dll")
        copy_dlls_to_target(parallel_exe "${PARALLEL_DLL_LIST}" ${VCPKG_DEBUG_BIN_PATH})
    else()
        set(PARALLEL_DLL_LIST "tbb*.dll")
        copy_dlls_to_target(parallel_exe "${PARALLEL_DLL_LIST}" ${VCPKG_BIN_PATH})
    endif()
endif()