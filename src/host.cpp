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
#include "kernel/kernel.hpp"

using namespace sycl;

int main() {

  static_assert(sizeof(size_t) == 8);

  size_t const window_size = 23;
  size_t const kmer_size = 19;
  size_t const pattern_size = 0; // TODO

  size_t const kmers_per_window = window_size - kmer_size + 1;
  size_t const kmers_per_pattern = pattern_size - kmer_size + 1;

  size_t const binSize = 0; // TODO The size of each bin in bits.
  size_t const hashShift = 0; // TODO The number of bits to shift the hash value before doing multiplicative hashing.
  size_t const minimalNumberOfMinimizers = kmers_per_window == 1 ? kmers_per_pattern	: std::ceil(static_cast<double>(kmers_per_pattern) / static_cast<double>(kmers_per_window));
  size_t const maximalNumberOfMinimizers = pattern_size - window_size + 1;

  size_t const numberOfQueries = 1;
  size_t const sequenceSize = 65; // querySize
  size_t const queriesOffset = 0;
  size_t const querySizesOffset = 0;

  std::string queries = "ACGATCGACTAGGAGCGATTACGACTGACTACATCTAGCTAGCTAGAGATTCTTCAGAGCTTAGC";
  //std::array<char, sequenceSize * numberOfQueries + 1> queries;
  std::array<size_t, numberOfQueries> querySizes = {sequenceSize};

  std::stringstream ibf_filename_ss;
  ibf_filename_ss << "raptor_" << window_size << "_" << kmer_size << ".index";
  std::string ibf_filename = ibf_filename_ss.str();

  size_t ibf_filesize = 8253;
  std::vector<char> ibfData; // raptor uses cereal::BinaryInputArchive to read and sdsl::bit_vector to store, propably doesn't work
  std::ifstream ibfData_ifs{ibf_filename, std::ios::binary};
  ibfData_ifs.read(ibfData.data(), ibf_filesize);

  std::vector<size_t> thresholds; // TODO

  std::vector<uint64_t> result;

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
    buffer queries_buffer(queries);
    buffer querySizes_buffer(querySizes);
    buffer ibfData_buffer(ibfData);
    buffer thresholds_buffer(thresholds);
    buffer result_buffer(result);

    // The definition of this function is in a different compilation unit,
    // so host and device code can be separately compiled.
    RunKernel(q,
      queries_buffer,
      queriesOffset,
      querySizes_buffer,
      querySizesOffset,
      numberOfQueries,
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
