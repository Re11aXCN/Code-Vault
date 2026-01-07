set(VCPKG_PATH "E:/Development/vcpkg/installed/x64-windows")

set(VCPKG_BIN_PATH ${VCPKG_PATH}/bin)
set(VCPKG_DEBUG_BIN_PATH ${VCPKG_PATH}/debug/bin)
set(VCPKG_INCLUDE_PATH ${VCPKG_PATH}/include)
set(VCPKG_LIBRARY_PATH ${VCPKG_PATH}/lib)
set(VCPKG_CMAKE_SHARED_PATH ${VCPKG_PATH}/share)

set(BOOST_INCLUDE_PATH "E:/Development/boost_1_87_0/build/include")
set(BOOST_LIBRARY_PATH "E:/Development/boost_1_87_0/build/lib")

include_directories(
    ${VCPKG_INCLUDE_PATH}
    ${BOOST_INCLUDE_PATH}
    $ENV{CUDA_TOOLKIT_ROOT_DIR}/include
)

function(setup_target_path target_name)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(${target_name} PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_DEBUG_BIN_PATH};$ENV{PATH}"
        )
    else()
        set_target_properties(${target_name} PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "PATH=${VCPKG_BIN_PATH};$ENV{PATH}"
        )
    endif()
endfunction()