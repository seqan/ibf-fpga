
###############################################################################
### FPGA Emulator
###############################################################################
add_library_sycl_fpga(run_min_ibf_fpga_w16_k8.fpga_emu_kernel OBJECT FPGA_DEVICE "EMULATOR" "../src/sycl/run_min_ibf_fpga_w16_k8.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k8.fpga_emu_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w16_k6.fpga_emu_kernel STATIC FPGA_DEVICE "EMULATOR" "../src/sycl/run_min_ibf_fpga_w16_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w16_k6.fpga_emu_kernel PUBLIC fpga_compilation_base)

add_library_sycl_fpga(run_min_ibf_fpga_w18_k6.fpga_emu_kernel SHARED FPGA_DEVICE "EMULATOR" "../src/sycl/run_min_ibf_fpga_w18_k6.cpp")
target_link_libraries(run_min_ibf_fpga_w18_k6.fpga_emu_kernel PUBLIC fpga_compilation_base)

add_executable(host_device_split_code.fpga_emu ../src/host.cpp)
target_link_libraries(host_device_split_code.fpga_emu PUBLIC intel_fpga)
target_link_libraries(host_device_split_code.fpga_emu PUBLIC run_min_ibf_fpga_w16_k8.fpga_emu_kernel)
target_link_libraries(host_device_split_code.fpga_emu PUBLIC run_min_ibf_fpga_w16_k6.fpga_emu_kernel)
# target_link_libraries(host_device_split_code.fpga_emu PUBLIC run_min_ibf_fpga_w18_k6.fpga_emu_kernel)
add_dependencies(host_device_split_code.fpga_emu run_min_ibf_fpga_w18_k6.fpga_emu_kernel)
target_compile_definitions(host_device_split_code.fpga_emu PUBLIC -DFPGA_EMULATOR)

add_custom_target(fpga_emu)
add_dependencies(fpga_emu host_device_split_code.fpga_emu)
