//==============================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================
#pragma once

#include "run_min_ibf_fpga.hpp"

namespace zib
{
struct run_min_ibf_fpga_w16_k8;

// This `ifndef` statement has two functions.
//
// Case 1, extern is defined:
// When included as normal header, the `extern` keyword will postpone any instantiation of the templated class
// that means `run_min_ibf_fpga<...>::operator(...)` does not need to be defined and the compiler is expecting
// a symbol `zib::run_min_ibf_fpga<zib::run_min_ibf_fpga_w16_k8, 18ul, 6ul>::operator()(sycl::queue&, sycl::buffer<float, 1>&, sycl::buffer<float, 1>&, sycl::buffer<float, 1>&, unsigned long)`
// will be expected at linking time.
//
// Case 2, extern is not defined:
// When included from cpp, `RUN_MIN_IBF_FPGA_W16_K8_CPP` is defined, we use the explicit instantiation of `run_min_ibf_fpga<...>`
// that means `run_min_ibf_fpga<...>::operator(...)` will be explicitly instantiated and the corresponding symbol will be created
#ifndef RUN_MIN_IBF_FPGA_W16_K8_CPP
extern
#endif
template
struct run_min_ibf_fpga<run_min_ibf_fpga_w16_k8, 16, 8>;

struct run_min_ibf_fpga_w16_k8
    : public run_min_ibf_fpga<run_min_ibf_fpga_w16_k8, 16, 8>
{};

} // zib