#include <filesystem>
#include <iostream>
#include <vector>

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

#include <min_ibf_fpga/backend_sycl/exception_handler.hpp>
#include <min_ibf_fpga/backend_sycl/kernel.hpp>

int main() {

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
  std::vector<min_ibf_fpga::backend_sycl::_types::HostSizeType> querySizes;
  std::ifstream queries_ifs(queries_filename, std::ios::binary);
  while (std::getline(queries_ifs, id)) {
    ids.push_back(id);
    std::getline(queries_ifs, query);
    queries.insert(queries.end(), query.begin(), query.end());
    querySizes.push_back(query.size());
    std::getline(queries_ifs, query); // ignore delimiter
    std::getline(queries_ifs, query); // ignore quality
  }

  size_t file_size, bytes_read, bytes_to_read;

  std::string ibfData_filename = "ibfdata.bin";
  std::vector<min_ibf_fpga::backend_sycl::_types::Chunk> ibfData;
  std::ifstream ibf_ifs(ibfData_filename, std::ios::binary);
  min_ibf_fpga::backend_sycl::_types::Chunk chunk;
  file_size = std::filesystem::file_size(ibfData_filename);
  assert(file_size > 0);
  bytes_read = 0;
  do {
    bytes_to_read = std::min(file_size - bytes_read, sizeof(chunk));
    ibf_ifs.read(reinterpret_cast<char*>(&chunk), bytes_to_read);
    ibfData.push_back(chunk);
    bytes_read += bytes_to_read;
  } while (bytes_read < file_size);

  std::string thresholds_filename = "thresholds_1e.bin";
  std::vector<min_ibf_fpga::backend_sycl::_types::HostSizeType> thresholds;
  std::ifstream th_ifs(thresholds_filename, std::ios::binary);
  min_ibf_fpga::backend_sycl::_types::HostSizeType threshold;
  file_size = std::filesystem::file_size(thresholds_filename);
  assert(file_size > 0);
  bytes_read = 0;
  do {
    bytes_to_read = std::min(file_size - bytes_read, sizeof(threshold));
    th_ifs.read(reinterpret_cast<char*>(&threshold), bytes_to_read);
    thresholds.push_back(threshold);
    bytes_read += bytes_to_read;
  } while (bytes_read < file_size);

  std::vector<min_ibf_fpga::backend_sycl::_types::Chunk> results;
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
    min_ibf_fpga::backend_sycl::RunKernel(q,
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
  ostrm.write(reinterpret_cast<char*>(results.data()), results.size() * sizeof(size_t));

  // Print results
  for (size_t i = 0; i < ids.size(); i++) {
    std::clog << ids[i].substr(1, std::string::npos) << "\t";
    uint64_t result = results[i];

    for (size_t byteOffset = 0; byteOffset < sizeof(uint64_t); ++byteOffset) {
      uint8_t& value = ((uint8_t*)&result)[byteOffset];

      for (size_t bitOffset = 0; bitOffset < 8; ++bitOffset) {
        if (value & (1 << 7))
          std::clog << byteOffset * 8 + bitOffset << ",";
        value <<= 1;
      }
    }
    std::clog << std::endl;
  }
  return EXIT_SUCCESS;
}
