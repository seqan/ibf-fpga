include("../cmake/cxx17_filesystem.cmake")
include("../cmake/sycl_fpga_set_default_variables.cmake")
include("../cmake/sycl_fpga_build_kernels.cmake")

set_default_variables()

set(HOST_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/host.cpp)
set(DEVICE_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fpga_device.cpp)
set(TARGET_NAME min-ibf-fpga-oneapi)
set(INCLUDE_PATH ../include)

# Create symlinks to the required binary files and query file
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/../test/data/query.fq ${CMAKE_CURRENT_BINARY_DIR}/../query.fq)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/ibfdata.bin ${CMAKE_CURRENT_BINARY_DIR}/../ibfdata.bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/thresholds_0e.bin ${CMAKE_CURRENT_BINARY_DIR}/../thresholds_0e.bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/thresholds_1e.bin ${CMAKE_CURRENT_BINARY_DIR}/../thresholds_1e.bin)

###############################################################################
### FPGA Emulator
###############################################################################

add_executable(${EMULATOR_TARGET} ${HOST_SOURCE_FILE})
target_include_directories(${EMULATOR_TARGET} PRIVATE ${INCLUDE_PATH})
target_compile_definitions(${EMULATOR_TARGET} PRIVATE -DFPGA_EMULATOR)
target_compile_options(${EMULATOR_TARGET} PRIVATE ${COMMON_COMPILE_FLAGS} ${EMULATOR_AC_INT_FLAG})
target_link_options(${EMULATOR_TARGET} PRIVATE ${COMMON_LINK_FLAGS} ${EMULATOR_AC_INT_FLAG})

build_kernels_emulator()

###############################################################################
### Generate Report
###############################################################################

build_kernels_report()

###############################################################################
### FPGA Hardware
###############################################################################

add_executable(${FPGA_TARGET} EXCLUDE_FROM_ALL ${HOST_SOURCE_FILE})
target_include_directories(${FPGA_TARGET} PRIVATE ${INCLUDE_PATH})
target_compile_definitions(${FPGA_TARGET} PRIVATE -DFPGA_HARDWARE)
target_compile_options(${FPGA_TARGET} PRIVATE ${COMMON_COMPILE_FLAGS} -qactypes)
target_link_options(${FPGA_TARGET} PRIVATE ${COMMON_LINK_FLAGS} -qactypes)

build_kernels_hardware()
