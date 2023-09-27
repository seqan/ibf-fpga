

string (CONCAT app_w23_k19_b64_e1_cout
    "query0_e0_012_e1_0123\t0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,\n"
    "query1_e0_1_e1_1\t2,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,\n"
    "query2_e0_3_e1_23\t0,1,4,5,8,9,12,13,16,17,20,21,24,25,28,29,32,33,36,37,40,41,44,45,48,49,52,53,56,57,60,61,\n"
    "query3_never\t\n")

if (TARGET min-ibf-fpga-oneapi.fpga_emu)
add_test(NAME app_w23_k19_b64_e1.fpga_emu COMMAND min-ibf-fpga-oneapi.fpga_emu WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
set_tests_properties(app_w23_k19_b64_e1.fpga_emu PROPERTIES PASS_REGULAR_EXPRESSION "${app_w23_k19_b64_e1_cout}")
endif ()

if (TARGET min-ibf-fpga-oneapi.fpga)
# See https://stackoverflow.com/a/61573823
add_test(NAME app_w23_k19_b64_e1.fpga
    COMMAND sh -c "if [ -f \"$<TARGET_FILE:min-ibf-fpga-oneapi.fpga>\" ]; then $<TARGET_FILE:min-ibf-fpga-oneapi.fpga>; else exit 127; fi"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
set_tests_properties(app_w23_k19_b64_e1.fpga PROPERTIES PASS_REGULAR_EXPRESSION "${app_w23_k19_b64_e1_cout}" SKIP_RETURN_CODE 127)
endif ()
