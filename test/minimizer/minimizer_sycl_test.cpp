#include "fixtures/all.hpp"

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>

#include <min_ibf_fpga/test/assert_equal.hpp>
#include <min_ibf_fpga/backend_sycl/kernel_minimizer_test.cpp>
#include <min_ibf_fpga/backend_sycl/exception_handler.hpp>

void sycl_test(minimizer_test_fixture test)
{
    using HostSizeType = min_ibf_fpga::backend_sycl::HostSizeType;
    using MinimizerToIBFData = min_ibf_fpga::backend_sycl::MinimizerToIBFData;

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

    std::vector<HostSizeType> querySizes;
    querySizes.push_back(test.query.size());

    size_t const queriesOffset = 0;
    size_t const querySizesOffset = 0;
    size_t const numberOfQueries = 1;
    std::vector<MinimizerToIBFData> pipe_results(test.query.size(), MinimizerToIBFData{});
    std::vector<uint64_t> minimizer{};

    { // device buffer scope
        auto queries_device_ptr = sycl::malloc_device<char>(queries.size(), q);
        static_assert(std::is_same_v<decltype(queries_device_ptr), char *>);
        q.memcpy(queries_device_ptr, queries.data(), queries.size() * sizeof(char));

        auto querySizes_device_ptr = sycl::malloc_device<HostSizeType>(querySizes.size(), q);
        static_assert(std::is_same_v<decltype(querySizes_device_ptr), HostSizeType *>);
        q.memcpy(querySizes_device_ptr, querySizes.data(), querySizes.size() * sizeof(HostSizeType));

        sycl::buffer minimizerToIbf_buffer(pipe_results);

        q.wait();

        min_ibf_fpga::backend_sycl::RunMinimizerKernel(q,
                queries_device_ptr,
                queriesOffset,
                querySizes_device_ptr,
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