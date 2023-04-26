

#include "fixtures/all.hpp"

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>

#include <min_ibf_fpga/test/assert_equal.hpp>
#include <min_ibf_fpga/backend_sycl/kernel_minimizer.hpp>
#include <min_ibf_fpga/backend_sycl/exception_handler.hpp>

void sycl_test(minimizer_test_fixture test)
{
    min_ibf_fpga::test::assert_equal(test.w, static_cast<size_t>(WINDOW_SIZE), "window size not supported");
    min_ibf_fpga::test::assert_equal(test.k, static_cast<size_t>(MIN_IBF_K), "k not supported");

#if FPGA_SIMULATOR
  auto device_selector = sycl::ext::intel::fpga_simulator_selector_v;
#elif FPGA_HARDWARE
  auto device_selector = sycl::ext::intel::fpga_selector_v;
#else  // #if FPGA_EMULATOR
  auto device_selector = sycl::ext::intel::fpga_emulator_selector_v;
#endif

    sycl::queue q(device_selector, fpga_tools::exception_handler);

    std::vector<char> queries;
    for(char c : test.query)
        queries.push_back(c);

    std::vector<min_ibf_fpga::backend_sycl::HostSizeType> querySizes;
    querySizes.push_back(test.query.size());

    size_t const queriesOffset = 0;
    size_t const querySizesOffset = 0;
    size_t const numberOfQueries = 1;
    std::vector<min_ibf_fpga::backend_sycl::MinimizerToIBFData> pipe_results(test.query.size(), min_ibf_fpga::backend_sycl::MinimizerToIBFData{});
    std::vector<uint64_t> minimizer{};

    { // device buffer scope
        sycl::buffer queries_buffer(queries);
        sycl::buffer querySizes_buffer(querySizes);
        sycl::buffer minimizerToIbf_buffer(pipe_results);

        min_ibf_fpga::backend_sycl::RunMinimizerKernel(q,
                queries_buffer,
                queriesOffset,
                querySizes_buffer,
                querySizesOffset,
                numberOfQueries,
                minimizerToIbf_buffer);
    } // device buffer scope

    min_ibf_fpga::backend_sycl::MinimizerToIBFData pipe_result{};
    for (int i = 0; i < pipe_results.size() && !pipe_result.isLastElement; ++i)
    {
        pipe_result = pipe_results[i];
        minimizer.push_back(pipe_result.hash);
    }

    test.compare_minimizer(minimizer);
}

int main()
{
    sycl_test(minimizer_w23_k19_query0_test);
    sycl_test(minimizer_w23_k19_query1_test);
    sycl_test(minimizer_w23_k19_query2_test);
    sycl_test(minimizer_w23_k19_query3_test);
    sycl_test(minimizer_w23_k19_query4_test);

    // not supported yet
    // sycl_test(minimizer_w19_k19_query0_test);
    // sycl_test(minimizer_w19_k19_query1_test);
    // sycl_test(minimizer_w19_k19_query2_test);
    // sycl_test(minimizer_w19_k19_query3_test);
    // sycl_test(minimizer_w19_k19_query4_test);

    return 0;
}