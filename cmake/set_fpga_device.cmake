macro(set_fpga_device)
    # Try to detect board by searching for known boards in the output of "aoc -list-boards"
    if(NOT DEFINED FPGA_DEVICE)
        set(known_boards "ofs_ia840f_usm" "ofs_d5005_usm")

        execute_process(
            COMMAND aoc -list-boards
            OUTPUT_VARIABLE output
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        foreach(search_string IN LISTS known_boards)
            string(FIND "${output}" "${search_string}" string_pos)

            # If the string is found
            if(NOT string_pos EQUAL -1)
                message(STATUS "Found board '${search_string}' in the output of 'aoc -list-boards'.")
                set(FPGA_DEVICE "${search_string}")
                break()
            endif()
        endforeach()
    endif()

    # Fall back to pre defined device for FPGA board selection
    if(NOT DEFINED FPGA_DEVICE)
        set(FPGA_DEVICE "intel_s10sx_pac:pac_s10_usm")
        message(STATUS "FPGA_DEVICE was not specified.\
                        \nConfiguring the design to run on the default FPGA board ${FPGA_DEVICE}.\
                        \nPlease refer to the README for information on board selection.")
    else()
        message(STATUS "Configuring the design to run on FPGA board ${FPGA_DEVICE}.")
    endif()
endmacro()