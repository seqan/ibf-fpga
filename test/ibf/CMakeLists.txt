
if (SEQAN3_FOUND AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_executable(ibf_seqan3_test ibf_seqan3_test.cpp)
    target_link_libraries(ibf_seqan3_test PUBLIC min_ibf_fpga_test "seqan3::seqan3")
    add_test(NAME ibf_seqan3_test COMMAND ibf_seqan3_test)
endif()