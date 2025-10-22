cmake_minimum_required (VERSION 3.8)

FILE(GLOB ORIGIN ./*.cpp ./*.h)

set(PROJECT_SOURCES 
    ${ORIGIN}
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

find_package(TBB REQUIRED HINTS ${VCPKG_CMAKE_SHARED_PATH})

target_include_directories(${PROJECT_NAME} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${BOOST_INCLUDE_PATH}>
    $<BUILD_INTERFACE:${VCPKG_INCLUDE_PATH}>
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    TBB::tbb
)

if(WIN32)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    file(GLOB TBB_DLLS 
        "${VCPKG_BIN_PATH}/tbb12.dll"
    )
     
    # 设置调试环境PATH
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_BIN_PATH};$ENV{PATH}"
    )
else()
    file(GLOB TBB_DLLS 
        "${VCPKG_BIN_PATH}/../debug/bin/tbb12_debug.dll"
    )
    # 设置调试环境PATH
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_BIN_PATH}/../debug/bin;$ENV{PATH}"
    )
endif()
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
if (MSVC)
# char / signed char / unsigned char 都是独立类型，可以使用以下方式指定编译器char是signed char还是unsigned char
# -funsigned-char
# -fsigned-char
    target_compile_options(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:/ZI>"
        "$<$<CONFIG:Debug>:/Od>"
        "$<$<CONFIG:Release>:/O2>"
        "$<$<CONFIG:RelWithDebInfo>:/Zi>"
        "$<$<CONFIG:RelWithDebInfo>:/O2>"
        "$<$<CONFIG:MinSizeRel>:/Zi>"
        "$<$<CONFIG:MinSizeRel>:/O1>"
    )
    
    target_link_options(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:/DEBUG>"
        "$<$<CONFIG:RelWithDebInfo>:/DEBUG:FULL>"
        "$<$<CONFIG:Release>:/DEBUG:NONE>"
        "$<$<CONFIG:MinSizeRel>:/DEBUG:FASTLINK>"
        "/STACK:4194304"
    )
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
        DEBUG_POSTFIX "d"
        COMPILE_PDB_NAME ${PROJECT_NAME}
        COMPILE_PDB_NAME_DEBUG ${PROJECT_NAME}d
        COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
else()
    target_link_options(${PROJECT_NAME} PRIVATE
        "-Wl,--stack,4194304"
    )
endif ()
