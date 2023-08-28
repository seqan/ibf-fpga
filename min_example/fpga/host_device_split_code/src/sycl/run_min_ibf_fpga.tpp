#include <sycl/ext/intel/fpga_extensions.hpp>

#include "run_min_ibf_fpga.hpp"
#include "../kernel/min_ibf_fpga.hpp"

namespace zib
{

template <typename functor_name_t, size_t w, size_t k>
void run_min_ibf_fpga<functor_name_t, w, k>::execute(
  sycl::queue& q,
  sycl::buffer<float,1>& buf_a,
  sycl::buffer<float,1>& buf_b,
  sycl::buffer<float,1>& buf_r,
  size_t size)
{
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

} // namespace zib