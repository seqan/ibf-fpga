

#include "fixtures/all.hpp"

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>
#include <min_ibf_fpga/backend_sycl/pipe_utils.hpp>

#include <min_ibf_fpga/backend_sycl/exception_handler.hpp>
#include <min_ibf_fpga/index/ibf_metadata.hpp>
#include <min_ibf_fpga/index/ibf_data.hpp>

#include <min_ibf_fpga/test/assert_equal.hpp>
#include <min_ibf_fpga/test/assert_loaded_ibf.hpp>
#include <min_ibf_fpga/test/load_ibf_index.hpp>

#include "RunIBFKernel.hpp"

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
struct IbfKernel_w23_k19
{
    using _constants = min_ibf_fpga::backend_sycl::min_ibf_constants<23, 19, 64>;
    using _backend = min_ibf_fpga::backend_sycl::sycl_backend;
    using _types = min_ibf_fpga::backend_sycl::min_ibf_types<_constants, _backend>;
    struct _pipe_name{}; // this creates the unique pipe name `IbfKernel_w23_k19::_pipe_name`
    using _MinimizerToIBFPipes = fpga_tools::PipeArray<_pipe_name, _types::MinimizerToIBFData, 25, _constants::number_of_kernels>;

    using type = min_ibf_fpga::backend_sycl::sycl_ibf_kernel<_MinimizerToIBFPipes, _constants, _types>;
};
class HostToKernelPipe_w23_k19;

struct IbfKernel_w19_k19
{
    using _constants = min_ibf_fpga::backend_sycl::min_ibf_constants<19, 19, 64>;
    using _backend = min_ibf_fpga::backend_sycl::sycl_backend;
    using _types = min_ibf_fpga::backend_sycl::min_ibf_types<_constants, _backend>;
    struct _pipe_name{}; // this creates the unique pipe name `IbfKernel_w19_k19::_pipe_name`
    using _MinimizerToIBFPipes = fpga_tools::PipeArray<_pipe_name, _types::MinimizerToIBFData, 25, _constants::number_of_kernels>;

    using type = min_ibf_fpga::backend_sycl::sycl_ibf_kernel<_MinimizerToIBFPipes, _constants, _types>;
};
class HostToKernelPipe_w19_k19;

