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
	char* queries_device_ptr,
	const typename SyclMinimizerKernel::type::HostSizeType queriesOffset,
	const typename SyclMinimizerKernel::type::HostSizeType* querySizes_device_ptr,
	const typename SyclMinimizerKernel::type::HostSizeType querySizesOffset,
	const typename SyclMinimizerKernel::type::HostSizeType numberOfQueries,
	const typename SyclIbfKernel::type::Chunk* ibfData_device_ptr,
	const typename SyclIbfKernel::type::HostSizeType binSize,
	const typename SyclIbfKernel::type::HostSizeType hashShift,
	const typename SyclIbfKernel::type::HostSizeType minimalNumberOfMinimizers,
	const typename SyclIbfKernel::type::HostSizeType maximalNumberOfMinimizers,
	const typename SyclIbfKernel::type::HostSizeType* thresholds_device_ptr,
	typename SyclIbfKernel::type::Chunk* result_device_ptr,
	sycl::event* minimizer_kernel_event,
	sycl::event* ibf_kernel_event);

} // namespace min_ibf_fpga::backend_sycl
