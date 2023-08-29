
#pragma once

#include <CL/sycl.hpp>
#include <stddef.h>

namespace zib
{

template <typename functor_name_t, size_t w, size_t k>
struct run_min_ibf_fpga
{
    static void execute(
        sycl::queue& q,
        sycl::buffer<float,1>& buf_a,
        sycl::buffer<float,1>& buf_b,
        sycl::buffer<float,1>& buf_r,
        size_t size);
};

} // namespace zib