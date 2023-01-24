#include <iostream>
#include <vector>

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

#include "exception_handler.hpp"

// This code sample demonstrates how to split the host and FPGA kernel code into
// separate compilation units so that they can be separately recompiled.
// Consult the README for a detailed discussion.
//  - host.cpp (this file) contains exclusively code that executes on the host.
//  - kernel.cpp contains almost exclusively code that executes on the device.
//  - kernel.hpp contains only the forward declaration of a function containing
//    the device code.
#include "kernel.hpp"

using namespace sycl;

int main() {
  size_t binSize;
  size_t hashShift; // The number of bits to shift the hash value before doing multiplicative hashing.
  size_t minimalNumberOfMinimizers;
  size_t maximalNumberOfMinimizers;

  // Select either the FPGA emulator or FPGA device
#if defined(FPGA_EMULATOR)
  ext::intel::fpga_emulator_selector device_selector;
#else
  ext::intel::fpga_selector device_selector;
#endif

  try {

    // Create a queue bound to the chosen device.
    // If the device is unavailable, a SYCL runtime exception is thrown.
    queue q(device_selector, fpga_tools::exception_handler);

    // create the device buffers
    buffer queries_buffer();    // <char, 1>
    buffer querySizes_buffer(); // <HostSizeType, 1>
    buffer ibfData_buffer();    // <Chunk, 1>
    buffer thresholds_buffer(); // <HostSizeType, 1>
    buffer result_buffer();     // <Chunk, 1>

    // The definition of this function is in a different compilation unit,
    // so host and device code can be separately compiled.
    RunKernel(q,
      queries_buffer,
      const HostSizeType queriesOffset,
      querySizes_buffer,
      const HostSizeType querySizesOffset,
      const HostSizeType numberOfQueries,
      ibfData_buffer,
      binSize,
      hashShift,
      minimalNumberOfMinimizers,
      maximalNumberOfMinimizers,
      thresholds_buffer,
      result_buffer);

  } catch (exception const &e) {
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
  // has been synchronized. Therefore, the output data has been updated
  // with the results of the kernel and is safely accesible by the host CPU.

  return EXIT_SUCCESS;
}
