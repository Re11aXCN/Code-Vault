cmake_minimum_required (VERSION 3.8)

FILE(GLOB ORIGIN ./*.cpp ./*.h)

set(PROJECT_SOURCES 
    ${ORIGIN}
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${BOOST_INCLUDE_PATH}>
    $<BUILD_INTERFACE:${VCPKG_INCLUDE_PATH}>
)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${BOOST_LIBRARY_PATH}
    ${VCPKG_LIBRARY_PATH}
)

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
