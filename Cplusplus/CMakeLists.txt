cmake_minimum_required(VERSION 3.20)
project(Cplusplus-learn
    VERSION 1.0.0
    DESCRIPTION "Cplusplus-learn"
    HOMEPAGE_URL ""
    LANGUAGES C CXX CUDA
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES 61) # GTX1080Ti
set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -allow-unsupported-compiler")
set(CMAKE_CUDA_ARCHITECTURES native)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

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
#add_subdirectory(src/cuda)
#add_subdirectory(src/leetcode)
#add_subdirectory(src/opengl)
#add_subdirectory(src/parallel)
#add_subdirectory(src/radix_sort)