###############################################################################
### Generate Report
###############################################################################
# To compile manually:
#   icpx -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=early host.cpp kernel/kernel.cpp -o fast_compile_report.a
set(FPGA_EARLY_IMAGE ${TARGET_NAME}_report.a)
# The compile output is not an executable, but an intermediate compilation result unique to SYCL.
add_executable(${FPGA_EARLY_IMAGE} ${HOST_SOURCE_FILE} ${DEVICE_SOURCE_FILE})
target_include_directories(${FPGA_EARLY_IMAGE} PRIVATE ../../include)
add_custom_target(report DEPENDS ${FPGA_EARLY_IMAGE})
target_compile_options(${FPGA_EARLY_IMAGE} PRIVATE ${HARDWARE_COMPILE_FLAGS})
target_link_options(${FPGA_EARLY_IMAGE} PRIVATE ${HARDWARE_LINK_FLAGS} -fsycl-link=early)
# fsycl-link=early stops the compiler after RTL generation, before invoking Quartus®