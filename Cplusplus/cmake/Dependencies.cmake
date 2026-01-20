# Detect compiler type
if(COMPILER_MSVC)
    # ...<version> /bin/Hostx64/x64/cl.exe
    get_filename_component(CL_EXE_PATH "${CMAKE_CXX_COMPILER}" ABSOLUTE)
    get_filename_component(CL_DIR "${CL_EXE_PATH}" DIRECTORY)
    get_filename_component(HOST_DIR "${CL_DIR}" DIRECTORY)
    get_filename_component(BIN_DIR "${HOST_DIR}" DIRECTORY)
    get_filename_component(VC_TOOLS_INSTALL_DIR "${BIN_DIR}" DIRECTORY)

    set(VC_INCLUDE_DIR "${VC_TOOLS_INSTALL_DIR}/include" CACHE PATH "MSVC C++ standard include directory")
    set(VC_MODULES_DIR "${VC_TOOLS_INSTALL_DIR}/modules" CACHE PATH "MSVC C++ modules directory")
    message(STATUS "MSVC C++ include dir: ${VC_INCLUDE_DIR}")
    message(STATUS "MSVC C++ modules dir: ${VC_MODULES_DIR}")

    if(NOT EXISTS "${VC_INCLUDE_DIR}")
        message(WARNING "❌ MSVC C++ include directory NOT found: ${VC_INCLUDE_DIR}")
        message(FATAL_ERROR "MSVC include directory is required, aborting...")
    else()
        include_directories(${VC_INCLUDE_DIR})
        message(STATUS "✔ MSVC C++ include dir: ${VC_INCLUDE_DIR}")
    endif()

    if(NOT EXISTS "${VC_MODULES_DIR}")
        include_directories(${VC_MODULES_DIR})
        message(WARNING "⚠️ MSVC C++ modules directory NOT found: ${VC_MODULES_DIR} (this may be normal for some MSVC versions)")
    else()
        message(STATUS "✔ MSVC C++ modules dir: ${VC_MODULES_DIR}")
    endif()
elseif(COMPILER_GCC)
    # GCC compiler
    message(STATUS "🔧 Using GCC compiler: ${CMAKE_CXX_COMPILER}")
    set(COMPILER_FAMILY "GCC")
    
    # For GCC, we don't need to set standard include directories explicitly
    # CMake automatically handles this
elseif(COMPILER_CLANG)
    # Clang compiler
    message(STATUS "🔧 Using Clang compiler: ${CMAKE_CXX_COMPILER}")
    set(COMPILER_FAMILY "Clang")
    
    # For Clang, we don't need to set standard include directories explicitly
    # CMake automatically handles this
