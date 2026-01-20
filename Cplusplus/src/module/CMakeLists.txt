file(GLOB_RECURSE MODULE_INTERFACES CONFIGURE_DEPENDS "*.ixx")
file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS "*.cpp")

if(MODULE_INTERFACES OR MODULE_SOURCES)
    add_library(simple_module)
    target_sources(simple_module PUBLIC
        FILE_SET simple_module
        TYPE CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${VC_MODULES_DIR}
        FILES ${MODULE_INTERFACES} ${VC_MODULES_DIR}/std.ixx ${VC_MODULES_DIR}/std.compat.ixx
    )
    set_target_properties(simple_module PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb
        COMPILE_PDB_NAME "simple_module"
    )

    add_executable(module_exe ${MODULE_SOURCES})
    set_target_properties(module_exe PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        WIN32_EXECUTABLE OFF
        LINK_WHAT_YOU_USE ON
        COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb
        PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb
        COMPILE_PDB_NAME "module_exe"
        PDB_NAME "module_exe"
    )

    if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        setup_target_path(module_exe)
    endif()

    target_include_directories(module_exe PRIVATE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
    target_link_libraries(module_exe PRIVATE simple_module)
endif()