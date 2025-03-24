macro(set_default_variables)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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
        message("No KERNEL_COPYS_LIST supplied. Defaulting to '1;2'.")
        set(KERNEL_COPYS_LIST "1;2")
    endif()

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 2022)
        set(BOARD_FLAG "-Xstarget")
        set(EMULATOR_AC_INT_FLAG "-qactypes")
    else()
        set(BOARD_FLAG "-Xsboard")
    endif()

    set(COMMON_COMPILE_FLAGS -fsycl -fintelfpga -Xshyper-optimized-handshaking=off -Wall)
    set(COMMON_LINK_FLAGS -fsycl -fintelfpga -Xshyper-optimized-handshaking=off)
    set(HARDWARE_LINK_FLAGS -Xshardware ${BOARD_FLAG}=${FPGA_DEVICE} ${USER_HARDWARE_FLAGS})
    # use cmake -D USER_HARDWARE_FLAGS=<flags> to set extra flags for FPGA backend compilation
endmacro()