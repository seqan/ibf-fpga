
if (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
    add_executable(minimizer_sycl_test minimizer_sycl_test.cpp)
    target_link_libraries(minimizer_sycl_test PUBLIC min_ibf_fpga_test)
    target_compile_definitions(minimizer_sycl_test PRIVATE -DFPGA_EMULATOR -DWINDOW_SIZE=23 -DMIN_IBF_K=19 -DBIN_COUNT=64)
    set_target_properties(minimizer_sycl_test PROPERTIES COMPILE_FLAGS "-Wall -fsycl -fintelfpga -qactypes")
    set_target_properties(minimizer_sycl_test PROPERTIES LINK_FLAGS "-fsycl -fintelfpga -qactypes")
    add_test(NAME minimizer_sycl_test COMMAND minimizer_sycl_test)
endif ()

# if (OPENCL_FOUND)
#     add_executable(minimizer_opencl_test minimizer_opencl_test.cpp)
#     target_link_libraries(minimizer_opencl_test PUBLIC min_ibf_fpga_test "OpenCL::OpenCL")
#     add_test(NAME minimizer_opencl_test COMMAND minimizer_opencl_test)

#     add_custom_target(minimizer_opencl_prepare_test
#         COMMAND ${CMAKE_COMMAND} -E create_symlink "$<TARGET_PROPERTY:aocx_ibf_w19_k19_h2_b64_kernels1_test_emu,AOCX_FILE>" "${CMAKE_CURRENT_BINARY_DIR}/ibf_w19_k19_h2_b64.aocx"
#         COMMAND ${CMAKE_COMMAND} -E create_symlink "$<TARGET_PROPERTY:aocx_ibf_w23_k19_h2_b64_kernels1_test_emu,AOCX_FILE>" "${CMAKE_CURRENT_BINARY_DIR}/ibf_w23_k19_h2_b64.aocx"

#         DEPENDS aocx_ibf_w19_k19_h2_b64_kernels1_test_emu
#         DEPENDS aocx_ibf_w23_k19_h2_b64_kernels1_test_emu

#         BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/ibf_w19_k19_h2_b64.aocx"
#         BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/ibf_w23_k19_h2_b64.aocx"
#     )

#     add_dependencies(minimizer_opencl_test minimizer_opencl_prepare_test)
# endif()

if (SEQAN3_FOUND AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_executable(minimizer_seqan3_test minimizer_seqan3_test.cpp)
    target_link_libraries(minimizer_seqan3_test PUBLIC min_ibf_fpga_test "seqan3::seqan3")
    add_test(NAME minimizer_seqan3_test COMMAND minimizer_seqan3_test)
endif()

# add_executable(minimizer_on_cpu_test minimizer_on_cpu_test.cpp)
# target_link_libraries(minimizer_on_cpu_test PUBLIC min_ibf_fpga_test)
# add_test(NAME minimizer_on_cpu_test COMMAND minimizer_on_cpu_test)