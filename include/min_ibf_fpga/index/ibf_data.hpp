#pragma once

#include <cstddef>

#include <sdsl/bit_vectors.hpp>

namespace min_ibf_fpga::index
{

struct ibf_data
{
    sdsl::bit_vector _data{};

    size_t const * data() const
    {
        return _data.data();
    }

    size_t size() const
    {
        return (_data.bit_size() + 63) / 64;
    }

    template <typename archive_t>
    void serialize(archive_t & archive)
    {
        // TODO: don't use sdsl at all, below is data definition taken from sdsl (needs to be verified if this is really the use deserialization code)
        // https://github.com/xxsds/sdsl-lite/blob/9a0d5676fd09fb8b52af214eca2d5809c9a32dbe/include/sdsl/int_vector.hpp#L1863-L1873
        // ar(CEREAL_NVP(cereal::make_size_tag(m_width)));
        // ar(CEREAL_NVP(growth_factor));
        // ar(CEREAL_NVP(cereal::make_size_tag(m_size)));
        // resize(size());
        // ar(cereal::make_nvp("data", cereal::binary_data(m_data, bit_data_size() * sizeof(uint64_t))));
        archive(_data);
    }
};

} // namespace min_ibf_fpga::index