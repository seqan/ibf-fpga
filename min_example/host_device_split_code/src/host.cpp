//==============================================================
// Copyright Intel Corporation
//
// SPDX-License-Identifier: MIT
// =============================================================

#include <iostream>
#include <vector>

#include <CL/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

// dpc_common.hpp can be found in the dev-utilities include folder.
// e.g., $ONEAPI_ROOT/dev-utilities//include/dpc_common.hpp
namespace dpc_common {
// This exception handler will catch async exceptions
static auto exception_handler = [](cl::sycl::exception_list eList) {
  for (std::exception_ptr const &e : eList) {
    try {
      std::rethrow_exception(e);
    } catch (std::exception const &e) {
#if _DEBUG
      std::cout << "Failure" << std::endl;
#endif
      std::terminate();
    }
  }
};

};  // namespace dpc_common

// This code sample demonstrates how to split the host and FPGA kernel code into
// separate compilation units so that they can be separately recompiled.
// Consult the README for a detailed discussion.
//  - host.cpp (this file) contains exclusively code that executes on the host.
//  - kernel.cpp contains almost exclusively code that executes on the device.
//  - kernel.hpp contains only the forward declaration of a function containing
//    the device code.
#include "sycl/run_min_ibf_fpga_w16_k8.hpp"
#include "sycl/run_min_ibf_fpga_w16_k6.hpp"
// #include "sycl/run_min_ibf_fpga_w18_k6.hpp"
#include "sycl_kernel_loader.hpp"

// the tolerance used in floating point comparisons
constexpr float kTol = 0.001;

// the array size of vectors a, b and c
constexpr size_t kArraySize = 32;

template <size_t w, size_t k, typename kernel_fn_t>
bool execute_and_test(kernel_fn_t && kernel_fn)
{
  std::vector<float> vec_a(kArraySize);
  std::vector<float> vec_b(kArraySize);
  std::vector<float> vec_r(kArraySize + 1);

  // Fill vectors a and b with random float values
  for (size_t i = 0; i < kArraySize; i++) {
    vec_a[i] = rand() / (float)RAND_MAX;
    vec_b[i] = rand() / (float)RAND_MAX;
  }

#if FPGA_SIMULATOR
  auto device_selector = sycl::ext::intel::fpga_simulator_selector_v;
#elif FPGA_HARDWARE
  auto device_selector = sycl::ext::intel::fpga_selector_v;
#else  // #if FPGA_EMULATOR
  auto device_selector = sycl::ext::intel::fpga_emulator_selector_v;
#endif


  try {

    // Create a queue bound to the chosen device.
    // If the device is unavailable, a SYCL runtime exception is thrown.
    sycl::queue q(device_selector, dpc_common::exception_handler);

    // create the device buffers
    sycl::buffer device_a(vec_a);
    sycl::buffer device_b(vec_b);
    sycl::buffer device_r(vec_r);

    // The definition of this function is in a different compilation unit,
    // so host and device code can be separately compiled.
    kernel_fn(q, device_a, device_b, device_r, kArraySize);

  } catch (sycl::exception const &e) {
    // Catches exceptions in the host code
    std::cerr << "Caught a SYCL host exception:\n" << e.what() << "\n";

    // Most likely the runtime couldn't find FPGA hardware!
    if (e.code().value() == CL_DEVICE_NOT_FOUND) {
      std::cerr << "If you are targeting an FPGA, please ensure that your "
                   "system has a correctly configured FPGA board.\n";
      std::cerr << "Run sys_check in the oneAPI root directory to verify.\n";
      std::cerr << "If you are targeting the FPGA emulator, compile with "
                   "-DFPGA_EMULATOR.\n";
    }
    std::terminate();
  }

  // At this point, the device buffers have gone out of scope and the kernel
  // has been synchronized. Therefore, the output data (vec_r) has been updated
  // with the results of the kernel and is safely accesible by the host CPU.

  // Test the results
  size_t correct = 0;
  for (size_t i = 0; i < kArraySize; i++) {
    float expected = vec_a[i] + vec_b[i] + w * 100 + k;
    float actual = vec_r[i];

    float diff = expected - actual;
    if (diff * diff < kTol * kTol) {
      correct++;
    } else {
      std::cout << "actual: " << actual << " <=> " << expected << " :expected" << std::endl;
    }
  }

  std::cout << "w: " << w << std::endl;
  std::cout << "k: " << k << std::endl;
  std::cout << "mode: " << vec_r[kArraySize] << std::endl;

  // Summarize results
  if (correct == kArraySize) {
    std::cout << "PASSED: " << correct << "/" << kArraySize << " results are correct\n";
  } else {
    std::cout << "FAILED: " << correct << "/" << kArraySize << " results are incorrect\n";
  }

  return correct == kArraySize;
}

int main() {
  bool passed = true;

  passed = passed & execute_and_test<16, 8>(zib::run_min_ibf_fpga_w16_k8::execute);
  passed = passed & execute_and_test<16, 6>(zib::run_min_ibf_fpga_w16_k6{});

  {
    using function_ptr_t = void (sycl::queue& q, sycl::buffer<float,1>& buf_a, sycl::buffer<float,1>& buf_b, sycl::buffer<float,1>& buf_r, size_t size);
    std::filesystem::path library_path = std::filesystem::current_path() / "librun_min_ibf_fpga_w18_k6.fpga_emu_kernel.so";
    std::string_view symbol_name = "run_min_ibf_fpga_w18_k6";

    sycl_kernel_loader<function_ptr_t *> run_min_ibf_fpga_w18_k6{library_path, symbol_name};
    passed = passed & execute_and_test<18, 6>(run_min_ibf_fpga_w18_k6);
  }

  return !passed;
}
