include("../cmake/cxx17_filesystem.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(DEVICE_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fpga_device.cpp)
set(KERNEL_HEADER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/kernel_w23_k19_b64.hpp)
set(HOST_SOURCE_FILE host.cpp)
set(TARGET_NAME min-ibf-fpga-oneapi)
set(EMULATOR_TARGET ${TARGET_NAME}.fpga_emu)
set(FPGA_TARGET ${TARGET_NAME}.fpga)

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

set(EMULATOR_COMPILE_FLAGS -fsycl -fintelfpga -qactypes -DFPGA_EMULATOR -Wall -Xshyper-optimized-handshaking=off)
set(EMULATOR_LINK_FLAGS -fsycl -fintelfpga -qactypes -Xshyper-optimized-handshaking=off)
set(HARDWARE_COMPILE_FLAGS -fsycl -fintelfpga -qactypes -DFPGA_HARDWARE -Wall -Xshyper-optimized-handshaking=off)
set(HARDWARE_LINK_FLAGS -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=${FPGA_DEVICE} ${USER_HARDWARE_FLAGS} -Xshyper-optimized-handshaking=off)
# use cmake -D USER_HARDWARE_FLAGS=<flags> to set extra flags for FPGA backend compilation

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
target_include_directories(${EMULATOR_TARGET} PRIVATE ../include)
target_compile_options(${EMULATOR_TARGET} PRIVATE ${EMULATOR_COMPILE_FLAGS})
target_link_options(${EMULATOR_TARGET} PRIVATE ${EMULATOR_LINK_FLAGS})
add_custom_target(fpga_emu DEPENDS ${EMULATOR_TARGET})

###############################################################################
### Generate Report
###############################################################################
# To compile manually:
#   icpx -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=early host.cpp kernel/kernel.cpp -o fast_compile_report.a
set(FPGA_EARLY_IMAGE ${TARGET_NAME}_report.a)
# The compile output is not an executable, but an intermediate compilation result unique to SYCL.
add_executable(${FPGA_EARLY_IMAGE} ${HOST_SOURCE_FILE} ${DEVICE_SOURCE_FILE})
target_include_directories(${FPGA_EARLY_IMAGE} PRIVATE ../include)
add_custom_target(report DEPENDS ${FPGA_EARLY_IMAGE})
target_compile_options(${FPGA_EARLY_IMAGE} PRIVATE ${HARDWARE_COMPILE_FLAGS})
target_link_options(${FPGA_EARLY_IMAGE} PRIVATE ${HARDWARE_LINK_FLAGS} -fsycl-link=early)
# fsycl-link=early stops the compiler after RTL generation, before invoking Quartus®

###############################################################################
### FPGA Hardware
###############################################################################
# To compile manually:
#   icpx -fsycl -fintelfpga -qactypes -c host.cpp -o host.o
#   icpx -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=image kernel/kernel.cpp -o dev_image.a
#   icpx -fsycl -fintelfpga -qactypes host.o dev_image.a -o min-ibf-fpga-oneapi.fpga

add_custom_target(fpga DEPENDS ${FPGA_TARGET})
set(DEVICE_IMAGE_OBJ "dev_image.o")

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_IMAGE_OBJ}
                   COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS_LIST} ${HARDWARE_LINK_FLAGS} -fsycl-link=image ${DEVICE_SOURCE_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_IMAGE_OBJ} -I${CMAKE_CURRENT_LIST_DIR}/../include
                   DEPENDS ${DEVICE_SOURCE_FILE} ${KERNEL_HEADER_FILE})

add_library(fpga_device.fpga OBJECT IMPORTED)
set_property(TARGET fpga_device.fpga PROPERTY IMPORTED_OBJECTS
    "${CMAKE_CURRENT_BINARY_DIR}/${DEVICE_IMAGE_OBJ}" # <build-dir>/dev_image.o
)

add_executable(${FPGA_TARGET} EXCLUDE_FROM_ALL ${HOST_SOURCE_FILE})
target_include_directories(${FPGA_TARGET} PRIVATE ../include)
target_compile_options(${FPGA_TARGET} PRIVATE ${HARDWARE_COMPILE_FLAGS})
target_link_options(${FPGA_TARGET} PRIVATE -fsycl -fintelfpga -qactypes -Xshyper-optimized-handshaking=off)
target_link_libraries(${FPGA_TARGET} PRIVATE fpga_device.fpga)