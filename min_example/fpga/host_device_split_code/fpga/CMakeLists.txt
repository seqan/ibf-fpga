

###############################################################################
### FPGA Hardware
###############################################################################
add_library_sycl_fpga(run_min_ibf_fpga_w16_k8.fpga_kernel SHARED FPGA_DEVICE "${FPGA_DEVICE}" "../src/sycl/run_min_ibf_fpga_w16_k8.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k8.fpga_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w16_k6.fpga_kernel SHARED FPGA_DEVICE "${FPGA_DEVICE}" "../src/sycl/run_min_ibf_fpga_w16_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k6.fpga_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w18_k6.fpga_kernel SHARED FPGA_DEVICE "${FPGA_DEVICE}" "../src/sycl/run_min_ibf_fpga_w18_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w18_k6.fpga_kernel PUBLIC fpga_compilation_base)

add_executable(host_device_split_code.fpga ../src/host.cpp)
target_link_libraries(host_device_split_code.fpga PUBLIC intel_fpga)
target_link_libraries(host_device_split_code.fpga PUBLIC run_min_ibf_fpga_w16_k8.fpga_kernel)
target_link_libraries(host_device_split_code.fpga PUBLIC run_min_ibf_fpga_w16_k6.fpga_kernel)
# target_link_libraries(host_device_split_code.fpga PUBLIC run_min_ibf_fpga_w18_k6.fpga_kernel)
target_compile_definitions(host_device_split_code.fpga PUBLIC -DFPGA_HARDWARE)

add_custom_target(fpga)
add_dependencies(fpga host_device_split_code.fpga)
