cmake_minimum_required (VERSION 3.8)

FILE(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS ./*.cpp ./*.h)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE OFF    # 在 Windows 系统中，运行时不启动控制台窗口，默认OFF
    LINK_WHAT_YOU_USE ON    # 告诉编译器不要自动剔除没有引用符号的链接库，默认OFF
)
find_package(TBB COMPONENTS tbb tbbmalloc tbbmalloc_proxy REQUIRED HINTS ${VCPKG_CMAKE_SHARED_PATH})
find_package(benchmark REQUIRED COMPONENTS benchmark benchmark_main HINTS ${VCPKG_CMAKE_SHARED_PATH})

target_include_directories(${PROJECT_NAME} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${BOOST_INCLUDE_PATH}>
    $<BUILD_INTERFACE:${VCPKG_INCLUDE_PATH}>
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    TBB::tbb
    TBB::tbbmalloc
    TBB::tbbmalloc_proxy
    benchmark::benchmark
    benchmark::benchmark_main
)

# 通用的DLL复制函数 - 接受DLL列表
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

# 设置调试环境PATH和复制DLL
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_DEBUG_BIN_PATH};$ENV{PATH}"
    )
    
    # 定义调试版DLL列表
    set(DEBUG_DLL_LIST
        "tbb12_debug.dll"
    )
    
    # 复制所有调试版DLL
    copy_dlls_to_target(${PROJECT_NAME} "${DEBUG_DLL_LIST}" ${VCPKG_DEBUG_BIN_PATH})
    
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_BIN_PATH};$ENV{PATH}"
    )
    
    # 定义发布版DLL列表
    set(RELEASE_DLL_LIST
        "tbb12.dll"
        "benchmark*.dll"
    )
    
    # 复制所有发布版DLL
    copy_dlls_to_target(${PROJECT_NAME} "${RELEASE_DLL_LIST}" ${VCPKG_BIN_PATH})
endif()

