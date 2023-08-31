
add_executable(read_in_binary_data_test read_in_binary_data_test.cpp)
target_link_libraries(read_in_binary_data_test PUBLIC min_ibf_fpga_test)
add_test(NAME read_in_binary_data_test COMMAND read_in_binary_data_test)

add_custom_target(read_in_binary_data_prepare_test
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_LIST_DIR}/../data/three_char.bin" "${CMAKE_CURRENT_BINARY_DIR}/three_char.bin"
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_LIST_DIR}/../../src/data/thresholds_0e.bin" "${CMAKE_CURRENT_BINARY_DIR}/thresholds_0e.bin"
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_LIST_DIR}/../../src/data/thresholds_1e.bin" "${CMAKE_CURRENT_BINARY_DIR}/thresholds_1e.bin"
    BYPRODUCTS
        "${CMAKE_CURRENT_BINARY_DIR}/three_char.bin"
        "${CMAKE_CURRENT_BINARY_DIR}/thresholds_0e.bin"
        "${CMAKE_CURRENT_BINARY_DIR}/thresholds_1e.bin"
)

add_dependencies(read_in_binary_data_test read_in_binary_data_prepare_test)

add_executable(write_out_binary_data_test write_out_binary_data_test.cpp)
target_link_libraries(write_out_binary_data_test PUBLIC min_ibf_fpga_test)
add_test(NAME write_out_binary_data_test COMMAND write_out_binary_data_test)

add_executable(print_results_test print_results_test.cpp)
target_link_libraries(print_results_test PUBLIC min_ibf_fpga_test)
add_test(NAME print_results_test COMMAND print_results_test)