template <typename IbfKernel, typename HostToKernelPipe>
void sycl_test(min_ibf_fpga::index::ibf_metadata const & ibf_meta, min_ibf_fpga::index::ibf_data const & ibf, ibf_test_fixture const & test)
{
    using MinimizerToIBFData = typename IbfKernel::_types::MinimizerToIBFData;
    using Chunk = typename IbfKernel::_types::Chunk;
    using HostSizeType = typename IbfKernel::_types::HostSizeType;

#if FPGA_SIMULATOR
  auto device_selector = sycl::ext::intel::fpga_simulator_selector_v;
#elif FPGA_HARDWARE
  auto device_selector = sycl::ext::intel::fpga_selector_v;
#else  // #if FPGA_EMULATOR
  auto device_selector = sycl::ext::intel::fpga_emulator_selector_v;
#endif

    sycl::queue q(device_selector, fpga_tools::exception_handler);

    std::vector<uint64_t> minimizer = test.minimizer();
    std::vector<MinimizerToIBFData> minimizerToIbf;
    for (size_t i = 0; i < minimizer.size(); i++)
    {
        MinimizerToIBFData data;
        data.hash = minimizer[i];
        data.isLastElement = false;
        if(i + 1 == minimizer.size())
        {
            data.isLastElement = true;
        }
        minimizerToIbf.push_back(data);
    }

    std::vector<Chunk> ibfData;
    for (size_t i = 0; i < ibf.size(); i++)
    {
        ibfData.push_back(ibf.data()[i]);
    }

    auto compute_bin_containment_from_minimizer = [&](size_t const errors) -> std::bitset<64>
    {
        size_t const binSize = ibf_meta.bin_size();
        size_t const hashShift = ibf_meta.hash_shift();
        size_t const numberOfQueries = 1;
        size_t const minimalNumberOfMinimizers = test.minimizer_range.minimal;
        size_t const maximalNumberOfMinimizers = test.minimizer_range.maximal;
        min_ibf_fpga::threshold::threshold_table ths = test.threshold_table_per_error[errors];
        std::vector<HostSizeType> thresholds;
        for (size_t i = 0; i < ths.table.size(); i++)
        {
            thresholds.push_back(ths.table[i]);
        }
        std::vector<Chunk> result(numberOfQueries, 0);

        { // device buffer scope
            auto ibfData_device_ptr = sycl::malloc_device<Chunk>(ibfData.size(), q);
            static_assert(std::is_same_v<decltype(ibfData_device_ptr), Chunk *>);
            q.memcpy(ibfData_device_ptr, ibfData.data(), ibfData.size() * sizeof(Chunk));

            auto thresholds_device_ptr = sycl::malloc_device<HostSizeType>(thresholds.size(), q);
            static_assert(std::is_same_v<decltype(thresholds_device_ptr), HostSizeType *>);
            q.memcpy(thresholds_device_ptr, thresholds.data(), thresholds.size() * sizeof(HostSizeType));

            auto result_device_ptr = sycl::malloc_device<Chunk>(result.size(), q);
            static_assert(std::is_same_v<decltype(result_device_ptr), Chunk *>);

            sycl::buffer minimizerToIbf_buffer(minimizerToIbf);

            q.wait();

            min_ibf_fpga::backend_sycl::RunIBFKernel<IbfKernel, HostToKernelPipe>(q,
                    minimizerToIbf_buffer,
                    ibfData_device_ptr,
                    binSize,
                    hashShift,
                    numberOfQueries,
                    minimalNumberOfMinimizers,
                    maximalNumberOfMinimizers,
                    thresholds_device_ptr,
                    result_device_ptr);

            q.wait();

            q.memcpy(result.data(), result_device_ptr, result.size() * sizeof(Chunk));

            q.wait();

            sycl::free(ibfData_device_ptr, q);
            sycl::free(thresholds_device_ptr, q);
            sycl::free(result_device_ptr, q);
        } // device buffer scope

        // TODO: this needs adjustment if bin_count > 64
        if (ibf_meta.bin_count() > 64) throw std::runtime_error{"NEEDS TO BE FIXED!!!!"};

        std::bitset<64> bin_bit_membership{result[0]};
        return bin_bit_membership;
    };

    // std::map<size_t, std::bitset<64>> minimizer_bit_membership_actual{};
    // std::array<size_t, 64> counter_actual{};
    std::vector<std::bitset<64>> bin_bit_membership_per_error{};
    for (size_t error = 0; error < test.threshold_per_error.size(); ++error)
    {
        std::bitset<64> bin_bit_membership = compute_bin_containment_from_minimizer(error);

        bin_bit_membership_per_error.push_back(bin_bit_membership);
        std::cout << "bin_bit_membership: " << bin_bit_membership << std::endl;
    }

    // test.compare_minimizer_membership(minimizer_bit_membership_actual);
    // test.compare_counter(counter_actual);
    test.compare_bin_bit_membership(bin_bit_membership_per_error);
}

int main()
{
    // raptor_23_19.index
    size_t const bin_count{64};
    size_t const bin_size{1024};

    auto [ibf_w23_k19_meta, ibf_w23_k19] = min_ibf_fpga::test::load_ibf_index("raptor_23_19.index");
    auto [ibf_w19_k19_meta, ibf_w19_k19] = min_ibf_fpga::test::load_ibf_index("raptor_19_19.index");

    min_ibf_fpga::test::assert_loaded_ibf(ibf_w23_k19_meta, min_ibf_fpga::test::ibf_meta_assertion_t {
        .bin_count = bin_count,
        .bin_size = bin_size
    });

    min_ibf_fpga::test::assert_loaded_ibf(ibf_w19_k19_meta, min_ibf_fpga::test::ibf_meta_assertion_t {
        .bin_count = bin_count,
        .bin_size = bin_size
    });

    sycl_test<IbfKernel_w23_k19, HostToKernelPipe_w23_k19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query0_test);
    sycl_test<IbfKernel_w23_k19, HostToKernelPipe_w23_k19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query1_test);
    sycl_test<IbfKernel_w23_k19, HostToKernelPipe_w23_k19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query2_test);
    sycl_test<IbfKernel_w23_k19, HostToKernelPipe_w23_k19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query3_test);
    sycl_test<IbfKernel_w23_k19, HostToKernelPipe_w23_k19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query4_test);

    // not supported yet
    sycl_test<IbfKernel_w19_k19, HostToKernelPipe_w19_k19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query0_test);
    sycl_test<IbfKernel_w19_k19, HostToKernelPipe_w19_k19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query1_test);
    sycl_test<IbfKernel_w19_k19, HostToKernelPipe_w19_k19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query2_test);
    sycl_test<IbfKernel_w19_k19, HostToKernelPipe_w19_k19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query3_test);
    sycl_test<IbfKernel_w19_k19, HostToKernelPipe_w19_k19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query4_test);

    return 0;
}