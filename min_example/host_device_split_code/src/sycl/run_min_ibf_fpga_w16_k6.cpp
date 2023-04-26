//==============================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================
#include <sycl/ext/intel/fpga_extensions.hpp>

#include "run_min_ibf_fpga_w16_k6.hpp"

#include "../kernel/min_ibf_fpga.hpp"

// This file contains 'almost' exclusively device code. The single-source SYCL
// code has been refactored between host.cpp and kernel.cpp to separate host and
// device code to the extent that the language permits.
//
// Note that ANY change in either this file or in kernel.hpp will be detected
// by the build system as a difference in the dependencies of device.o,
// triggering a full recompilation of the device code. 
//
// This is true even of trivial changes, e.g. tweaking the function definition 
// or the names of variables like 'q' or 'h', EVEN THOUGH these are not truly 
// "device code".


// Forward declare the kernel names in the global scope. This FPGA best practice
// reduces compiler name mangling in the optimization reports.
namespace zib
{

void run_min_ibf_fpga_w16_k6::operator()(
  sycl::queue& q,
  sycl::buffer<float,1>& buf_a,
  sycl::buffer<float,1>& buf_b,
  sycl::buffer<float,1>& buf_r,
  size_t size)
{
  using functor_name_t = std::remove_pointer_t<decltype(this)>;

  // submit the kernel
  q.submit([&](sycl::handler &h) {
    // Data accessors
    sycl::accessor a(buf_a, h, sycl::read_only);
    sycl::accessor b(buf_b, h, sycl::read_only);
    sycl::accessor r(buf_r, h, sycl::write_only, sycl::no_init);
    std::cout << "run_min_ibf_fpga_w" << w << "_k" << k << std::endl;

    // Kernel executes with pipeline parallelism on the FPGA.
    // Use kernel_args_restrict to specify that a, b, and r do not alias.
    h.single_task<functor_name_t>([=]() [[intel::kernel_args_restrict]] {
      zib::min_ibf_fpga<w, k> fpga_kernel{};
      fpga_kernel(size, a, b, r);
    });
  });
}
} // zib