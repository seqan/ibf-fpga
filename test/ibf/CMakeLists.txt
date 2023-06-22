
find_path(SDSL_INCLUDE_DIR NAMES sdsl/bit_vectors.hpp HINTS "${SEQAN3_ROOT_DIR}/submodules/sdsl-lite/include" "${CMAKE_CURRENT_SOURCE_DIR}/../../external/sdsl-lite/include")
find_path(CEREAL_INCLUDE_DIR NAMES cereal/archives/binary.hpp HINTS "${SEQAN3_ROOT_DIR}/submodules/cereal/include" "${CMAKE_CURRENT_SOURCE_DIR}/../../external/cereal/include")

if (CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM" AND SDSL_INCLUDE_DIR AND CEREAL_INCLUDE_DIR)
    add_executable(ibf_sycl_test ibf_sycl_test.cpp)
    target_link_libraries(ibf_sycl_test PUBLIC min_ibf_fpga_test)
    target_include_directories(ibf_sycl_test PUBLIC ${SDSL_INCLUDE_DIR} ${CEREAL_INCLUDE_DIR})
    set_target_properties(ibf_sycl_test PROPERTIES COMPILE_FLAGS "-Wall -fsycl -fintelfpga -qactypes -DFPGA_EMULATOR")
    set_target_properties(ibf_sycl_test PROPERTIES LINK_FLAGS "-fsycl -fintelfpga -qactypes")
    add_test(NAME ibf_sycl_test COMMAND ibf_sycl_test)
endif ()

if (OPENCL_FOUND AND SDSL_INCLUDE_DIR AND CEREAL_INCLUDE_DIR)
    add_executable(ibf_opencl_test ibf_opencl_test.cpp)
    target_link_libraries(ibf_opencl_test PUBLIC min_ibf_fpga_test "OpenCL::OpenCL")
    target_include_directories(ibf_opencl_test PUBLIC ${SDSL_INCLUDE_DIR} ${CEREAL_INCLUDE_DIR})
    add_test(NAME ibf_opencl_test COMMAND ibf_opencl_test)

    add_custom_target(ibf_opencl_prepare_test
        COMMAND ${CMAKE_COMMAND} -E create_symlink "$<TARGET_PROPERTY:aocx_ibf_w19_k19_h2_b64_kernels1_test_emu,AOCX_FILE>" "${CMAKE_CURRENT_BINARY_DIR}/ibf_w19_k19_h2_b64.aocx"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "$<TARGET_PROPERTY:aocx_ibf_w23_k19_h2_b64_kernels1_test_emu,AOCX_FILE>" "${CMAKE_CURRENT_BINARY_DIR}/ibf_w23_k19_h2_b64.aocx"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_LIST_DIR}/../data/raptor_19_19.index" "${CMAKE_CURRENT_BINARY_DIR}/raptor_19_19.index"
        COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_LIST_DIR}/../data/raptor_23_19.index" "${CMAKE_CURRENT_BINARY_DIR}/raptor_23_19.index"

        DEPENDS aocx_ibf_w19_k19_h2_b64_kernels1_test_emu
        DEPENDS aocx_ibf_w23_k19_h2_b64_kernels1_test_emu

        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/ibf_w19_k19_h2_b64.aocx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/ibf_w23_k19_h2_b64.aocx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/raptor_19_19.index"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/raptor_23_19.index"
    )

    add_dependencies(ibf_opencl_test ibf_opencl_prepare_test)

endif ()

if (SEQAN3_FOUND AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_executable(ibf_seqan3_test ibf_seqan3_test.cpp)
    target_link_libraries(ibf_seqan3_test PUBLIC min_ibf_fpga_test "seqan3::seqan3")
    add_test(NAME ibf_seqan3_test COMMAND ibf_seqan3_test)
endif()

set(INDEX_LIST "raptor_19_19;raptor_23_19")
foreach(INDEX ${INDEX_LIST})
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/../data/${INDEX}.index ${CMAKE_CURRENT_BINARY_DIR}/${INDEX}.index)
endforeach()
