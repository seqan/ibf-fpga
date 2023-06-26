#pragma once

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>

#include <min_ibf_fpga/backend_sycl/min_ibf_constants.hpp>

namespace min_ibf_fpga::backend_sycl
{
template <typename constants_t, typename backend>
struct min_ibf_types;

struct sycl_backend {};

template <size_t ...sizes>
struct min_ibf_types<min_ibf_constants<sizes...>, sycl_backend>
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    using _constants = min_ibf_constants<sizes...>;

    // Host types
    typedef ulong HostHash;
    static_assert(sizeof(HostHash) == sizeof(uint64_t), "Host Hash type must be uint64_t compatible.");

    // Internal types
    typedef ushort Counter;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Asserts
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    static_assert(sizeof(HostHash) >= sizeof(_constants::minimizer_seed), "Minimizer seed doesn't fit Hash type");

    static_assert(_constants::window_size >= _constants::min_ibf_k, "Window size needs to be greater or equal K-mer size");

    static_assert(sizeof(HostHash) * 8 >= 2 * _constants::min_ibf_k, "K-mer doesn't fit Hash type");

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Types & Conversions
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    using HostSizeType = ac_int<_constants::host_size_type_bits, false>;

    using Element = ac_int<2, false>;

    using QueryIndex = ac_int<25, true>;

    using Chunk = ac_int<_constants::chunk_bits, false>;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Types & Conversions
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    using Hash = ac_int<2 * _constants::min_ibf_k, false>;

    using Minimizer = struct
    {
        Hash hash;
        unsigned char position;
    };

    using MinimizerToIBFData = struct //__attribute__((__packed__))
    {
        bool isLastElement;
        Hash hash;
    };
};

} // namespace min_ibf_fpga::backend_sycl