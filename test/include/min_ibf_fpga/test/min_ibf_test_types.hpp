#pragma once


#include <min_ibf_fpga/backend_sycl/min_ibf_constants.hpp>

namespace min_ibf_fpga::backend_sycl
{
template <typename constants_t, typename backend>
struct min_ibf_types;

struct cpu_test_backend {};

template <size_t ...sizes>
struct min_ibf_types<min_ibf_constants<sizes...>, cpu_test_backend>
{
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    using _constants = min_ibf_constants<sizes...>;

    // Host types
    // typedef ulong HostHash;
    using HostHash = ulong;
    // static_assert(sizeof(HostHash) == sizeof(uint64_t), "Host Hash type must be uint64_t compatible.");

    // Internal types
    // typedef ushort Counter;
    using Counter = ushort;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Asserts
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    // static_assert(sizeof(HostHash) >= sizeof(_constants::minimizer_seed), "Minimizer seed doesn't fit Hash type");

    // static_assert(_constants::window_size >= _constants::min_ibf_k, "Window size needs to be greater or equal K-mer size");

    // static_assert(sizeof(HostHash) * 8 >= 2 * _constants::min_ibf_k, "K-mer doesn't fit Hash type");

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Types & Conversions
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    // using HostSizeType = ac_int<_constants::host_size_type_bits, false>;
    using HostSizeType = uint64_t;

    // using DoubleHostSizeType = ac_int<_constants::host_size_type_bits * 2, false>;
    using DoubleHostSizeType = unsigned __int128;

    // using Element = ac_int<2, false>;
    struct Element
    {
        Element() = delete;
        Element(uint8_t const value) :
            _value{value}
        {
            _assert_value_in_bounds();
        }

        Element operator~() const
        {
            uint8_t new_value = (~_value) & 0b11;
            return Element{new_value};
        }

        operator uint64_t() const
        {
            _assert_value_in_bounds();
            return _value;
        }

    private:
        void _assert_value_in_bounds() const
        {
            if (_value >= uint8_t{4})
                throw std::runtime_error("Element MUST be two bit and isn't allowed to exceed that.");
        }

        uint8_t _value;
    };

    // using QueryIndex = ac_int<25, true>;
    using QueryIndex = unsigned;

    // using Chunk = ac_int<_constants::chunk_bits, false>;
    // using Chunk = uint64_t;
    struct Chunk
    {
        explicit Chunk(uint64_t const value) : _value{value}
        {}

        struct reference
        {
            uint64_t * value_ptr{nullptr};
            size_t bit_position{0};

            reference & operator=(bool bit)
            {
                uint64_t const mask = (uint64_t{1} << bit_position);

                if (bit)
                    *value_ptr |= mask;
                else
                    *value_ptr &= ~mask;

                return *this;
            }

            operator uint64_t() const
            {
                uint64_t const mask_bit_one = (uint64_t{1} << bit_position);

                return (*value_ptr & mask_bit_one) != 0;
            }
        };

        operator uint64_t() const
        {
            return _value;
        }

        reference operator[](size_t const idx) &
        {
            return reference{&_value, idx};
        }

        Chunk operator~() const
        {
            return Chunk{~_value};
        }

        Chunk & operator&=(Chunk const & other)
        {
            _value &= other._value;
            return *this;
        }

    private:
        uint64_t _value{};
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Types & Conversions
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    // using Hash = ac_int<2 * _constants::min_ibf_k, false>;
    using Hash = uint64_t;

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