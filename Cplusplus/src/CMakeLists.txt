cmake_minimum_required (VERSION 3.8)

FILE(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS ./*.cpp ./*.h)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE OFF    # 在 Windows 系统中，运行时不启动控制台窗口，默认OFF
    LINK_WHAT_YOU_USE ON    # 告诉编译器不要自动剔除没有引用符号的链接库，默认OFF
)
find_package(TBB COMPONENTS tbb tbbmalloc tbbmalloc_proxy REQUIRED HINTS ${VCPKG_CMAKE_SHARED_PATH})

target_include_directories(${PROJECT_NAME} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${BOOST_INCLUDE_PATH}>
    $<BUILD_INTERFACE:${VCPKG_INCLUDE_PATH}>
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    TBB::tbb
    TBB::tbbmalloc
    TBB::tbbmalloc_proxy
)

# 设置调试环境PATH
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    FILE(GLOB TBB_DLLS ${VCPKG_DEBUG_BIN_PATH}/tbb12_debug.dll)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_DEBUG_BIN_PATH};$ENV{PATH}"
    )
    foreach(DLL ${TBB_DLLS})
    if(EXISTS ${DLL})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying ${DLL} to output directory"
        )
    endif()
endforeach()
else()
    FILE(GLOB TBB_DLLS ${VCPKG_BIN_PATH}/tbb12.dll)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_BIN_PATH};$ENV{PATH}"
    )
    foreach(DLL ${TBB_DLLS})
    if(EXISTS ${DLL})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMENT "Copying ${DLL} to output directory"
        )
    endif()
endforeach()
endif()

