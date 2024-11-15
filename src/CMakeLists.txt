include("../cmake/cxx17_filesystem.cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(DEVICE_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/fpga_device.cpp)
set(HOST_SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/host.cpp)
set(TARGET_NAME min-ibf-fpga-oneapi)
set(EMULATOR_TARGET ${TARGET_NAME}.fpga_emu)
set(FPGA_TARGET ${TARGET_NAME}.fpga)

if(NOT DEFINED WINDOW_SIZE_LIST)
    message("No WINDOW_SIZE_LIST supplied. Defaulting to '23'.")
    set(WINDOW_SIZE_LIST "23")
endif()

if(NOT DEFINED MIN_IBF_K_LIST)
    message("No MIN_IBF_K_LIST supplied. Defaulting to '19'.")
    set(MIN_IBF_K_LIST "19")
endif()

if(NOT DEFINED BIN_COUNT_LIST)
    message("No BIN_COUNT_LIST supplied. Defaulting to '64;8192'.")
    set(BIN_COUNT_LIST "64;8192")
endif()

if(NOT DEFINED KERNEL_COPYS_LIST)
    message("No KERNEL_COPYS_LIST supplied. Defaulting to '1;2;4'.")
    set(KERNEL_COPYS_LIST "1;2;4")
endif()

# Create symlinks to the required binary files and query file
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/../test/data/query.fq ${CMAKE_CURRENT_BINARY_DIR}/../query.fq)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/ibfdata.bin ${CMAKE_CURRENT_BINARY_DIR}/../ibfdata.bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/thresholds_0e.bin ${CMAKE_CURRENT_BINARY_DIR}/../thresholds_0e.bin)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/data/thresholds_1e.bin ${CMAKE_CURRENT_BINARY_DIR}/../thresholds_1e.bin)

# A SYCL ahead-of-time (AoT) compile processes the device code in two stages.
# 1. The "compile" stage compiles the device code to an intermediate representation (SPIR-V).
# 2. The "link" stage invokes the compiler's FPGA backend before linking.
#    For this reason, FPGA backend flags must be passed as link flags in CMake.
set(COMMON_COMPILE_FLAGS -fsycl -fintelfpga -qactypes -Xshyper-optimized-handshaking=off -Wall)
set(COMMON_LINK_FLAGS -fsycl -fintelfpga -qactypes -Xshyper-optimized-handshaking=off)
set(HARDWARE_LINK_FLAGS -Xshardware -Xstarget=${FPGA_DEVICE} ${USER_HARDWARE_FLAGS})
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

foreach(WINDOW_SIZE ${WINDOW_SIZE_LIST})
    foreach(MIN_IBF_K ${MIN_IBF_K_LIST})
        if(${WINDOW_SIZE} GREATER ${MIN_IBF_K})
            foreach(BIN_COUNT ${BIN_COUNT_LIST})
                foreach(KERNEL_COPYS ${KERNEL_COPYS_LIST})
                    set(EMULATOR_DEVICE_TARGET ${TARGET_NAME}_kernel_w${WINDOW_SIZE}_k${MIN_IBF_K}_b${BIN_COUNT}_kernels${KERNEL_COPYS}.fpga_emu)
                    add_library(${EMULATOR_DEVICE_TARGET} SHARED ${DEVICE_SOURCE_FILE})
                    target_include_directories(${EMULATOR_DEVICE_TARGET} PRIVATE ../include)
                    target_compile_definitions(${EMULATOR_DEVICE_TARGET} PRIVATE -DFPGA_EMULATOR -DWINDOW_SIZE=${WINDOW_SIZE} -DMIN_IBF_K=${MIN_IBF_K} -DBIN_COUNT=${BIN_COUNT} -DKERNEL_COPYS=${KERNEL_COPYS})
                    target_compile_options(${EMULATOR_DEVICE_TARGET} PRIVATE ${COMMON_COMPILE_FLAGS})
                    target_link_options(${EMULATOR_DEVICE_TARGET} PRIVATE ${COMMON_LINK_FLAGS})
                endforeach()
            endforeach()
        else()
            message(WARNING "Deselecting w${WINDOW_SIZE}_k${MIN_IBF_K} because window size is not bigger than k-mer size.")
        endif()
    endforeach()
endforeach()

add_executable(${EMULATOR_TARGET} ${HOST_SOURCE_FILE})
target_include_directories(${EMULATOR_TARGET} PRIVATE ../include)
target_compile_definitions(${EMULATOR_TARGET} PRIVATE -DFPGA_EMULATOR)
target_compile_options(${EMULATOR_TARGET} PRIVATE ${COMMON_COMPILE_FLAGS})
target_link_options(${EMULATOR_TARGET} PRIVATE ${COMMON_LINK_FLAGS})