else()
    # Unknown compiler
    message(WARNING "⚠️ Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
    set(COMPILER_FAMILY "Unknown")
endif()

# Set vcpkg path based on operating system and compiler
#if(DEFINED ENV{VCPKG_ROOT})
#    set(VCPKG_ROOT $ENV{VCPKG_ROOT} CACHE PATH "Vcpkg root directory")
#else()
    if(WIN32)
        set(VCPKG_ROOT "E:/Development/vcpkg" CACHE PATH "Vcpkg root directory")
    elseif(UNIX AND NOT APPLE)
        set(VCPKG_ROOT "$ENV{HOME}/vcpkg" CACHE PATH "Vcpkg root directory")
    elseif(APPLE)
        set(VCPKG_ROOT "/opt/vcpkg" CACHE PATH "Vcpkg root directory")
    else()
        set(VCPKG_ROOT "" CACHE PATH "Vcpkg root directory")
        message(WARNING "❌ Unknown operating system, vcpkg path not set")
    endif()
#endif()

# Set vcpkg triplet based on compiler and architecture
if(WIN32)
    if(MSVC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(VCPKG_TRIPLET "x64-windows" CACHE STRING "Vcpkg triplet")
        else()
            set(VCPKG_TRIPLET "x86-windows" CACHE STRING "Vcpkg triplet")
        endif()
    elseif(COMPILER_CLANG OR COMPILER_GCC)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(VCPKG_TRIPLET "x64-mingw-dynamic" CACHE STRING "Vcpkg triplet")
        else()
            set(VCPKG_TRIPLET "x86-mingw-dynamic" CACHE STRING "Vcpkg triplet")
        endif()
    endif()
elseif(UNIX AND NOT APPLE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(VCPKG_TRIPLET "x64-linux" CACHE STRING "Vcpkg triplet")
    else()
        set(VCPKG_TRIPLET "x86-linux" CACHE STRING "Vcpkg triplet")
    endif()
elseif(APPLE)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(VCPKG_TRIPLET "x64-osx" CACHE STRING "Vcpkg triplet")
    else()
        set(VCPKG_TRIPLET "x86-osx" CACHE STRING "Vcpkg triplet")
    endif()
endif()

# Construct vcpkg paths
set(VCPKG_PATH "${VCPKG_ROOT}/installed/${VCPKG_TRIPLET}")
set(VCPKG_BIN_PATH "${VCPKG_PATH}/bin")
set(VCPKG_DEBUG_BIN_PATH "${VCPKG_PATH}/debug/bin")
set(VCPKG_INCLUDE_PATH "${VCPKG_PATH}/include")
set(VCPKG_LIBRARY_PATH "${VCPKG_PATH}/lib")
set(VCPKG_CMAKE_SHARED_PATH "${VCPKG_PATH}/share")

message(STATUS "🔧 Vcpkg root: ${VCPKG_ROOT}")
message(STATUS "🔧 Vcpkg triplet: ${VCPKG_TRIPLET}")
message(STATUS "🔧 Vcpkg path: ${VCPKG_PATH}")

# Set Boost paths based on operating system
if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT $ENV{BOOST_ROOT} CACHE PATH "Boost root directory")
    set(BOOST_INCLUDE_PATH "${BOOST_ROOT}/include" CACHE PATH "Boost include directory")
    
    if(WIN32)
        set(BOOST_LIBRARY_PATH "${BOOST_ROOT}/lib" CACHE PATH "Boost library directory")
    elseif(UNIX)
        set(BOOST_LIBRARY_PATH "${BOOST_ROOT}/lib" CACHE PATH "Boost library directory")
    endif()
else()
    if(WIN32)
        set(BOOST_INCLUDE_PATH "E:/Development/boost_1_87_0/build/include" CACHE PATH "Boost include directory")
        set(BOOST_LIBRARY_PATH "E:/Development/boost_1_87_0/build/lib" CACHE PATH "Boost library directory")
    elseif(UNIX AND NOT APPLE)
        set(BOOST_INCLUDE_PATH "/usr/local/include" CACHE PATH "Boost include directory")
        set(BOOST_LIBRARY_PATH "/usr/local/lib" CACHE PATH "Boost library directory")
    elseif(APPLE)
        set(BOOST_INCLUDE_PATH "/usr/local/include" CACHE PATH "Boost include directory")
        set(BOOST_LIBRARY_PATH "/usr/local/lib" CACHE PATH "Boost library directory")
    endif()
endif()

message(STATUS "🔧 Boost include: ${BOOST_INCLUDE_PATH}")
message(STATUS "🔧 Boost library: ${BOOST_LIBRARY_PATH}")

include_directories(
    ${VCPKG_INCLUDE_PATH}
    ${BOOST_INCLUDE_PATH}
    $ENV{CUDA_TOOLKIT_ROOT_DIR}/include
)

function(setup_target_path target_name)
    if(MSVC)
        # For Visual Studio, set debug environment path
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set_target_properties(${target_name} PROPERTIES
                VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_DEBUG_BIN_PATH};$ENV{PATH}"
            )
        else()
            set_target_properties(${target_name} PROPERTIES
                VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_BIN_PATH};$ENV{PATH}"
            )
        endif()
    elseif(COMPILER_CLANG OR COMPILER_GCC)
        # For GCC/Clang, set RPATH
        if(WIN32)
            # On Windows with MinGW, set PATH similarly to MSVC
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set_target_properties(${target_name} PROPERTIES
                    VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_DEBUG_BIN_PATH};$ENV{PATH}"
                )
            else()
                set_target_properties(${target_name} PROPERTIES
                    VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_BIN_PATH};$ENV{PATH}"
                )
            endif()
        else()
            # On Linux/macOS, set RPATH
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                set_target_properties(${target_name} PROPERTIES
                    BUILD_WITH_INSTALL_RPATH TRUE
                    INSTALL_RPATH "${VCPKG_DEBUG_BIN_PATH}:${VCPKG_DEBUG_BIN_PATH}/../lib"
                )
            else()
                set_target_properties(${target_name} PROPERTIES
                    BUILD_WITH_INSTALL_RPATH TRUE
                    INSTALL_RPATH "${VCPKG_BIN_PATH}:${VCPKG_BIN_PATH}/../lib"
                )
            endif()
        endif()
    endif()
endfunction()