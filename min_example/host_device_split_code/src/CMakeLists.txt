include ("../cmake/add_library_sycl_fpga.cmake")
include ("../cmake/sycl_fpga_report_of.cmake")

set(TARGET_NAME host_device_split_code)

# FPGA board selection
if(NOT DEFINED FPGA_BOARD)
    set(FPGA_BOARD "intel_a10gx_pac:pac_a10")
    message(STATUS "FPGA_BOARD was not specified.\
                    \nConfiguring the design to run on the default FPGA board ${FPGA_BOARD} (Intel(R) PAC with Intel Arria(R) 10 GX FPGA). \
                    \nPlease refer to the README for information on board selection.")
else()
    message(STATUS "Configuring the design to run on FPGA board ${FPGA_BOARD}")
endif()

# Common Compilation Flags of host and device code
# This is a Windows-specific flag that enables exception handling in host code
if(WIN32)
    set(WIN_FLAG "/EHsc")
endif()

add_library(fpga_compilation_base INTERFACE)
target_compile_options(fpga_compilation_base INTERFACE -Wall "${WIN_FLAG}")

# NOTE: -fsycl -fintelfpga is needed for the host code as well; Maybe because of the extension sycl::ext::intel::fpga_selector?
# It compiles without, but an runtime error occurs
add_library(intel_fpga INTERFACE)
target_compile_options(intel_fpga INTERFACE -fsycl -fintelfpga)
target_link_options(intel_fpga INTERFACE -fsycl -fintelfpga)

###############################################################################
### FPGA Emulator
###############################################################################
add_library_sycl_fpga(run_min_ibf_fpga_w16_k8.fpga_emu_kernel OBJECT FPGA_BOARD "EMULATOR" "sycl/run_min_ibf_fpga_w16_k8.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k8.fpga_emu_kernel_device_image PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w16_k6.fpga_emu_kernel STATIC FPGA_BOARD "EMULATOR" "sycl/run_min_ibf_fpga_w16_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k6.fpga_emu_kernel_device_image PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w18_k6.fpga_emu_kernel SHARED FPGA_BOARD "EMULATOR" "sycl/run_min_ibf_fpga_w18_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w18_k6.fpga_emu_kernel_device_image PUBLIC fpga_compilation_base)

add_executable(${TARGET_NAME}.fpga_emu host.cpp)
target_link_libraries(${TARGET_NAME}.fpga_emu PUBLIC intel_fpga)
target_link_libraries(${TARGET_NAME}.fpga_emu PUBLIC run_min_ibf_fpga_w16_k8.fpga_emu_kernel)
target_link_libraries(${TARGET_NAME}.fpga_emu PUBLIC run_min_ibf_fpga_w16_k6.fpga_emu_kernel)
# target_link_libraries(${TARGET_NAME}.fpga_emu PUBLIC run_min_ibf_fpga_w18_k6.fpga_emu_kernel)
add_dependencies(${TARGET_NAME}.fpga_emu run_min_ibf_fpga_w18_k6.fpga_emu_kernel)
target_compile_definitions(${TARGET_NAME}.fpga_emu PUBLIC -DFPGA_EMULATOR)

###############################################################################
### FPGA Hardware
###############################################################################
add_library_sycl_fpga(run_min_ibf_fpga_w16_k8.fpga_kernel STATIC FPGA_BOARD "${FPGA_BOARD}" "sycl/run_min_ibf_fpga_w16_k8.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k8.fpga_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w16_k6.fpga_kernel STATIC FPGA_BOARD "${FPGA_BOARD}" "sycl/run_min_ibf_fpga_w16_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k6.fpga_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w18_k6.fpga_kernel STATIC FPGA_BOARD "${FPGA_BOARD}" "sycl/run_min_ibf_fpga_w18_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w18_k6.fpga_kernel PUBLIC fpga_compilation_base)

add_executable(${TARGET_NAME}.fpga host.cpp)
target_link_libraries(${TARGET_NAME}.fpga PUBLIC intel_fpga)
target_link_libraries(${TARGET_NAME}.fpga PUBLIC run_min_ibf_fpga_w16_k8.fpga_kernel)
target_link_libraries(${TARGET_NAME}.fpga PUBLIC run_min_ibf_fpga_w16_k6.fpga_kernel)
target_link_libraries(${TARGET_NAME}.fpga PUBLIC run_min_ibf_fpga_w18_k6.fpga_kernel)

###############################################################################
### Generate Report
###############################################################################
sycl_fpga_report_of(run_min_ibf_fpga_w16_k8.fpga_kernel)
sycl_fpga_report_of(run_min_ibf_fpga_w16_k6.fpga_kernel)
sycl_fpga_report_of(run_min_ibf_fpga_w18_k6.fpga_kernel)

add_custom_target(${TARGET_NAME}.report)
add_dependencies(${TARGET_NAME}.report run_min_ibf_fpga_w16_k8.fpga_kernel.report.html)
add_dependencies(${TARGET_NAME}.report run_min_ibf_fpga_w16_k6.fpga_kernel.report.html)
add_dependencies(${TARGET_NAME}.report run_min_ibf_fpga_w18_k6.fpga_kernel.report.html)