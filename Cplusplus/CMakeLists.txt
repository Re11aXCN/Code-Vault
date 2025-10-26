cmake_minimum_required(VERSION 3.20)
project ("Cplusplus-learn")

# 设置C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # 可执行文件输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # 动态链接库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # 静态链接库输出目录

# 第三方库路径
set(VCPKG_BIN_PATH "E:/Development/vcpkg-2025.06.13/installed/x64-windows/bin")
set(VCPKG_DEBUG_BIN_PATH ${VCPKG_BIN_PATH}/../debug/bin)
set(BOOST_INCLUDE_PATH "E:/Development/boost_1_87_0/build/include")
set(VCPKG_INCLUDE_PATH "E:/Development/vcpkg-2025.06.13/installed/x64-windows/include")
set(BOOST_LIBRARY_PATH "E:/Development/boost_1_87_0/build/lib")
set(VCPKG_LIBRARY_PATH "E:/Development/vcpkg-2025.06.13/installed/x64-windows/lib/")
set(VCPKG_CMAKE_SHARED_PATH "E:/Development/vcpkg-2025.06.13/installed/x64-windows/share")

# 包含目录
include_directories(
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/um"
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/shared"
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/winrt"
    "C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/cppwinrt"
    "C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8/Include/um"
    ${VCPKG_INCLUDE_PATH}
    ${BOOST_INCLUDE_PATH}
    .
)

if(WIN32 AND MSVC)

    #为了使得MSVC __cplusplus宏正确返回C++版本号
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus") 

    # 通用MSVC 宏设置
    add_compile_definitions(_UNICODE UNICODE _WIN32_WINNT=0x0601 NOMINMAX _USE_MATH_DEFINES)

    # 字符集设置
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")

    # C++严格标准一致性，禁用宽松行为，若要支持 C++20 模块import 必须开启
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/permissive->")

     # 通用编译选项
    add_compile_options(
        "$<$<CXX_COMPILER_ID:MSVC>:/MP>"           # 多处理器编译
        "$<$<CXX_COMPILER_ID:MSVC>:/Zc:wchar_t>"   # wchar_t作为原生类型
        "$<$<CXX_COMPILER_ID:MSVC>:/W3>"           # 警告级别3
        "$<$<CXX_COMPILER_ID:MSVC>:/Zp8>"          # 8字节对齐
        "$<$<CXX_COMPILER_ID:MSVC>:/GF>"           # 字符串池
        "$<$<CXX_COMPILER_ID:MSVC>:/WX->"          # 不将警告视为错误
        "$<$<CXX_COMPILER_ID:MSVC>:/Zc:forScope>"  # for循环范围一致性
        "$<$<CXX_COMPILER_ID:MSVC>:/Zc:inline>"    # 移除未引用函数
        "$<$<CXX_COMPILER_ID:MSVC>:/Gd>"           # __cdecl调用约定
        "$<$<CXX_COMPILER_ID:MSVC>:/fp:precise>"   # 浮点精度
        "$<$<CXX_COMPILER_ID:MSVC>:/FC>"           # 完整路径诊断
    )

    # 调试配置
    add_compile_options(
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/JMC>"    # 仅我的代码调试
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/ZI>"     # 编辑并继续
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Od>"     # 禁用优化
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/RTC1>"   # 运行时检查
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Gm->"    # 最小重新生成关闭
    )
    
    # Release配置
    add_compile_options(
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/GS>"    # 缓冲区安全检查
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Qpar>"  # 自动并行化
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/GL>"    # 全程序优化
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Gy>"    # 函数级链接
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/O2>"    # 最大化速度
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Ob2>"   # 内联任何合适函数
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/sdl>"   # SDL检查
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Oi>"    # 内建函数
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/openmp>" # OpenMP支持
    )
    
    # RelWithDebInfo配置
    add_compile_options(
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/GS>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Qpar>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/GL>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Gy>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/O2>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Ob2>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/sdl>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Oi>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/openmp>"
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebInfo>>:/Zi>"  # 调试信息
    )
    
    # 运行时库设置
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # 链接选项
    add_link_options(
        "$<$<CONFIG:Debug>:/DEBUG>"
        "$<$<CONFIG:RelWithDebInfo>:/DEBUG:FULL>"
        "$<$<CONFIG:Release>:/DEBUG:NONE>"
        "/STACK:4194304"
    ) # Release配置的链接优化
    add_link_options(
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release,RelWithDebInfo>>:/LTCG>"  # 链接时代码生成
    )
    
    # 设置调试信息格式
    if(POLICY CMP0141)
        cmake_policy(SET CMP0141 NEW)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug>:EditAndContinue>,$<$<CONFIG:RelWithDebInfo>:ProgramDatabase>>")
    endif()
    
    # PDB文件设置
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb)
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb)
    
else()
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        message(STATUS "Found ccache program: ${CCACHE_PROGRAM}")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_PROGRAM})
    endif()
    # 非MSVC编译器设置
    add_link_options("-Wl,--stack,4194304")
endif()

add_subdirectory(src)