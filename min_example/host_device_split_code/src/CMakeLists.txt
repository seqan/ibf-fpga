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
add_library_sycl_fpga(run_min_ibf_fpga_w16_k8.fpga_emu_kernel OBJECT FPGA_DEVICE "EMULATOR" "sycl/run_min_ibf_fpga_w16_k8.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k8.fpga_emu_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w16_k6.fpga_emu_kernel STATIC FPGA_DEVICE "EMULATOR" "sycl/run_min_ibf_fpga_w16_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k6.fpga_emu_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w18_k6.fpga_emu_kernel SHARED FPGA_DEVICE "EMULATOR" "sycl/run_min_ibf_fpga_w18_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w18_k6.fpga_emu_kernel PUBLIC fpga_compilation_base)

add_executable(host_device_split_code.fpga_emu host.cpp)
target_link_libraries(host_device_split_code.fpga_emu PUBLIC intel_fpga)
target_link_libraries(host_device_split_code.fpga_emu PUBLIC run_min_ibf_fpga_w16_k8.fpga_emu_kernel)
target_link_libraries(host_device_split_code.fpga_emu PUBLIC run_min_ibf_fpga_w16_k6.fpga_emu_kernel)
# target_link_libraries(host_device_split_code.fpga_emu PUBLIC run_min_ibf_fpga_w18_k6.fpga_emu_kernel)
add_dependencies(host_device_split_code.fpga_emu run_min_ibf_fpga_w18_k6.fpga_emu_kernel)
target_compile_definitions(host_device_split_code.fpga_emu PUBLIC -DFPGA_EMULATOR)

###############################################################################
### FPGA Hardware
###############################################################################
add_library_sycl_fpga(run_min_ibf_fpga_w16_k8.fpga_kernel SHARED FPGA_DEVICE "${FPGA_DEVICE}" "sycl/run_min_ibf_fpga_w16_k8.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k8.fpga_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w16_k6.fpga_kernel SHARED FPGA_DEVICE "${FPGA_DEVICE}" "sycl/run_min_ibf_fpga_w16_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k6.fpga_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w18_k6.fpga_kernel SHARED FPGA_DEVICE "${FPGA_DEVICE}" "sycl/run_min_ibf_fpga_w18_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w18_k6.fpga_kernel PUBLIC fpga_compilation_base)

add_executable(host_device_split_code.fpga host.cpp)
target_link_libraries(host_device_split_code.fpga PUBLIC intel_fpga)
target_link_libraries(host_device_split_code.fpga PUBLIC run_min_ibf_fpga_w16_k8.fpga_kernel)
target_link_libraries(host_device_split_code.fpga PUBLIC run_min_ibf_fpga_w16_k6.fpga_kernel)
# target_link_libraries(host_device_split_code.fpga PUBLIC run_min_ibf_fpga_w18_k6.fpga_kernel)
target_compile_definitions(host_device_split_code.fpga PUBLIC -DFPGA_HARDWARE)

###############################################################################
### Generate Report
###############################################################################
sycl_fpga_report_of(run_min_ibf_fpga_w16_k8.fpga_kernel)
sycl_fpga_report_of(run_min_ibf_fpga_w16_k6.fpga_kernel)
sycl_fpga_report_of(run_min_ibf_fpga_w18_k6.fpga_kernel)

add_custom_target(host_device_split_code.report)
add_dependencies(host_device_split_code.report run_min_ibf_fpga_w16_k8.fpga_kernel.report.html)
add_dependencies(host_device_split_code.report run_min_ibf_fpga_w16_k6.fpga_kernel.report.html)
add_dependencies(host_device_split_code.report run_min_ibf_fpga_w18_k6.fpga_kernel.report.html)
