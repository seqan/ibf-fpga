//==============================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================
#include <CL/sycl.hpp>

namespace zib
{
struct run_min_ibf_fpga_w16_k6
{
    static constexpr size_t w = 16, k = 6;

    void operator()(
        sycl::queue& q,
        sycl::buffer<float,1>& buf_a,
        sycl::buffer<float,1>& buf_b,
        sycl::buffer<float,1>& buf_r,
        size_t size);
};
} // zib