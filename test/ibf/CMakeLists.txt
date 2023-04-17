
find_path(SDSL_INCLUDE_DIR NAMES sdsl/bit_vectors.hpp HINTS "${SEQAN3_SUBMODULES_DIR}/submodules/sdsl-lite/include")
find_path(CEREAL_INCLUDE_DIR NAMES cereal/archives/binary.hpp HINTS "${SEQAN3_SUBMODULES_DIR}/submodules/cereal/include")

if (OPENCL_FOUND AND SDSL_INCLUDE_DIR AND CEREAL_INCLUDE_DIR)
    add_executable(ibf_opencl_test ibf_opencl_test.cpp)
    target_link_libraries(ibf_opencl_test PUBLIC min_ibf_fpga_test "OpenCL::OpenCL")
    target_include_directories(ibf_opencl_test PUBLIC ${SDSL_INCLUDE_DIR} ${CEREAL_INCLUDE_DIR})
    add_test(NAME ibf_opencl_test COMMAND ibf_opencl_test)
endif ()

if (SEQAN3_FOUND AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_executable(ibf_seqan3_test ibf_seqan3_test.cpp)
    target_link_libraries(ibf_seqan3_test PUBLIC min_ibf_fpga_test "seqan3::seqan3")
    add_test(NAME ibf_seqan3_test COMMAND ibf_seqan3_test)
endif()