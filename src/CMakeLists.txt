include("../cmake/cxx17_filesystem.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(DEVICE_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fpga_device.cpp)
set(KERNEL_HEADER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/kernel_w23_k19_b64.hpp)
set(HOST_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/host.cpp)
set(TARGET_NAME min-ibf-fpga-oneapi)

add_subdirectory(emulator)

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/../test/data/query.fq ${CMAKE_CURRENT_BINARY_DIR}/../query.fq)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/ibfdata.bin ${CMAKE_CURRENT_BINARY_DIR}/../ibfdata.bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/thresholds_0e.bin ${CMAKE_CURRENT_BINARY_DIR}/../thresholds_0e.bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/thresholds_1e.bin ${CMAKE_CURRENT_BINARY_DIR}/../thresholds_1e.bin)

# A SYCL ahead-of-time (AoT) compile processes the device code in two stages.
# 1. The "compile" stage compiles the device code to an intermediate representation (SPIR-V).
# 2. The "link" stage invokes the compiler's FPGA backend before linking.
#    For this reason, FPGA backend flags must be passed as link flags in CMake.
set(CMAKE_CXX_FLAGS_LIST "${CMAKE_CXX_FLAGS}")
separate_arguments(CMAKE_CXX_FLAGS_LIST)

set(HARDWARE_COMPILE_FLAGS -fsycl -fintelfpga -qactypes -DFPGA_HARDWARE -Wall -Xshyper-optimized-handshaking=off)
set(HARDWARE_LINK_FLAGS -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=${FPGA_DEVICE} ${USER_HARDWARE_FLAGS} -Xshyper-optimized-handshaking=off)
# use cmake -D USER_HARDWARE_FLAGS=<flags> to set extra flags for FPGA backend compilation

add_subdirectory(report)
add_subdirectory(fpga EXCLUDE_FROM_ALL)
