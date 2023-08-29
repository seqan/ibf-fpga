include("../cmake/cxx17_filesystem.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Common Flags for SYCL Fpga targets
add_library(intel_fpga_sycl INTERFACE)
target_compile_options(intel_fpga_sycl INTERFACE -fsycl -fintelfpga)
target_link_options(intel_fpga_sycl INTERFACE -fsycl -fintelfpga)

# Base for FPGA Host AND Device Target EMULATOR
add_library(intel_fpga_emulator_host_and_device INTERFACE)
target_compile_definitions(intel_fpga_emulator_host_and_device INTERFACE -DFPGA_EMULATOR)
target_link_libraries(intel_fpga_emulator_host_and_device INTERFACE intel_fpga_sycl)

# Base for FPGA Host Target
add_library(intel_fpga_host INTERFACE)
target_compile_definitions(intel_fpga_host INTERFACE -DFPGA_HARDWARE)
target_link_libraries(intel_fpga_host INTERFACE intel_fpga_sycl)

# Base for FPGA Device Target
# use cmake -D USER_HARDWARE_FLAGS=<flags> to set extra flags for FPGA backend compilation
add_library(intel_fpga_device INTERFACE)
target_compile_definitions(intel_fpga_device INTERFACE -DFPGA_HARDWARE)
target_link_libraries(intel_fpga_device INTERFACE intel_fpga_sycl)
target_link_options(intel_fpga_device INTERFACE -Xshardware -Xstarget=${FPGA_DEVICE} ${USER_HARDWARE_FLAGS})

######## CUSTOM Flags for this build #########
target_compile_options(intel_fpga_sycl INTERFACE -qactypes -Xshyper-optimized-handshaking=off -Wall)
target_link_options(intel_fpga_sycl INTERFACE -qactypes -Xshyper-optimized-handshaking=off)

add_subdirectory(emulator)

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/../test/data/query.fq ${CMAKE_CURRENT_BINARY_DIR}/../query.fq)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/ibfdata.bin ${CMAKE_CURRENT_BINARY_DIR}/../ibfdata.bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/thresholds_0e.bin ${CMAKE_CURRENT_BINARY_DIR}/../thresholds_0e.bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/thresholds_1e.bin ${CMAKE_CURRENT_BINARY_DIR}/../thresholds_1e.bin)

# A SYCL ahead-of-time (AoT) compile processes the device code in two stages.
# 1. The "compile" stage compiles the device code to an intermediate representation (SPIR-V).
# 2. The "link" stage invokes the compiler's FPGA backend before linking.
#    For this reason, FPGA backend flags must be passed as link flags in CMake.

add_subdirectory(report)
add_subdirectory(fpga EXCLUDE_FROM_ALL)
