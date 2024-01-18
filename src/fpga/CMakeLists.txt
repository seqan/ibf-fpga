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

add_executable(min-ibf-fpga-oneapi.fpga ../host.cpp)
target_include_directories(min-ibf-fpga-oneapi.fpga PRIVATE ../../include)
target_compile_definitions(min-ibf-fpga-oneapi.fpga PRIVATE -DFPGA_HARDWARE)
target_compile_options(min-ibf-fpga-oneapi.fpga PRIVATE -fsycl -fintelfpga -qactypes -Wall -Xshyper-optimized-handshaking=off)
target_link_options(min-ibf-fpga-oneapi.fpga PRIVATE -fsycl -fintelfpga -qactypes -Xshyper-optimized-handshaking=off)
target_link_libraries(min-ibf-fpga-oneapi.fpga PRIVATE fpga_device.fpga)

add_custom_target(fpga DEPENDS min-ibf-fpga-oneapi.fpga)