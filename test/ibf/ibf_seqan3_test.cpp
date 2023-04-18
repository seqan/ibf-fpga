
#include "fixtures/all.hpp"

#include <seqan3/alphabet/nucleotide/dna4.hpp>
#include <seqan3/range/views/char_to.hpp>
#include <seqan3/range/views/minimiser_hash.hpp>
#include <seqan3/range/views/to.hpp>

#include <seqan3/search/dream_index/interleaved_bloom_filter.hpp>

#include <min_ibf_fpga/test/assert_loaded_ibf.hpp>

#include <filesystem>

void seqan3_test(seqan3::interleaved_bloom_filter<> const & ibf, ibf_test_fixture const & test)
{
    auto membership_agent = ibf.membership_agent();

    std::vector<std::tuple<size_t, std::bitset<64>>> minimizer_bit_membership_actual{};
    std::array<size_t, 64> counter_actual{};
    std::vector<std::bitset<64>> bin_bit_membership_per_error{};

    for (auto [hash, _] : test.minimizer_bit_membership)
    {
        auto const & membership = membership_agent.bulk_contains(hash);
        std::bitset<64> bit_membership{};
        for (size_t i = 0; i < membership.size(); ++i)
        {
            bit_membership[i] = membership[i];

            if (membership[i])
                ++counter_actual[i];
        }
        minimizer_bit_membership_actual.push_back({hash, bit_membership});
    }

    for (size_t error = 0; error < test.threshold_per_error.size(); ++error)
    {
        size_t const threshold = test.threshold_per_error[error];

        std::bitset<64> bin_bit_membership{};
        for (size_t i = 0; i < counter_actual.size(); ++i)
        {
            if (counter_actual[i] >= threshold)
                bin_bit_membership[i] = true;
        }

        bin_bit_membership_per_error.push_back(bin_bit_membership);
    }

    test.compare_minimizer_membership(minimizer_bit_membership_actual);
    test.compare_counter(counter_actual);
    test.compare_bin_bit_membership(bin_bit_membership_per_error);
}

int main()
{
    // raptor_23_19.index
    size_t const bin_count{64};
    size_t const bin_size{1024};

    auto load_ibf = [](std::filesystem::path const ibf_path)
    {
        seqan3::interleaved_bloom_filter<> ibf{};

        std::ifstream archiveStream{ibf_path, std::ios::binary};
        cereal::BinaryInputArchive archive{archiveStream};
        archive(ibf);

        return ibf;
    };

    seqan3::interleaved_bloom_filter<> ibf_w23_k19 = load_ibf("raptor_23_19.index");
    seqan3::interleaved_bloom_filter<> ibf_w19_k19 = load_ibf("raptor_19_19.index");

    min_ibf_fpga::test::assert_loaded_ibf(ibf_w23_k19, min_ibf_fpga::test::ibf_meta_assertion_t{
        .bin_count = bin_count,
        .bin_size = bin_size
    });

    min_ibf_fpga::test::assert_loaded_ibf(ibf_w19_k19, min_ibf_fpga::test::ibf_meta_assertion_t{
        .bin_count = bin_count,
        .bin_size = bin_size
    });

    seqan3_test(ibf_w23_k19, ibf_w23_k19_query0_test);
    seqan3_test(ibf_w23_k19, ibf_w23_k19_query1_test);
    seqan3_test(ibf_w23_k19, ibf_w23_k19_query2_test);
    seqan3_test(ibf_w23_k19, ibf_w23_k19_query3_test);
    seqan3_test(ibf_w23_k19, ibf_w23_k19_query4_test);

    seqan3_test(ibf_w19_k19, ibf_w19_k19_query0_test);
    seqan3_test(ibf_w19_k19, ibf_w19_k19_query1_test);
    seqan3_test(ibf_w19_k19, ibf_w19_k19_query2_test);
    seqan3_test(ibf_w19_k19, ibf_w19_k19_query3_test);
    seqan3_test(ibf_w19_k19, ibf_w19_k19_query4_test);

    return 0;
}