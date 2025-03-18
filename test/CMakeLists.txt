
cmake_minimum_required(VERSION 3.16)
project (min_ibf_fpga_test)

include("../cmake/cxx17_filesystem.cmake")

enable_testing()

add_library(min_ibf_fpga_test INTERFACE)
target_include_directories(min_ibf_fpga_test INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../include")
target_include_directories(min_ibf_fpga_test INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(min_ibf_fpga_test INTERFACE cxx_std_17)

# Note: Our OpenCL installation will not be found by standard search paths. We therefore search first the OpenCL FPGA installation and then set the environment variable OCL_ROOT to
find_path (OpenCL_FGPA_INCLUDE_DIR NAMES CL/cl.h HINTS "/opt/intel/oneapi/compiler/latest/linux/lib/oclfpga/host/include/")
if (OpenCL_FGPA_INCLUDE_DIR AND NOT DEFINED ENV{OCL_ROOT})
    set(ENV{OCL_ROOT} "${OpenCL_FGPA_INCLUDE_DIR}")
endif ()
find_package(OpenCL)

# SeqAn3 only works with gcc
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
find_package(SeqAn3)
endif ()

#add_subdirectory(../external/min-ibf-fpga external-min-ibf-fpga-build)

add_subdirectory(app)
add_subdirectory(fastq)
add_subdirectory(ibf)
add_subdirectory(io)
add_subdirectory(minimizer)
add_subdirectory(test)
add_subdirectory(threshold)