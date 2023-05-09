# add_library_sycl_fpga(<name> [OBJECT | STATIC | SHARED | MODULE]
#                       FPGA_DEVICE [EMULATOR | <FPGA_DEVICE>]
#                       source1 [source2 ...])
#
# Note: Use `target_compile_options(<name>_device_image [PUBLIC|PRIVATE] -flag1 -flag2)` to add flags to
#       Or `target_link_libraries(<name>_device_image [PUBLIC|PRIVATE] <interface library>)` if you have a interface library

set(_ADD_LIBRARY_SYCL_FPGA_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")

macro(add_library_sycl_fpga TARGET_NAME LIBRARY_TYPE FPGA_DEVICE_KEYWORD FPGA_DEVICE W K SOURCE1)
    if (NOT "${FPGA_DEVICE_KEYWORD}" STREQUAL "FPGA_DEVICE")
        message (FATAL_ERROR "EXPECTED Keyword 'FPGA_DEVICE', got '${FPGA_DEVICE_KEYWORD}'")
    endif ()

    string(TOLOWER "${FPGA_DEVICE}" FPGA_DEVICE_LOWER)
    string(TOUPPER "${LIBRARY_TYPE}" LIBRARY_TYPE_UPPER)
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

        add_library("${TARGET_NAME}_device_image" OBJECT "${SOURCE1}")
        target_compile_options("${TARGET_NAME}_device_image" PRIVATE -fsycl -fintelfpga -qactypes -DMIN_IBF_K=${K} -DWINDOW_SIZE=${W})
        target_link_options("${TARGET_NAME}_device_image" PRIVATE -fsycl -fintelfpga)

        if ("${LIBRARY_TYPE_UPPER}" STREQUAL "SHARED")
            set_target_properties ("${TARGET_NAME}_device_image" PROPERTIES POSITION_INDEPENDENT_CODE ON)
        endif ()

        add_library("${TARGET_NAME}" "${LIBRARY_TYPE}" "${_ADD_LIBRARY_SYCL_FPGA_BASE_DIR}/empty_source.cpp")
        if ("${LIBRARY_TYPE_UPPER}" STREQUAL "OBJECT")
            target_link_libraries("${TARGET_NAME}" INTERFACE $<TARGET_OBJECTS:${TARGET_NAME}_device_image>)
        else ()
            target_link_libraries("${TARGET_NAME}" PRIVATE "${TARGET_NAME}_device_image")
        endif ()
        target_compile_options("${TARGET_NAME}" PRIVATE -fsycl -fintelfpga -qactypes)
        target_link_options("${TARGET_NAME}" PRIVATE -fsycl -fintelfpga)
    else ()
        ###############################################################################
        ### FPGA Hardware
        ###############################################################################
        # To compile manually:
        #   dpcpp -fsycl -fintelfpga -c host.cpp -o host.o
        #   dpcpp -fsycl -fintelfpga -Xshardware -Xsboard=<FPGA_DEVICE> -fsycl-link=image kernel.cpp -o dev_image.a
        #   dpcpp -fsycl -fintelfpga host.o dev_image.a -o fast_recompile.fpga

        set(CMAKE_CXX_FLAGS_LIST "${CMAKE_CXX_FLAGS}")
        separate_arguments(CMAKE_CXX_FLAGS_LIST)

        # Compiler Flags needed to invoke `Intel(R) Quartus(R) Prime`
        set (HARDWARE_COMPILE_FLAGS "-fsycl;-fintelfpga;-Xshardware;-Xsboard=${FPGA_DEVICE};-fsycl-link=image")

        # Comment this out to debug cmake dependencies and build steps (this will compile the object file the traditional way and skips the synthesis)
        # set (HARDWARE_COMPILE_FLAGS "-c")

        set (_compile_options "$<TARGET_PROPERTY:${TARGET_NAME}_device_image,COMPILE_OPTIONS>")
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.device.image.o
                           COMMAND ${CMAKE_CXX_COMPILER}
                                   ${_compile_options}
                                   ${CMAKE_CXX_FLAGS_LIST}
                                   ${HARDWARE_COMPILE_FLAGS}
                                   ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE1}
                                #    <build-dir>/kernel.device.image.o
                                   -o ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.device.image.o
                           DEPENDS ${SOURCE1} ${KERNEL_HEADER_FILE}
                           COMMAND_EXPAND_LISTS)

        add_library("${TARGET_NAME}_device_image" OBJECT IMPORTED)
        set_property(TARGET "${TARGET_NAME}_device_image" PROPERTY IMPORTED_OBJECTS
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.device.image.o # <build-dir>/kernel.device.image.o
        )

        add_library("${TARGET_NAME}" "${LIBRARY_TYPE}" "${_ADD_LIBRARY_SYCL_FPGA_BASE_DIR}/empty_source.cpp")
        if ("${LIBRARY_TYPE_UPPER}" STREQUAL "OBJECT")
            target_link_libraries("${TARGET_NAME}" INTERFACE $<TARGET_OBJECTS:${TARGET_NAME}_device_image>)
        else ()
            target_link_libraries("${TARGET_NAME}" PRIVATE "${TARGET_NAME}_device_image")
        endif ()
        target_compile_options("${TARGET_NAME}" PRIVATE -fsycl -fintelfpga)
        target_link_options("${TARGET_NAME}" PRIVATE -fsycl -fintelfpga)
    endif ()

    set_property(TARGET "${TARGET_NAME}" PROPERTY SYCL_FPGA_SOURCE "${SOURCE1}")
    set_property(TARGET "${TARGET_NAME}" PROPERTY SYCL_FPGA_DEVICE "${FPGA_DEVICE}")
endmacro()