cmake_minimum_required (VERSION 3.4)

project (host_device_split_code CXX)

# FPGA board selection
if(NOT DEFINED FPGA_DEVICE)
    set(FPGA_DEVICE "intel_s10sx_pac:pac_s10_usm")
    message(STATUS "FPGA_DEVICE was not specified.\
                    \nConfiguring the design to run on the default FPGA board ${FPGA_DEVICE}.\
                    \nPlease refer to the README for information on board selection.")
else()
    message(STATUS "Configuring the design to run on FPGA board ${FPGA_DEVICE}")
endif()

include ("../../cmake/add_library_sycl_fpga.cmake")
include ("../../cmake/sycl_fpga_report_of.cmake")

include ("../../cmake/cxx17_filesystem.cmake")

add_subdirectory (src)
