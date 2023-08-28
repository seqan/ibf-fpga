
#pragma once

namespace zib
{

template <unsigned w, unsigned k>
struct min_ibf_fpga
{
    template <typename array_read_t, typename array_write_t>
    void operator()(std::size_t const size, array_read_t const & a, array_read_t const & b, array_write_t & r)
    {
        r[size] = 100 * w + k;
        // do IBF computation
        for (size_t i = 0; i < size; ++i) {
            r[i] = a[i] + b[i] + w * 100 + k;
        }
    }
};

} // namespace zib