cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Cplusplus-learn")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus") #为了使得MSVC __cplusplus宏正确返回C++版本号

if(WIN32 AND MSVC)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:utf-8>")
    # 设置最低支持Windows 7 (0x0601)
    add_compile_definitions(_WIN32_WINNT=0x0601 -DNOMINMAX) #全局宏定义，会传播给子目录，target不会
endif()

set(VCPKG_BIN_PATH "E:/Development/vcpkg-2025.06.13/installed/x64-windows/bin")
set(BOOST_INCLUDE_PATH "E:/Development/boost_1_87_0/build/include")
set(VCPKG_INCLUDE_PATH "E:/Development/vcpkg-2025.06.13/installed/x64-windows/include")

set(BOOST_LIBRARY_PATH "E:/Development/boost_1_87_0/build/lib")
set(VCPKG_LIBRARY_PATH "E:/Development/vcpkg-2025.06.13/installed/x64-windows/lib/")
set(VCPKG_CMAKE_SHARED_PATH "E:/Development/vcpkg-2025.06.13/installed/x64-windows/share")
add_subdirectory(src)