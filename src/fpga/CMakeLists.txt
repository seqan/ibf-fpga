set(FPGA_TARGET ${TARGET_NAME}.fpga)

###############################################################################
### FPGA Hardware
###############################################################################
# To compile manually:
#   icpx -fsycl -fintelfpga -qactypes -c host.cpp -o host.o
#   icpx -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=image kernel/kernel.cpp -o dev_image.a
#   icpx -fsycl -fintelfpga -qactypes host.o dev_image.a -o min-ibf-fpga-oneapi.fpga

add_library(fpga_device.fpga SHARED ../fpga_device.cpp)
target_include_directories(fpga_device.fpga PRIVATE ../../include)
target_link_libraries(fpga_device.fpga PRIVATE intel_fpga_device)

add_executable(${FPGA_TARGET} ${HOST_SOURCE_FILE})
target_include_directories(${FPGA_TARGET} PRIVATE ../../include)
target_compile_options(${FPGA_TARGET} PRIVATE ${HARDWARE_COMPILE_FLAGS})
target_link_options(${FPGA_TARGET} PRIVATE -fsycl -fintelfpga -qactypes -Xshyper-optimized-handshaking=off)
target_link_libraries(${FPGA_TARGET} PRIVATE fpga_device.fpga)