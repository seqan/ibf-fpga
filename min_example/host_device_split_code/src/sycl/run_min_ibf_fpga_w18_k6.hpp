//==============================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================
#pragma once

#include "run_min_ibf_fpga.hpp"

namespace zib
{
struct run_min_ibf_fpga_w18_k6;

#ifndef RUN_MIN_IBF_FPGA_W18_K6_CPP
extern
#endif
template
struct run_min_ibf_fpga<run_min_ibf_fpga_w18_k6, 18, 6>;

struct run_min_ibf_fpga_w18_k6
    : public run_min_ibf_fpga<run_min_ibf_fpga_w18_k6, 18, 6>
{};

} // zib

extern "C"
auto * run_min_ibf_fpga_w18_k6 = zib::run_min_ibf_fpga_w18_k6::execute;