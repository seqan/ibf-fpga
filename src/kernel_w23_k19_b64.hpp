#pragma once

#include <min_ibf_fpga/backend_sycl/kernel.hpp>

#include <min_ibf_fpga/backend_sycl/min_ibf_constants.hpp>
#include <min_ibf_fpga/backend_sycl/min_ibf_types.hpp>
#include <min_ibf_fpga/backend_sycl/sycl_kernel_ibf.hpp>
#include <min_ibf_fpga/backend_sycl/sycl_kernel_minimizer.hpp>

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
struct MinimizerKernel_w23_k19_b64
{
    using _constants = min_ibf_fpga::backend_sycl::min_ibf_constants<23, 19, 64>;
    using _backend = min_ibf_fpga::backend_sycl::sycl_backend;
    using _types = min_ibf_fpga::backend_sycl::min_ibf_types<_constants, _backend>;

    using type = min_ibf_fpga::backend_sycl::sycl_minimizer_kernel<_constants, _types>;
};

struct IbfKernel_w23_k19_b64
{
    using _constants = MinimizerKernel_w23_k19_b64::_constants;
    using _types = MinimizerKernel_w23_k19_b64::_types;
    using _MinimizerToIBFPipes = MinimizerKernel_w23_k19_b64::type::MinimizerToIBFPipes;

    using type = min_ibf_fpga::backend_sycl::sycl_ibf_kernel<_MinimizerToIBFPipes, _constants, _types>;
};

#ifndef MIN_IBF_FPGA_DEVICE_SOURCE
extern
#endif // MIN_IBF_FPGA_DEVICE_SOURCE
template
std::pair<sycl::event, sycl::event> min_ibf_fpga::backend_sycl::RunKernel<MinimizerKernel_w23_k19_b64, IbfKernel_w23_k19_b64>(sycl::queue& queue,
  char* queries_device_ptr,
  const typename MinimizerKernel_w23_k19_b64::type::HostSizeType queriesOffset,
  const typename MinimizerKernel_w23_k19_b64::type::HostSizeType* querySizes_device_ptr,
  const typename MinimizerKernel_w23_k19_b64::type::HostSizeType querySizesOffset,
  const typename MinimizerKernel_w23_k19_b64::type::HostSizeType numberOfQueries,
  const typename IbfKernel_w23_k19_b64::type::Chunk* ibfData_device_ptr,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType binSize,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType hashShift,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType minimalNumberOfMinimizers,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType maximalNumberOfMinimizers,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType* thresholds_device_ptr,
  typename IbfKernel_w23_k19_b64::type::Chunk* result_device_ptr,
  std::vector<sycl::event>* kernelDependencies);
