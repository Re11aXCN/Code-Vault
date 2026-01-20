# -----------------------------------------------------------------------------
# Compiler-Specific Configuration(General)
# -----------------------------------------------------------------------------
if(COMPILER_MSVC)
    # MSVC Runtime Library Configuration
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # Debug Information Format
    if(POLICY CMP0141)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug>:EditAndContinue>,$<$<CONFIG:RelWithDebInfo>:ProgramDatabase>>")
    endif()

    # General Compilation Options
    set(MSVC_COMPILE_GENERAL_OPTIONS
        # /ifcOutput ${CMAKE_BINARY_DIR}/  # Generate interface definition file
        /utf-8                # character set
        # C strict standard consistency, disable lenient behavior, 
        # to support C20 module import it must be enabled
        /permissive-
        
        # C++20 Modules Support
        /experimental:module  # Enable module support (MSVC 2019 16.8+)
        # /std:c++latest        # Use latest C++ standard for modules

        /Zc:__cplusplus       # Correctly set the __cplusplus macro
        /Zc:wchar_t           # wchar_t as a primitive type
        /Zc:forScope          # for loop range consistency
        /Zc:inline            # Remove unused functions
        /Zc:preprocessor

        /W4                   # Warning Level 4
        /WX-                  # Treat warnings as errors, "WX" will not compile, due to the way the standard library's internal code is written
        /MP                   # Multiprocessor Compilation
        /Zp8                  # 8-byte alignment
        /GF                   # Enable string pool
        /Gd                   # __cdecl calling convention

        # Floating point precision is default; in debug mode it is precise, 
        # in release mode it is fast, or you can set "/fp:fast" or "/fp:strict" to keep it consistent.
        /fp:precise           
        /FC                   # Full Path Diagnosis

        # BUG
        # If the settings, we cannot use coroutines and what Microsoft officially 
        # describes may not be consistent.
        # /await:strict         # Enable cooperative routine support, can use "/await", VS 2026 should replace it as "/await:strict"
        
        /openmp:experimental  # OpenMP support
        /sdl                  # SDL checks
        /EHsc                 # C Exception Handling
        /errorReport:prompt   # Error Reporting Mode
        /diagnostics:column   # Diagnostic Information Format
        /nologo
        /Gm-                  # Disable minimal rebuild
        # https://learn.microsoft.com/zh-cn/cpp/build/reference/arch-x64?view=msvc-170
        /arch:AVX2            # Enable AVX2 instruction set
    )
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:${MSVC_COMPILE_GENERAL_OPTIONS}>")
    add_compile_definitions(
        -D_UNICODE
        -DUNICODE
        -D_WIN32_WINNT=0x0A00
        -DNOMINMAX
        -D_CRT_SECURE_NO_WARNINGS
        -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    )
    add_link_options(
        "/STACK:4194304"  # Set the stack size to 4MB
    )
else()
    # GCC/Clang Configuration
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        message(STATUS "Found ccache program: ${CCACHE_PROGRAM}")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PROGRAM})
    endif()

    # g++:
    # g++ -fmodules-ts -x c++-system-header iostream
    # g++ -fmodules-ts fig16_01.cpp -o fig16_01

    
    # msvc module file extension is ixx, clang is cppm, 
    # clang needs to use a command to recognize the ixx suffix, like [ -x c++-module welcome.ixx ]
    # g++ -fmodules-ts -c -x c++ deitel.math.ixx [ -x c++ ]

    # clang++ (-16 may need to be removed or changed based on your clang++ version):
    # clang++-16 -std=c++20 -x c++-system-header --precompile string -o string.pcm
    # clang++-16 -std=c++20 -x c++-system-header --precompile iostream -o iostream.pcm
    # clang++-16 -std=c++20 -fmodule-file=string.pcm -x c++-module welcome.ixx --precompile -o welcome.pcm
    # clang++-16 -std=c++20 -fmodule-file=iostream.pcm fig16_03.cpp -fprebuilt-module-path=. string-pcm welcome.pcm -o fig16_03

    # GCC/Clang Modules Support
    if(COMPILER_GCC)
        add_compile_options(-fmodules-ts -fmodule-mapper=inline)
    elseif(COMPILER_CLANG)
        add_compile_options(-fmodules -fmodules-ts -fbuiltin-module-map -fimplicit-module-maps -fprebuilt-module-path=${CMAKE_CURRENT_BINARY_DIR})
    endif()
    
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unused-parameter
        -Wno-unused-variable
    )
     add_link_options("-Wl,--stack,4194304")
endif()

# -----------------------------------------------------------------------------
# Build Type Configuration
# -----------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Build Type: Debug")
    add_definitions(-DDEBUG -D_DEBUG)
    
    if(COMPILER_MSVC)
        # MSVC Debug compilation options
        add_compile_options(
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/JMC>"    # Just My Code debugging
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/ZI>"     # Edit and Continue
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Od>"     # Disable optimization
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/RTC1>"   # Runtime error checks
        )
        
        # MSVC Debug link options
        add_link_options(
            "$<$<CONFIG:Debug>:/DEBUG>"
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/DEBUG>"
        )
    else()
        # GCC/Clang Debug compilation options
        add_compile_options(-g -O0 -fno-omit-frame-pointer)
    endif()
    
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Build Type: Release")
    add_definitions(-DNDEBUG)
    
    if(COMPILER_MSVC)
        # MSVC Release compilation options
        add_compile_options(
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/O2>"    # Maximize speed
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Ob2>"   # Inline any suitable function
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/GL>"    # Whole program optimization
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/GS>"    # Buffer security check
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Qpar>"  # Auto-parallelization
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Gy>"    # Function-level linking
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Oi>"    # Intrinsic functions
        )
        
        # MSVC Release link options
        add_link_options(
            "$<$<CONFIG:Release>:/DEBUG:NONE>"
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/LTCG>"  # Link-time code generation
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/DEBUG:NONE>"
        )
    else()
        # GCC/Clang Release compilation options
        add_compile_options(-O3)
    endif()
    
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "Build Type: RelWithDebInfo")
    
    if(COMPILER_MSVC)
        # MSVC RelWithDebInfo compilation options
        add_compile_options(
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/O2>" 
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Ob1>"
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Zi>" 
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/GL>" 
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/GS>" 
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Qpar>"
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Gy>" 
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Oi>" 
        )
        
        # MSVC RelWithDebInfo link options
        add_link_options(
            "$<$<CONFIG:RelWithDebInfo>:/DEBUG:FULL>"
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/LTCG>"
            "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/DEBUG:FULL>"
        )
    else()
        # GCC/Clang RelWithDebInfo compilation options
        add_compile_options(-O2 -g)
    endif()
    
else()
    message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
endif()

# -----------------------------------------------------------------------------
# Sanitizers Configuration
# -----------------------------------------------------------------------------
if(USE_SANITIZERS)
    if(COMPILER_CLANG OR COMPILER_GCC)
        add_compile_options(
            -fsanitize=address
            -fsanitize=undefined
            -fno-omit-frame-pointer
        )
        add_link_options(
            -fsanitize=address
            -fsanitize=undefined
        )
        message(STATUS "Sanitizers enabled")
    else()
        message(WARNING "Sanitizers only supported for Clang/GCC")
    endif()
endif()
