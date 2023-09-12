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
void min_ibf_fpga::backend_sycl::RunKernel<MinimizerKernel_w23_k19_b64, IbfKernel_w23_k19_b64>(sycl::queue& queue,
  sycl::buffer<char, 1>& queries_buffer,
  const typename MinimizerKernel_w23_k19_b64::type::HostSizeType queriesOffset,
  sycl::buffer<typename MinimizerKernel_w23_k19_b64::type::HostSizeType, 1>& querySizes_buffer,
  const typename MinimizerKernel_w23_k19_b64::type::HostSizeType querySizesOffset,
  const typename MinimizerKernel_w23_k19_b64::type::HostSizeType numberOfQueries,
  const typename IbfKernel_w23_k19_b64::type::Chunk * ibfData_device_ptr,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType binSize,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType hashShift,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType minimalNumberOfMinimizers,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType maximalNumberOfMinimizers,
  const typename IbfKernel_w23_k19_b64::type::HostSizeType * thresholds_device_ptr,
  sycl::buffer<typename IbfKernel_w23_k19_b64::type::Chunk, 1>& result_buffer);
