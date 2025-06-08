# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("cpluspluslearn")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus") #为了使得MSVC __cplusplus宏正确返回C++版本号
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:utf-8>")


# 设置 vcpkg 工具链（推荐在命令行设置）
# 在 CMake 配置时添加: -DCMAKE_TOOLCHAIN_FILE=D:/Development/vcpkg/scripts/buildsystems/vcpkg.cmake
# 设置 vcpkg 目标目录
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg target triplet")
set(VCPKG_INSTALLED_DIR "D:/Development/vcpkg/installed/${VCPKG_TARGET_TRIPLET}")

# 添加 vcpkg 包含目录
include_directories(${VCPKG_INSTALLED_DIR}/include)

# 添加 vcpkg 库目录
link_directories(${VCPKG_INSTALLED_DIR}/lib)

# BOOST
set(BOOST_ROOT "E:/Development/boost_1_87_0/build")
find_package(Boost 1.87.0 REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

if(WIN32)
    # 设置最低支持Windows 7 (0x0601)
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

add_executable(cplusplus_exe
 "src/web/async/AsyncTest.cxx")
 target_link_libraries(cplusplus_exe PRIVATE Boost::system)