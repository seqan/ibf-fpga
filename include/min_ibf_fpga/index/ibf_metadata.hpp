#pragma once

#include <cstddef>

namespace min_ibf_fpga::index
{

struct ibf_metadata
{
    size_t _bin_count{};
    size_t _technical_bin_count{};
    size_t _bin_size{};
    size_t _hash_shift{};
    size_t _bin_words{};
    size_t _hash_function_count{};

    size_t bin_count() const { return _bin_count; }
    size_t technical_bin_count() const { return _technical_bin_count; }
    size_t bin_size() const { return _bin_size; }
    size_t hash_shift() const { return _hash_shift; }
    size_t bin_words() const { return _bin_words; }
    size_t hash_function_count() const { return _hash_function_count; }
    size_t bit_size() const { return _technical_bin_count * _bin_size; }

    template <typename archive_t>
    void serialize(archive_t & archive)
    {
        archive(_bin_count);
        archive(_technical_bin_count);
        archive(_bin_size);
        archive(_hash_shift);
        archive(_bin_words);
        archive(_hash_function_count);
    }
};

} // namespace min_ibf_fpga::index