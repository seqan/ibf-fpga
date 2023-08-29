
add_executable(fastq_parser_test fastq_parser_test.cpp)
target_link_libraries(fastq_parser_test PUBLIC min_ibf_fpga_test)
add_test(NAME fastq_parser_test COMMAND fastq_parser_test)

add_custom_target(fastq_parser_prepare_test
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_LIST_DIR}/../data/query.fq" "${CMAKE_CURRENT_BINARY_DIR}/query.fq"
    BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/query.fq"
)

add_dependencies(fastq_parser_test fastq_parser_prepare_test)