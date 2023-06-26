#pragma once

namespace min_ibf_fpga::backend_sycl
{
template <size_t _window_size, size_t _kmer_size, size_t _bin_count, size_t _hash_count = 2, size_t _number_of_kernels = 1>
struct min_ibf_constants
{
    template <class lhs_t, class rhs_t>
    static constexpr auto integer_division_ceil(const lhs_t lhs, const rhs_t rhs)
    {
        return (lhs + rhs - 1) / rhs;
    }

    static_assert(_kmer_size > 0u, "k-mer can't be of zero length.");
    static_assert(_kmer_size < 32u, "k-mer can't exceed 32 due to program limitations.");
    static_assert(_window_size >= _kmer_size, "Window size must be at least as big as the k-mer.");

    static constexpr auto window_size = _window_size;
    static constexpr auto min_ibf_k = _kmer_size; // TODO rename
    static constexpr auto bin_count = _bin_count;
    static constexpr auto hash_count = _hash_count;
    static constexpr auto number_of_kernels = _number_of_kernels;

    static constexpr auto number_of_kmers_per_window = window_size - min_ibf_k + 1;

    static constexpr auto initialization_iterations = (min_ibf_k - 1) + (number_of_kmers_per_window - 1);

    static constexpr uint64_t minimizer_seed = 0x8f3f73b5cf1c9ade;
    static constexpr uint64_t minimizer_seed_adjusted = minimizer_seed >> (64 - 2 * min_ibf_k);

    static constexpr auto host_size_type_bits = 64;
    static constexpr auto technical_bin_count_words = integer_division_ceil(bin_count, host_size_type_bits);
    static constexpr auto technical_bin_count = technical_bin_count_words * host_size_type_bits;

    static constexpr auto max_bus_width = 512;
    static constexpr auto chunk_bits = technical_bin_count > max_bus_width? max_bus_width : technical_bin_count;
    static constexpr auto chunks = technical_bin_count > max_bus_width? integer_division_ceil(technical_bin_count, max_bus_width) : 1;
    static constexpr auto chunks_per_bin = integer_division_ceil(technical_bin_count, max_bus_width);

    static_assert((bool)(technical_bin_count < max_bus_width) || (technical_bin_count % max_bus_width == 0), "Bin count needs to be less or a multiple of max bus width");

    static constexpr uint64_t seeds[5] = {
        13572355802537770549u, // 2**64 / (e/2)
        13043817825332782213u, // 2**64 / sqrt(2)
        10650232656628343401u, // 2**64 / sqrt(3)
        16499269484942379435u, // 2**64 / (sqrt(5)/2)
        4893150838803335377u // 2**64 / (3*pi/5)
    };
    static_assert(sizeof(seeds) / sizeof(uint64_t) >= hash_count, "The number of hash functions must be smaller or equal the number of seeds");
};
} // namespace min_ibf_fpga::backend_sycl