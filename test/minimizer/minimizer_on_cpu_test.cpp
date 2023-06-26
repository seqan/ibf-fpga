

#include "fixtures/all.hpp"

#include <min_ibf_fpga/backend_sycl/kernel_minimizer.hpp>
#include <min_ibf_fpga/backend_sycl/min_ibf_constants.hpp>

#include <min_ibf_fpga/test/assert_equal.hpp>
#include <min_ibf_fpga/test/min_ibf_test_types.hpp>

template <size_t windows_size, size_t kmer_size, size_t bin_count = 64>
void on_cpu_test(minimizer_test_fixture test)
{
    using constants = min_ibf_fpga::backend_sycl::min_ibf_constants<windows_size, kmer_size, bin_count>;
    using types = min_ibf_fpga::backend_sycl::min_ibf_types<constants, min_ibf_fpga::backend_sycl::cpu_test_backend>;
    using minimizer_kernel_t = min_ibf_fpga::backend_sycl::minimizer_kernel<constants, types>;

    using HostSizeType = typename types::HostSizeType;
    using MinimizerToIBFData = typename types::MinimizerToIBFData;

    min_ibf_fpga::test::assert_equal(test.w, static_cast<size_t>(constants::window_size), "window size not supported");
    min_ibf_fpga::test::assert_equal(test.k, static_cast<size_t>(constants::min_ibf_k), "k not supported");

    std::vector<uint64_t> minimizer{};

    minimizer_kernel_t::compute_minimizer(
        test.query.size(),
        [&](size_t const query_idx) {
            return test.query[query_idx];
        },
        [&, i = 0](MinimizerToIBFData const & pipe_result) mutable {
            bool const isLastElement = (++i) == (test.minimizer.size());
            min_ibf_fpga::test::assert_equal(pipe_result.isLastElement, isLastElement, "isLastElement(" + std::to_string(i) + ", " + std::to_string(test.minimizer.size()) + ") should only be true on last element.");
            minimizer.push_back(pipe_result.hash);
        }
    );

    test.compare_minimizer(minimizer);
}

int main()
{
    on_cpu_test<23, 19>(minimizer_w23_k19_query0_test);
    on_cpu_test<23, 19>(minimizer_w23_k19_query1_test);
    on_cpu_test<23, 19>(minimizer_w23_k19_query2_test);
    on_cpu_test<23, 19>(minimizer_w23_k19_query3_test);
    on_cpu_test<23, 19>(minimizer_w23_k19_query4_test);

    on_cpu_test<19, 19>(minimizer_w19_k19_query0_test);
    on_cpu_test<19, 19>(minimizer_w19_k19_query1_test);
    on_cpu_test<19, 19>(minimizer_w19_k19_query2_test);
    on_cpu_test<19, 19>(minimizer_w19_k19_query3_test);
    on_cpu_test<19, 19>(minimizer_w19_k19_query4_test);

    return 0;
}