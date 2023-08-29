# add_library_sycl_fpga(<name> [OBJECT | STATIC | SHARED | MODULE]
#                       FPGA_DEVICE [EMULATOR | <FPGA_DEVICE>]
#                       source1 [source2 ...])
#
# Note: Use `target_compile_options(<name>_device_image [PUBLIC|PRIVATE] -flag1 -flag2)` to add flags to
#       Or `target_link_libraries(<name>_device_image [PUBLIC|PRIVATE] <interface library>)` if you have a interface library

set(_ADD_LIBRARY_SYCL_FPGA_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

macro(add_library_sycl_fpga TARGET_NAME LIBRARY_TYPE FPGA_DEVICE_KEYWORD FPGA_DEVICE SOURCE1)
    if (NOT "${FPGA_DEVICE_KEYWORD}" STREQUAL "FPGA_DEVICE")
        message (FATAL_ERROR "EXPECTED Keyword 'FPGA_DEVICE', got '${FPGA_DEVICE_KEYWORD}'")
    endif ()

    string(TOLOWER "${FPGA_DEVICE}" FPGA_DEVICE_LOWER)

    add_library("${TARGET_NAME}" "${LIBRARY_TYPE}" "${SOURCE1}")
    target_compile_options("${TARGET_NAME}" PRIVATE -fsycl -fintelfpga)
    target_link_options("${TARGET_NAME}" PRIVATE -fsycl -fintelfpga)

    if (FPGA_DEVICE_LOWER STREQUAL "emulator")
        ###############################################################################
        ### FPGA Emulator
        ###############################################################################
        # To compile in a single command:
        #    dpcpp -fsycl -fintelfpga -DFPGA_EMULATOR host.cpp kernel.cpp -o fast_recompile.fpga_emu
        # CMake executes:
        #    [compile] dpcpp -fsycl -fintelfpga -DFPGA_EMULATOR -o host.cpp.o -c host.cpp
        #    [compile] dpcpp -fsycl -fintelfpga -DFPGA_EMULATOR -o kernel.cpp.o -c kernel.cpp
        #    [link]    dpcpp -fsycl -fintelfpga host.cpp.o kernel.cpp.o -o fast_recompile.fpga_emu
    else ()
        ###############################################################################
        ### FPGA Hardware
        ###############################################################################
        # To compile manually:
        #   dpcpp -fsycl -fintelfpga -c host.cpp -o host.o
        #   dpcpp -fsycl -fintelfpga -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=image kernel.cpp -o dev_image.a
        #   dpcpp -fsycl -fintelfpga host.o dev_image.a -o fast_recompile.fpga

        target_link_options("${TARGET_NAME}" PRIVATE -Xshardware -Xstarget=${FPGA_DEVICE})
    endif ()

    set_property(TARGET "${TARGET_NAME}" PROPERTY SYCL_FPGA_SOURCE "${SOURCE1}")
    set_property(TARGET "${TARGET_NAME}" PROPERTY SYCL_FPGA_DEVICE "${FPGA_DEVICE}")
endmacro()