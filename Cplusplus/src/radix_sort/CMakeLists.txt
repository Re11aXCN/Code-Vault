file(GLOB_RECURSE RADIXSORT_SOURCES CONFIGURE_DEPENDS "*.cpp")
if(RADIXSORT_SOURCES)
  add_executable(radix_sort_exe
      bechmark_radixsort_par.cpp
   )

  set_target_properties(radix_sort_exe PROPERTIES
      WIN32_EXECUTABLE OFF    # 在 Windows 系统中，运行时不启动控制台窗口，默认OFF
      LINK_WHAT_YOU_USE ON    # 告诉编译器不要自动剔除没有引用符号的链接库，默认OFF
  )
  if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      setup_target_path(radix_sort_exe)
  endif()

  find_package(benchmark REQUIRED COMPONENTS benchmark benchmark_main HINTS ${VCPKG_CMAKE_SHARED_PATH})
  
  target_include_directories(radix_sort_exe PRIVATE 
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  )
  
  target_link_libraries(radix_sort_exe PRIVATE
      benchmark::benchmark
      benchmark::benchmark_main
  )

  # 主程序需要的DLL列表
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(MAIN_DLL_LIST
          "benchmark*.dll"
      )
      copy_dlls_to_target(radix_sort_exe "${MAIN_DLL_LIST}" ${VCPKG_DEBUG_BIN_PATH})
  else()
      set(MAIN_DLL_LIST
          "benchmark*.dll"
      )
      copy_dlls_to_target(radix_sort_exe "${MAIN_DLL_LIST}" ${VCPKG_BIN_PATH})
  endif()
endif()

