#pragma once

#include <min_ibf_fpga/threshold/threshold_table.hpp>

namespace min_ibf_fpga::threshold
{

constexpr size_t kmer_count_of_query(size_t const query_length, size_t const kmer_size)
{
    return query_length - kmer_size + 1;
}

constexpr size_t kmer_lemma_threshold(size_t const kmer_count, size_t const kmer_size, size_t const error)
{
    size_t const number_of_destroyed_kmer = kmer_size * error;
    size_t const minimal_number_of_intact_kmer = kmer_count - number_of_destroyed_kmer;

    return minimal_number_of_intact_kmer;
}

struct kmer_lemma_threshold_table : public threshold_table
{
    kmer_lemma_threshold_table(size_t const query_length, size_t const kmer_size, size_t const error) :
        threshold_table{}
    {
        size_t const kmer_count = kmer_count_of_query(query_length, kmer_size);

        this->table = std::vector<size_t>(
                kmer_count,
                kmer_lemma_threshold(kmer_count, kmer_size, error) - 2
            );
    }
};

} // namespace min_ibf_fpga::threshold