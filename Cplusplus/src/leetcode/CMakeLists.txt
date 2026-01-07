file(GLOB_RECURSE LEETCODE_SOURCES CONFIGURE_DEPENDS "*.cpp")

if(LEETCODE_SOURCES)
    add_executable(leetcode_exe ${LEETCODE_SOURCES})
    
    set_target_properties(leetcode_exe PROPERTIES
        WIN32_EXECUTABLE OFF
        LINK_WHAT_YOU_USE ON
    )
    if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        setup_target_path(leetcode_exe)
    endif()
    target_include_directories(leetcode_exe PRIVATE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
endif()