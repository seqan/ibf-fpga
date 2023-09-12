#pragma once

#include <sycl/sycl.hpp>

// Note: This header only contains code that can easily be shared between host and device.

namespace min_ibf_fpga::backend_sycl
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename SyclMinimizerKernel, typename SyclIbfKernel>
void RunKernel(sycl::queue& queue,
       sycl::buffer<char, 1>& queries_buffer,
       const typename SyclMinimizerKernel::type::HostSizeType queriesOffset,
       sycl::buffer<typename SyclMinimizerKernel::type::HostSizeType, 1>& querySizes_buffer,
       const typename SyclMinimizerKernel::type::HostSizeType querySizesOffset,
       const typename SyclMinimizerKernel::type::HostSizeType numberOfQueries,
       const typename SyclIbfKernel::type::Chunk* ibfData_device_ptr,
       const typename SyclIbfKernel::type::HostSizeType binSize,
       const typename SyclIbfKernel::type::HostSizeType hashShift,
       const typename SyclIbfKernel::type::HostSizeType minimalNumberOfMinimizers,
       const typename SyclIbfKernel::type::HostSizeType maximalNumberOfMinimizers,
       const typename SyclIbfKernel::type::HostSizeType* thresholds_device_ptr,
       sycl::buffer<typename SyclIbfKernel::type::Chunk, 1>& result_buffer);

} // namespace min_ibf_fpga::backend_sycl
