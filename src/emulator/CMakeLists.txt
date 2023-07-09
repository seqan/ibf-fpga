set(EMULATOR_TARGET ${TARGET_NAME}.fpga_emu)

# A SYCL ahead-of-time (AoT) compile processes the device code in two stages.
# 1. The "compile" stage compiles the device code to an intermediate representation (SPIR-V).
# 2. The "link" stage invokes the compiler's FPGA backend before linking.
#    For this reason, FPGA backend flags must be passed as link flags in CMake.

set(EMULATOR_COMPILE_FLAGS -fsycl -fintelfpga -qactypes -DFPGA_EMULATOR -Wall -Xshyper-optimized-handshaking=off)
set(EMULATOR_LINK_FLAGS -fsycl -fintelfpga -qactypes -Xshyper-optimized-handshaking=off)

###############################################################################
### FPGA Emulator
###############################################################################
# To compile in a single command:
#    icpx -fsycl -fintelfpga -qactypes -DFPGA_EMULATOR host.cpp kernel/kernel.cpp -o min-ibf-fpga-oneapi.fpga_emu
# CMake executes:
#    [compile] icpx -fsycl -fintelfpga -qactypes -DFPGA_EMULATOR -o host.cpp.o -c host.cpp
#    [compile] icpx -fsycl -fintelfpga -qactypes -DFPGA_EMULATOR -o kernel/kernel.cpp.o -c kernel/kernel.cpp
#    [link]    icpx -fsycl -fintelfpga -qactypes host.cpp.o kernel/kernel.cpp.o -o min-ibf-fpga-oneapi.fpga_emu
add_executable(${EMULATOR_TARGET} ${HOST_SOURCE_FILE} ${DEVICE_SOURCE_FILE})
target_include_directories(${EMULATOR_TARGET} PRIVATE ../../include)
target_compile_options(${EMULATOR_TARGET} PRIVATE ${EMULATOR_COMPILE_FLAGS})
target_link_options(${EMULATOR_TARGET} PRIVATE ${EMULATOR_LINK_FLAGS})
add_custom_target(fpga_emu DEPENDS ${EMULATOR_TARGET})