###############################################################################
### Generate Report
###############################################################################
# To compile manually:
#   icpx -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=early host.cpp kernel/kernel.cpp -o fast_compile_report.a

foreach(WINDOW_SIZE ${WINDOW_SIZE_LIST})
    foreach(MIN_IBF_K ${MIN_IBF_K_LIST})
        if(${WINDOW_SIZE} GREATER ${MIN_IBF_K})
            foreach(BIN_COUNT ${BIN_COUNT_LIST})
                foreach(KERNEL_COPYS ${KERNEL_COPYS_LIST})
                    set(FPGA_EARLY_IMAGE  ${TARGET_NAME}_kernel_w${WINDOW_SIZE}_k${MIN_IBF_K}_b${BIN_COUNT}_kernels${KERNEL_COPYS}_report.a)
                    # The compile output is not an executable, but an intermediate compilation result unique to SYCL.
                    add_executable(${FPGA_EARLY_IMAGE} ${HOST_SOURCE_FILE} ${DEVICE_SOURCE_FILE})
                    target_include_directories(${FPGA_EARLY_IMAGE} PRIVATE ../include)
                    target_compile_definitions(${FPGA_EARLY_IMAGE} PRIVATE -DFPGA_HARDWARE  -DWINDOW_SIZE=${WINDOW_SIZE} -DMIN_IBF_K=${MIN_IBF_K} -DBIN_COUNT=${BIN_COUNT} -DKERNEL_COPYS=${KERNEL_COPYS})
                    target_compile_options(${FPGA_EARLY_IMAGE} PRIVATE ${COMMON_COMPILE_FLAGS})
                    target_link_options(${FPGA_EARLY_IMAGE} PRIVATE ${COMMON_LINK_FLAGS} ${HARDWARE_LINK_FLAGS} -fsycl-link=early)
                    # fsycl-link=early stops the compiler after RTL generation, before invoking Quartus®
                endforeach()
            endforeach()
        endif()
    endforeach()
endforeach()

###############################################################################
### FPGA Hardware
###############################################################################
# To compile manually:
#   icpx -fsycl -fintelfpga -qactypes -c host.cpp -o host.o
#   icpx -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=image kernel/kernel.cpp -o dev_image.a
#   icpx -fsycl -fintelfpga -qactypes host.o dev_image.a -o min-ibf-fpga-oneapi.fpga

foreach(WINDOW_SIZE ${WINDOW_SIZE_LIST})
    foreach(MIN_IBF_K ${MIN_IBF_K_LIST})
        if(${WINDOW_SIZE} GREATER ${MIN_IBF_K})
            foreach(BIN_COUNT ${BIN_COUNT_LIST})
                foreach(KERNEL_COPYS ${KERNEL_COPYS_LIST})
                    set(FPGA_DEVICE_TARGET ${TARGET_NAME}_kernel_w${WINDOW_SIZE}_k${MIN_IBF_K}_b${BIN_COUNT}_kernels${KERNEL_COPYS}.fpga)
                    add_library(${FPGA_DEVICE_TARGET} SHARED EXCLUDE_FROM_ALL ${DEVICE_SOURCE_FILE})
                    target_include_directories(${FPGA_DEVICE_TARGET} PRIVATE ../include)
                    target_compile_definitions(${FPGA_DEVICE_TARGET} PRIVATE -DFPGA_HARDWARE -DWINDOW_SIZE=${WINDOW_SIZE} -DMIN_IBF_K=${MIN_IBF_K} -DBIN_COUNT=${BIN_COUNT} -DKERNEL_COPYS=${KERNEL_COPYS})
                    target_compile_options(${FPGA_DEVICE_TARGET} PRIVATE ${COMMON_COMPILE_FLAGS})
                    target_link_options(${FPGA_DEVICE_TARGET} PRIVATE ${COMMON_LINK_FLAGS} ${HARDWARE_LINK_FLAGS} -fsycl-link=image)
                endforeach()
            endforeach()
        endif()
    endforeach()
endforeach()

add_executable(${FPGA_TARGET} EXCLUDE_FROM_ALL ${HOST_SOURCE_FILE})
target_include_directories(${FPGA_TARGET} PRIVATE ../include)
target_compile_definitions(${FPGA_TARGET} PRIVATE -DFPGA_HARDWARE)
target_compile_options(${FPGA_TARGET} PRIVATE ${COMMON_COMPILE_FLAGS})
target_link_options(${FPGA_TARGET} PRIVATE ${COMMON_LINK_FLAGS})
# target_link_libraries(${FPGA_TARGET} PRIVATE ${FPGA_DEVICE_TARGET})
