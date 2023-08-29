# A SYCL ahead-of-time (AoT) compile processes the device code in two stages.
# 1. The "compile" stage compiles the device code to an intermediate representation (SPIR-V).
# 2. The "link" stage invokes the compiler's FPGA backend before linking.
#    For this reason, FPGA backend flags must be passed as link flags in CMake.

###############################################################################
### FPGA Emulator
###############################################################################
# To compile in a single command:
#    icpx -fsycl -fintelfpga -qactypes -DFPGA_EMULATOR host.cpp kernel/kernel.cpp -o min-ibf-fpga-oneapi.fpga_emu
# CMake executes:
#    [compile] icpx -fsycl -fintelfpga -qactypes -DFPGA_EMULATOR -o host.cpp.o -c host.cpp
#    [compile] icpx -fsycl -fintelfpga -qactypes -DFPGA_EMULATOR -o kernel/kernel.cpp.o -c kernel/kernel.cpp
#    [link]    icpx -fsycl -fintelfpga -qactypes host.cpp.o kernel/kernel.cpp.o -o min-ibf-fpga-oneapi.fpga_emu
add_executable(min-ibf-fpga-oneapi.fpga_emu ../host.cpp ../fpga_device.cpp)
target_include_directories(min-ibf-fpga-oneapi.fpga_emu PRIVATE ../../include)
target_compile_definitions(min-ibf-fpga-oneapi.fpga_emu PRIVATE -DFPGA_EMULATOR)
target_compile_options(min-ibf-fpga-oneapi.fpga_emu PRIVATE -fsycl -fintelfpga -qactypes -Wall -Xshyper-optimized-handshaking=off)
target_link_options(min-ibf-fpga-oneapi.fpga_emu PRIVATE -fsycl -fintelfpga -qactypes -Xshyper-optimized-handshaking=off)

add_custom_target(fpga_emu DEPENDS min-ibf-fpga-oneapi.fpga_emu)
