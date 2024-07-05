#include <experimental/filesystem>
namespace std {
  namespace filesystem = experimental::filesystem;
}
#include <iostream>
#include <vector>

#include <dlfcn.h>

#if __INTEL_LLVM_COMPILER < 20230000
	#include <CL/sycl.hpp>
#else
	#include <sycl/sycl.hpp>
#endif
#include <sycl/ext/intel/fpga_extensions.hpp>

#include <min_ibf_fpga/fastq/fastq_parser.hpp>
#include <min_ibf_fpga/io/print_results.hpp>
#include <min_ibf_fpga/io/read_in_binary_data.hpp>
#include <min_ibf_fpga/io/write_out_binary_data.hpp>

#include <min_ibf_fpga/backend_sycl/exception_handler.hpp>
#include <min_ibf_fpga/backend_sycl/shared.hpp>

template <size_t chunk_bits>
int RunHost() {
  using HostSizeType = min_ibf_fpga::backend_sycl::HostSizeType;
  using Chunk = ac_int<chunk_bits, false>;

  static_assert(sizeof(size_t) == 8);

  size_t const window_size = 23;
  size_t const kmer_size = 19;
  size_t const number_of_bins = 64;
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

#if __INTEL_LLVM_COMPILER < 20230100
	#ifdef FPGA_EMULATOR
  sycl::ext::intel::fpga_emulator_selector device_selector;
	#else
  sycl::ext::intel::fpga_selector device_selector;
	#endif
#else
	#ifdef FPGA_EMULATOR
  auto device_selector = sycl::ext::intel::fpga_emulator_selector_v;
	#else
  auto device_selector = sycl::ext::intel::fpga_selector_v;
	#endif
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

    // Malloc on device and transfer data
    auto ibfData_device_ptr = sycl::malloc_device<Chunk>(ibfData.size(), q);
    static_assert(std::is_same_v<decltype(ibfData_device_ptr), Chunk *>);
    q.memcpy(ibfData_device_ptr, ibfData.data(), ibfData.size() * sizeof(Chunk));

    auto thresholds_device_ptr = sycl::malloc_device<HostSizeType>(thresholds.size(), q);
    static_assert(std::is_same_v<decltype(thresholds_device_ptr), HostSizeType *>);
    q.memcpy(thresholds_device_ptr, thresholds.data(), thresholds.size() * sizeof(HostSizeType));

    auto queries_host_ptr = sycl::malloc_host<char>(queries.size(), q);
    static_assert(std::is_same_v<decltype(queries_host_ptr), char *>);
    std::memcpy(queries_host_ptr, queries.data(), queries.size() * sizeof(char));

    auto querySizes_host_ptr = sycl::malloc_host<HostSizeType>(querySizes.size(), q);
    static_assert(std::is_same_v<decltype(querySizes_host_ptr), HostSizeType *>);
    std::memcpy(querySizes_host_ptr, querySizes.data(), querySizes.size() * sizeof(HostSizeType));

    auto results_host_ptr = sycl::malloc_host<Chunk>(results.size(), q);
    static_assert(std::is_same_v<decltype(results_host_ptr), Chunk *>);

    std::pair<sycl::event, sycl::event> events;

#if FPGA_HARDWARE
    std::string library_suffix = ".fpga.so";
#else
    std::string library_suffix = ".fpga_emu.so";
#endif

    std::string name = "libmin-ibf-fpga-oneapi_kernel_w" + std::to_string(window_size) + "_k" + std::to_string(kmer_size) + "_b" + std::to_string(number_of_bins) + library_suffix;
    std::filesystem::path library_path = std::filesystem::current_path() / name;

    auto kernel_lib = dlopen(library_path.c_str(), RTLD_NOW);
    auto RunKernel = (void (*)(
      sycl::queue&,
      const char*,
      const HostSizeType,
      const HostSizeType*,
      const HostSizeType,
      const HostSizeType,
      const Chunk*,
      const HostSizeType,
      const HostSizeType,
      const HostSizeType,
      const HostSizeType,
      const HostSizeType*,
      Chunk*,
      std::pair<sycl::event, sycl::event>*
      ))dlsym(kernel_lib, "RunKernel");

    q.wait(); // Wait for data transfers to finish (USM)

    // The definition of this function is in a different compilation unit, so host and device code can be separately compiled.
    RunKernel(q,
      queries_host_ptr,
      queriesOffset,
      querySizes_host_ptr,
      querySizesOffset,
      querySizes.size(), // numberOfQueries
      ibfData_device_ptr,
      bin_size,
      hash_shift,
      minimalNumberOfMinimizers,
      maximalNumberOfMinimizers,
      thresholds_device_ptr,
      results_host_ptr,
      &events);

    q.wait(); // Wait for RunKernel to finish before copying back results

    // Copy back results (could probably skip that and use results_host_ptr directly)
    q.memcpy(results.data(), results_host_ptr, results.size() * sizeof(Chunk));

    q.wait(); // Wait for results to be copied back before freeing device memory

    sycl::free(queries_host_ptr, q);
    sycl::free(querySizes_host_ptr, q);
    sycl::free(ibfData_device_ptr, q);
    sycl::free(thresholds_device_ptr, q);
    sycl::free(results_host_ptr, q);

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

  // Dump results to binary file
  std::ofstream ostrm("results.bin", std::ios::binary);
  min_ibf_fpga::io::write_out_binary_data(ostrm, results);

  // Print results
  min_ibf_fpga::io::print_results(ids, results, std::clog);
  return EXIT_SUCCESS;
}

int main() {
    size_t const chunk_bits = 64;
    RunHost<chunk_bits>();
}
