#include <sycl/sycl.hpp>
#if FPGA_HARDWARE || FPGA_EMULATOR || FPGA_SIMULATOR
#include <sycl/ext/intel/fpga_extensions.hpp>
#endif

extern "C"
{
void add(int *const vec_a, int *const vec_b, int *const vec_c, int kVectSize, sycl::queue & queueA);
void mul(int *const vec_a, int *const vec_b, int *const vec_c, int kVectSize, sycl::queue & queueA);
}

bool verify_add(
  int *const vec_a,
  int *const vec_b,
  int *const vec_c,
  int kVectSize)
{
    bool passed = true;
    // verify that vec_c is correct
    for (int i = 0; i < kVectSize; i++) {
      int expected = vec_a[i] + vec_b[i];
      if (vec_c[i] != expected) {
        std::cout << "idx=" << i << ": result " << vec_c[i] << ", expected ("
                  << expected << ") A=" << vec_a[i] << " + B=" << vec_b[i]
                  << std::endl;
        passed = false;
      }
    }

    std::cout << (passed ? "PASSED" : "FAILED") << std::endl;
    return passed;
}

bool verify_mul(
  int *const vec_a,
  int *const vec_b,
  int *const vec_c,
  int kVectSize)
{
    bool passed = true;
    // verify that vec_c is correct
    for (int i = 0; i < kVectSize; i++) {
      int expected = vec_a[i] * vec_b[i];
      if (vec_c[i] != expected) {
        std::cout << "idx=" << i << ": result " << vec_c[i] << ", expected ("
                  << expected << ") A=" << vec_a[i] << " * B=" << vec_b[i]
                  << std::endl;
        passed = false;
      }
    }

    std::cout << (passed ? "PASSED" : "FAILED") << std::endl;
    return passed;
}

constexpr int kVectSize = 256;

int main() {
  bool passed = true;
  try {
    // Use compile-time macros to select either:
    //  - the FPGA emulator device (CPU emulation of the FPGA)
    //  - the FPGA device (a real FPGA)
    //  - the simulator device
#if FPGA_SIMULATOR
    auto selector = sycl::ext::intel::fpga_simulator_selector_v;
#elif FPGA_HARDWARE
    auto selector = sycl::ext::intel::fpga_selector_v;
#elif FPGA_EMULATOR
    auto selector = sycl::ext::intel::fpga_emulator_selector_v;
#else
    auto selector = sycl::default_selector_v;
#endif

    // create the device queue
    sycl::queue q(selector);

    auto device = q.get_device();

    std::cout << "Running on device: "
              << device.get_info<sycl::info::device::name>().c_str()
              << std::endl;

    if (!device.has(sycl::aspect::usm_host_allocations)) {
      std::terminate();
    }

    // declare arrays and fill them
    // allocate in shared memory so the kernel can see them
    int *vec_a = sycl::malloc_shared<int>(kVectSize, q);
    int *vec_b = sycl::malloc_shared<int>(kVectSize, q);
    int *vec_c = sycl::malloc_shared<int>(kVectSize, q);
    for (int i = 0; i < kVectSize; i++) {
      vec_a[i] = i;
      vec_b[i] = (kVectSize - i);
    }

    std::cout << "add two vectors of size " << kVectSize << std::endl;

    add(vec_a, vec_b, vec_c, kVectSize, q);

    passed &= verify_add(vec_a, vec_b, vec_c, kVectSize);

    std::cout << "multiply two vectors of size " << kVectSize << std::endl;
    mul(vec_a, vec_b, vec_c, kVectSize, q);

    passed &= verify_mul(vec_a, vec_b, vec_c, kVectSize);

    sycl::free(vec_a, q);
    sycl::free(vec_b, q);
    sycl::free(vec_c, q);
  } catch (sycl::exception const &e) {
    // Catches exceptions in the host code.
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
  return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}