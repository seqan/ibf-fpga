cmake_minimum_required (VERSION 3.13)

project(MinIBFFPGAoneAPI CXX)

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "IntelLLVM")
    message (FATAL_ERROR "Used compiler is not ICPX; please set -DCMAKE_CXX_COMPILER=icpx")
endif()

include("cmake/set_fpga_device.cmake")

set_fpga_device()

add_subdirectory(src)
add_subdirectory(test)
