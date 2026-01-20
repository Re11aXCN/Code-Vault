file(GLOB_RECURSE MUSIC_SOURCES CONFIGURE_DEPENDS "*.cpp")

if(MUSIC_SOURCES)
    add_executable(music_exe ${MUSIC_SOURCES})
    
    set_target_properties(music_exe PROPERTIES
        WIN32_EXECUTABLE OFF
        LINK_WHAT_YOU_USE ON
    )
    if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        setup_target_path(music_exe)
    endif()
    target_include_directories(music_exe PRIVATE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
endif()