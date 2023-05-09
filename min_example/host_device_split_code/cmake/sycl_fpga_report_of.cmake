

# TODO: Support a syntax like this
# TODO: Example report specifies all source files. Do we need this? Or are just the source files needed of the device kernel?
# ```
# add_executable(device device.cpp)
# sycl_fpga_report_of(device)
# ```
macro(sycl_fpga_report_of TARGET_NAME)
    cmake_minimum_required (VERSION 3.19...3.25)

    ###############################################################################
    ### Generate Report
    ###############################################################################
    # To compile manually:
    #   dpcpp -fsycl -fintelfpga -Xshardware -Xsboard=<FPGA_BOARD> -fsycl-link=early host.cpp kernel.cpp -o fast_compile_report.a
    # A DPC++ ahead-of-time (AoT) compile processes the device code in two stages.
    # 1. The "compile" stage compiles the device code to an intermediate representation (SPIR-V).
    # 2. The "link" stage invokes the compiler's FPGA backend before linking.
    #    For this reason, FPGA backend flags must be passed as link flags in CMake.
    set(REPORT_NAME "${TARGET_NAME}.report")

    get_property(_source TARGET "${TARGET_NAME}" PROPERTY SYCL_FPGA_SOURCE)
    get_property(_board TARGET "${TARGET_NAME}" PROPERTY SYCL_FPGA_BOARD)

    if (NOT _source OR NOT _board)
        message(FATAL_ERROR "sycl_fpga_report_of: target `${TARGET_NAME}` was not created by add_library_sycl_fpga")
    endif ()

    string(TOLOWER "${_board}" _board_lower)
    if (FPGA_BOARD_LOWER STREQUAL "emulator")
        message(FATAL_ERROR "sycl_fpga_report_of: can't create report of emulator target `${TARGET_NAME}`")
    endif ()

    # The compile output is not an executable, but an intermediate compilation result unique to DPC++.
    add_executable(${REPORT_NAME} EXCLUDE_FROM_ALL ${_source})
    set_target_properties(${REPORT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "CMakeFiles/")
    target_compile_options(${REPORT_NAME} PUBLIC -fsycl -fintelfpga $<TARGET_PROPERTY:${TARGET_NAME},COMPILE_OPTIONS>)
    target_link_options(${REPORT_NAME} PUBLIC -fsycl -fintelfpga -Xshardware -Xsboard=${_board} -fsycl-link=early)
    # fsycl-link=early stops the compiler after RTL generation, before invoking Quartus®

    set(report_html_full_path "$<TARGET_FILE_DIR:${REPORT_NAME}>/$<TARGET_FILE_BASE_NAME:${REPORT_NAME}>.prj/reports/report.html")
    set(report_html_name "${TARGET_NAME}.report.html")
    set(report_html_symlink_path_ "$<TARGET_PROPERTY:${TARGET_NAME},TARGET_FILE_DIR>")
    set(report_html_symlink_path "$<$<BOOL:${report_html_symlink_path_}>:${report_html_symlink_path_}/>")
    set(report_html_short_path "${report_html_symlink_path}${report_html_name}")

    cmake_policy(PUSH)
    # https://cmake.org/cmake/help/latest/policy/CMP0112.html#policy:CMP0112
    # Target file component generator expressions do not add target dependencies.
    # CMake 3.19 and above prefer to not add this dependency.
    cmake_policy(SET CMP0112 NEW)
    add_custom_target(
        "${TARGET_NAME}.report.html"
        ALL
        COMMAND
            ${CMAKE_COMMAND} -E
            create_symlink
                "'${report_html_full_path}'"
                "'${report_html_short_path}'"
        COMMENT "Created ${report_html_name}"
        DEPENDS "${REPORT_NAME}"
    )
    cmake_policy(POP)
endmacro()
