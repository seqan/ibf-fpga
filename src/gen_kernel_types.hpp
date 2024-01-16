#ifndef WINDOW_SIZE
	#define WINDOW_SIZE 23
#endif

#ifndef MIN_IBF_K
	#define MIN_IBF_K 19
#endif

#ifndef BIN_COUNT
	#define BIN_COUNT 64
#endif

#include <min_ibf_fpga/backend_sycl/kernel.tpp>

//#include <min_ibf_fpga/backend_sycl/kernel.hpp>

#include <min_ibf_fpga/backend_sycl/min_ibf_constants.hpp>
#include <min_ibf_fpga/backend_sycl/min_ibf_types.hpp>
#include <min_ibf_fpga/backend_sycl/sycl_kernel_ibf.hpp>
#include <min_ibf_fpga/backend_sycl/sycl_kernel_minimizer.hpp>

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
struct MinimizerKernel
{
    using _constants = min_ibf_fpga::backend_sycl::min_ibf_constants<WINDOW_SIZE, MIN_IBF_K, BIN_COUNT>;
    using _backend = min_ibf_fpga::backend_sycl::sycl_backend;
    using _types = min_ibf_fpga::backend_sycl::min_ibf_types<_constants, _backend>;

    using type = min_ibf_fpga::backend_sycl::sycl_minimizer_kernel<_constants, _types>;
};

struct IbfKernel
{
    using _constants = MinimizerKernel::_constants;
    using _types = MinimizerKernel::_types;
    using _MinimizerToIBFPipes = MinimizerKernel::type::MinimizerToIBFPipes;

    using type = min_ibf_fpga::backend_sycl::sycl_ibf_kernel<_MinimizerToIBFPipes, _constants, _types>;
};

#ifndef MIN_IBF_FPGA_DEVICE_SOURCE
extern
#endif // MIN_IBF_FPGA_DEVICE_SOURCE
template
std::pair<sycl::event, sycl::event> min_ibf_fpga::backend_sycl::RunKernel<MinimizerKernel, IbfKernel>(sycl::queue& queue,
  char* queries_device_ptr,
  const typename MinimizerKernel::type::HostSizeType queriesOffset,
  const typename MinimizerKernel::type::HostSizeType* querySizes_device_ptr,
  const typename MinimizerKernel::type::HostSizeType querySizesOffset,
  const typename MinimizerKernel::type::HostSizeType numberOfQueries,
  const typename IbfKernel::type::Chunk* ibfData_device_ptr,
  const typename IbfKernel::type::HostSizeType binSize,
  const typename IbfKernel::type::HostSizeType hashShift,
  const typename IbfKernel::type::HostSizeType minimalNumberOfMinimizers,
  const typename IbfKernel::type::HostSizeType maximalNumberOfMinimizers,
  const typename IbfKernel::type::HostSizeType* thresholds_device_ptr,
  typename IbfKernel::type::Chunk* result_device_ptr,
  std::vector<sycl::event>* kernelDependencies);
