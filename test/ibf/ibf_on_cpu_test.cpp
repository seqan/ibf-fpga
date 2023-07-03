#define MIN_IBF_FPGA_INIT_EVERYTHING 1

#include "fixtures/all.hpp"

#include <min_ibf_fpga/backend_sycl/kernel_ibf.hpp>
#include <min_ibf_fpga/backend_sycl/min_ibf_constants.hpp>

#include <min_ibf_fpga/index/ibf_metadata.hpp>
#include <min_ibf_fpga/index/ibf_data.hpp>

#include <min_ibf_fpga/test/assert_equal.hpp>
#include <min_ibf_fpga/test/assert_loaded_ibf.hpp>
#include <min_ibf_fpga/test/load_ibf_index.hpp>
#include <min_ibf_fpga/test/min_ibf_test_types.hpp>


template <size_t windows_size, size_t kmer_size, size_t bin_count = 64>
void on_cpu_test(min_ibf_fpga::index::ibf_metadata const & ibf_meta, min_ibf_fpga::index::ibf_data const & ibf, ibf_test_fixture const & test)
{
    using constants = min_ibf_fpga::backend_sycl::min_ibf_constants<windows_size, kmer_size, bin_count>;
    using types = min_ibf_fpga::backend_sycl::min_ibf_types<constants, min_ibf_fpga::backend_sycl::cpu_test_backend>;
    using ibf_kernel_t = min_ibf_fpga::backend_sycl::ibf_kernel<constants, types>;

    using MinimizerToIBFData = typename types::MinimizerToIBFData;
    using Hash = typename types::Hash;
    using Chunk = typename types::Chunk;
    using HostSizeType = typename types::HostSizeType;
    using QueryIndex = typename types::QueryIndex;
    using Counter = typename types::Counter;

    std::vector<uint64_t> minimizer = test.minimizer();

    std::vector<Chunk> ibfData;
    for (size_t i = 0; i < ibf.size(); i++)
    {
        ibfData.push_back(Chunk{ibf.data()[i]});
    }

    // std::array<size_t, 64> counter_actual{};
    std::vector<std::bitset<64>> bin_bit_membership_per_error{};
    for (size_t error = 0; error < test.threshold_per_error.size(); ++error)
    {
        std::vector<std::tuple<size_t, std::bitset<64>>> minimizer_bit_membership_actual{};
        size_t const numberOfQueries = 1;
        std::vector<Chunk> result(numberOfQueries, Chunk{0});

        ibf_kernel_t::compute_ibf(
            ibfData.data(),
            ibf_meta.bin_size(),
            ibf_meta.hash_shift(),
            [&](const QueryIndex localNumberOfHashes){
                size_t const minimalNumberOfMinimizers = test.minimizer_range.minimal;
                size_t const maximalNumberOfMinimizers = test.minimizer_range.maximal;

                //threshold = getThreshold(localNumberOfHashes, minimalNumberOfMinimizers, maximalNumberOfMinimizers, thresholds);
                // manual inline of getThreshold() because of thresholds accessor
                const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

                HostSizeType index = localNumberOfHashes < minimalNumberOfMinimizers? 0 : localNumberOfHashes - minimalNumberOfMinimizers;
                index = index < maximalIndex? index : maximalIndex;

                Counter threshold = test.threshold_table_per_error[error].table[static_cast<size_t>(index)] + 2;
                return threshold;
            }, [&, i = 0]() mutable -> MinimizerToIBFData {
                return MinimizerToIBFData {
                    .isLastElement{i + 1 == minimizer.size()},
                    .hash{minimizer[i++]}
                };
            }, [&](size_t const chunkIndex, Chunk const & localResult) {
                size_t const queryIndex = 0;
                size_t const result_idx = queryIndex * constants::chunks + chunkIndex;
                result[result_idx] = localResult;
            }, [&](unsigned char const chunk_idx, Hash const & minimizer, Chunk const & minimizer_membership) {
                min_ibf_fpga::test::assert_equal(chunk_idx, (unsigned char){0}, "TODO: test more than one chunk.");

                std::bitset<64> bit_membership{minimizer_membership};
                minimizer_bit_membership_actual.push_back({minimizer, bit_membership});
            }
        );

        // TODO: this needs adjustment if bin_count > 64
        if (ibf_meta.bin_count() > 64) throw std::runtime_error{"NEEDS TO BE FIXED!!!!"};

        std::bitset<64> bin_bit_membership{(uint64_t)result[0]};

        bin_bit_membership_per_error.push_back(bin_bit_membership);

        // minimizer_bit_membership is independent on error; only the threshold changes the bin_bit_membership
        test.compare_minimizer_membership(minimizer_bit_membership_actual);
    }

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

    on_cpu_test<23, 19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query0_test);
    on_cpu_test<23, 19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query1_test);
    on_cpu_test<23, 19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query2_test);
    on_cpu_test<23, 19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query3_test);
    on_cpu_test<23, 19>(ibf_w23_k19_meta, ibf_w23_k19, ibf_w23_k19_query4_test);

    on_cpu_test<19, 19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query0_test);
    on_cpu_test<19, 19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query1_test);
    on_cpu_test<19, 19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query2_test);
    on_cpu_test<19, 19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query3_test);
    on_cpu_test<19, 19>(ibf_w19_k19_meta, ibf_w19_k19, ibf_w19_k19_query4_test);

    return 0;
}