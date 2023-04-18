#pragma once

#include <cstddef>

#include <min_ibf_fpga/threshold/kmer_lemma_threshold_table.hpp>

namespace min_ibf_fpga::test
{

// How many minimizer can a query of certain length for a given window_size and kmer_size produce?
// This gives an interval of the minimal and maximal number of such minimizer.
struct minimizer_range_t
{
    size_t minimal{};
    size_t maximal{};

    constexpr static minimizer_range_t of_query(size_t const query_length, size_t const window_size, size_t const kmer_size)
    {
        using min_ibf_fpga::threshold::kmer_count_of_query;

        auto int_div_ceil = [](size_t const nominator, size_t const denominator)
        {
            return (nominator + denominator - 1) / denominator;
        };

        size_t const kmers_per_window = kmer_count_of_query(window_size, kmer_size);
        size_t const kmers_per_pattern = kmer_count_of_query(query_length, kmer_size);

        return {
            // maximum distance between all minimizer
            .minimal = int_div_ceil(kmers_per_pattern, kmers_per_window),
            // every window position produces a minimizer
            .maximal = kmer_count_of_query(query_length, window_size)
        };
    }
};

} // namespace min_ibf_fpga::test