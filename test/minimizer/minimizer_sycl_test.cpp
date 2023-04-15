

#include "fixtures/all.hpp"

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>

#include <ibf_fpga/test/assert_equal.hpp>
#include <ibf_fpga/sycl/kernel_minimizer.hpp>
#include <ibf_fpga/sycl/exception_handler.hpp>

void sycl_test(minimizer_test_fixture test)
{
    ibf::test::assert_equal(test.w, static_cast<size_t>(WINDOW_SIZE), "window size not supported");
    ibf::test::assert_equal(test.k, static_cast<size_t>(K), "k not supported");

#if defined(FPGA_EMULATOR)
    sycl::ext::intel::fpga_emulator_selector device_selector;
#else
    sycl::ext::intel::fpga_selector device_selector;
#endif
    sycl::queue q(device_selector, fpga_tools::exception_handler);

    std::vector<char> queries;
    for(char c : test.query)
        queries.push_back(c);

    std::vector<HostSizeType> querySizes;
    querySizes.push_back(test.query.size());

    size_t const queriesOffset = 0;
    size_t const querySizesOffset = 0;
    size_t const numberOfQueries = 1;
    std::vector<MinimizerToIBFData> pipe_results(test.query.size(), MinimizerToIBFData{});
    std::vector<uint64_t> minimizer{};

    { // device buffer scope
        sycl::buffer queries_buffer(queries);
        sycl::buffer querySizes_buffer(querySizes);
        sycl::buffer minimizerToIbf_buffer(pipe_results);

        RunMinimizerKernel(q,
                queries_buffer,
                queriesOffset,
                querySizes_buffer,
                querySizesOffset,
                numberOfQueries,
                minimizerToIbf_buffer);
    } // device buffer scope

    MinimizerToIBFData pipe_result{};
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