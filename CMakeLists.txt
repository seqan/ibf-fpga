set(CMAKE_CXX_COMPILER icpx)

cmake_minimum_required (VERSION 3.13)

project(MinIBFFPGAoneAPI CXX)

include("cmake/set_fpga_device.cmake")

set_fpga_device()

add_subdirectory(src)
add_subdirectory(test)
