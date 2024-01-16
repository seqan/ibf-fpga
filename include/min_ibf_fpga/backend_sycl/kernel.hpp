#pragma once

#include <sycl/sycl.hpp>

namespace min_ibf_fpga::backend_sycl
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename SyclMinimizerKernel, typename SyclIbfKernel>
std::pair<sycl::event, sycl::event> RunKernel(sycl::queue& queue,
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
	std::vector<sycl::event>* kernelDependencies);

} // namespace min_ibf_fpga::backend_sycl
