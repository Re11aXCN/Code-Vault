cmake_minimum_required(VERSION 3.28)
# CMAKE VERSION-4.2.0 UUID
# https://github.com/Kitware/CMake/blob/v4.2.0/Help/dev/experimental.rst#id7
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

project(Cplusplus-learn
    VERSION 1.0.0
    DESCRIPTION "Cplusplus-learn"
    HOMEPAGE_URL ""
    LANGUAGES C CXX CUDA
)

# Detect compiler type
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "Compiler: MSVC")
    set(COMPILER_MSVC TRUE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Compiler: Clang")
    set(COMPILER_CLANG TRUE)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Compiler: GCC")
    set(COMPILER_GCC TRUE)
else()
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES 61) # GTX1080Ti
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -allow-unsupported-compiler")
set(CMAKE_CUDA_ARCHITECTURES native)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH "Runtime output directory")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib CACHE PATH "Library output directory")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib CACHE PATH "Archive output directory")
set(CMAKE_PCH_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/pch CACHE PATH "Precompiled header output directory")
set(CMAKE_ASM_LISTING_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/asm CACHE PATH "Assembly listing output directory")
set(CMAKE_CXX_MODULE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/module CACHE PATH "C++ module output directory")
if (COMPILER_MSVC)
  set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/pdb CACHE PATH "PDB compile output directory")
  set(CMAKE_PDB_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/pdb CACHE PATH "PDB output directory")
endif()

if(WIN32)
    add_definitions(-D_WIN32)
elseif(APPLE)
    add_definitions(-D__APPLE__)
elseif(UNIX)
    add_definitions(-D__linux__)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(cmake/Dependencies.cmake)
include(cmake/CompilerConfig.cmake)
include(cmake/CopyDLLs.cmake)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/stdex)

# 添加所有子目录
add_subdirectory(src)
#add_subdirectory(src/module)
#add_subdirectory(src/cuda)
#add_subdirectory(src/leetcode)
#add_subdirectory(src/opengl)
#add_subdirectory(src/parallel)
#add_subdirectory(src/radix_sort)