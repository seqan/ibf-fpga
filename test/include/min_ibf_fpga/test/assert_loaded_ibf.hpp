#pragma once

#include <iostream>
#include <sstream>

namespace min_ibf_fpga::test
{

struct ibf_meta_assertion_t
{
    size_t bin_count{};
    size_t bin_size{};

    // pretty constant default
    size_t hash_function_count{2};

    // computed values
    size_t bit_size{bin_count * bin_size};
};

// ibf_meta_t can be seqan3::interleaved_bloom_filter<> or min_ibf_fpga::ibf_metadata
template <typename ibf_meta_t>
void assert_loaded_ibf(ibf_meta_t const & ibf_meta, ibf_meta_assertion_t const & ibf_meta_assertion)
{
    min_ibf_fpga::test::assert_equal(ibf_meta_assertion.bin_count, ibf_meta.bin_count(), "bin counts match");
    min_ibf_fpga::test::assert_equal(ibf_meta_assertion.bin_size, ibf_meta.bin_size(), "bin sizes match");
    min_ibf_fpga::test::assert_equal(ibf_meta_assertion.hash_function_count, ibf_meta.hash_function_count(), "hash function counts match");
    min_ibf_fpga::test::assert_equal(ibf_meta_assertion.bit_size, ibf_meta.bit_size(), "bit sizes match");
};

} // namespace min_ibf_fpga::test
