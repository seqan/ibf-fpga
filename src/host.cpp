#include <filesystem>
#include <iostream>
#include <vector>

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

#include <min_ibf_fpga/fastq/fastq_parser.hpp>
#include <min_ibf_fpga/io/print_results.hpp>
#include <min_ibf_fpga/io/read_in_binary_data.hpp>
#include <min_ibf_fpga/io/write_out_binary_data.hpp>

#include <min_ibf_fpga/backend_sycl/exception_handler.hpp>
#include <min_ibf_fpga/backend_sycl/kernel.hpp>

#include <min_ibf_fpga/backend_sycl/sycl_kernel_ibf.hpp>
#include <min_ibf_fpga/backend_sycl/sycl_kernel_minimizer.hpp>

#include "kernel_w23_k19_b64.hpp"

int main() {
  using HostSizeType = MinimizerKernel_w23_k19_b64::type::HostSizeType;
  using Chunk = IbfKernel_w23_k19_b64::type::Chunk;

  static_assert(sizeof(size_t) == 8);

  size_t const window_size = 23;
  size_t const kmer_size = 19;
  size_t const pattern_size = 65;

  size_t const kmers_per_window = window_size - kmer_size + 1;
  size_t const kmers_per_pattern = pattern_size - kmer_size + 1;

  size_t const bin_size = 1024; // The size of each bin in bits.
  size_t const hash_shift = 53; // The number of bits to shift the hash value before doing multiplicative hashing.
  size_t const minimalNumberOfMinimizers = kmers_per_window == 1 ? kmers_per_pattern : std::ceil(static_cast<double>(kmers_per_pattern) / static_cast<double>(kmers_per_window));
  size_t const maximalNumberOfMinimizers = pattern_size - window_size + 1;

  size_t const queriesOffset = 0;
  size_t const querySizesOffset = 0;

  std::string queries_filename = "query.fq";
  std::string query, id;
  std::vector<std::string> ids;
  std::vector<char> queries;
  std::vector<HostSizeType> querySizes;
  std::ifstream queries_ifs(queries_filename, std::ios::binary);

  min_ibf_fpga::fastq::fastq_parser parser{.inputStream = std::move(queries_ifs)};
  parser([&](auto && id, auto && query)
  {
    ids.push_back(id);

    queries.insert(queries.end(), query.begin(), query.end());
    querySizes.push_back(query.size());
  });

  std::vector<Chunk> ibfData;
  {
    std::string ibfData_filename = "ibfdata.bin";
    std::ifstream ibfData_ifs(ibfData_filename, std::ios::binary);
    min_ibf_fpga::io::read_in_binary_data(ibfData_ifs, ibfData);
  }

  std::vector<HostSizeType> thresholds;
  {
    std::string thresholds_filename = "thresholds_1e.bin";
    std::ifstream thresholds_ifs(thresholds_filename, std::ios::binary);
    min_ibf_fpga::io::read_in_binary_data(thresholds_ifs, thresholds);
  }

  std::vector<Chunk> results;
  results.resize(querySizes.size()); // numberOfQueries

#if FPGA_SIMULATOR
  auto device_selector = sycl::ext::intel::fpga_simulator_selector_v;
#elif FPGA_HARDWARE
  auto device_selector = sycl::ext::intel::fpga_selector_v;
#else  // #if FPGA_EMULATOR
  auto device_selector = sycl::ext::intel::fpga_emulator_selector_v;
#endif

#ifdef DEBUG
  {
#else
  try {
#endif

    // Create a queue bound to the chosen device.
    // If the device is unavailable, a SYCL runtime exception is thrown.
    // Note: SYCL queues are out of order by default
    sycl::queue q(device_selector, fpga_tools::exception_handler);

    // Create the device buffers
    sycl::buffer queries_buffer(queries);
    sycl::buffer querySizes_buffer(querySizes);
    sycl::buffer ibfData_buffer(ibfData);
    sycl::buffer thresholds_buffer(thresholds);
    sycl::buffer results_buffer(results);

    // The definition of this function is in a different compilation unit,
    // so host and device code can be separately compiled.
    min_ibf_fpga::backend_sycl::RunKernel<MinimizerKernel_w23_k19_b64, IbfKernel_w23_k19_b64>(q,
      queries_buffer,
      queriesOffset,
      querySizes_buffer,
      querySizesOffset,
      querySizes.size(), // numberOfQueries
      ibfData_buffer,
      bin_size,
      hash_shift,
      minimalNumberOfMinimizers,
      maximalNumberOfMinimizers,
      thresholds_buffer,
      results_buffer);

#ifdef DEBUG
  }
#else
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
#endif

  // At this point, the device buffers have gone out of scope and the kernel
  // has been synchronized. Therefore, the output data has been updated
  // with the results of the kernel and is safely accessible by the host CPU.

  // Dump results to binary file
  std::ofstream ostrm("results.bin", std::ios::binary);
  min_ibf_fpga::io::write_out_binary_data(ostrm, results);

  // Print results
  min_ibf_fpga::io::print_results(ids, results, std::clog);
  return EXIT_SUCCESS;
}
