#pragma once

#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include <min_ibf_fpga/backend_sycl/unrolled_loop.hpp>

// Kernel includes
#include <min_ibf_fpga/backend_sycl/kernel.hpp>


namespace min_ibf_fpga::backend_sycl
{

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
	std::vector<sycl::event>* kernelDependencies)
{
	using sycl_minimizer_kernel_t = typename SyclMinimizerKernel::type;
	using sycl_ibf_kernel_t = typename SyclIbfKernel::type;
	using constants = typename sycl_minimizer_kernel_t::constants;

	std::pair<sycl::event, sycl::event> events;

	fpga_tools::UnrolledLoop<constants::number_of_kernels>([&](auto id)
	{
		static constexpr size_t pipe_id = id;

		events.first = queue.submit([&](sycl::handler &handler)
		{
			sycl_minimizer_kernel_t minimizer_kernel{
				.queries{queries_device_ptr},
				.queriesOffset{queriesOffset},
				.querySizes{querySizes_device_ptr},
				.querySizesOffset{querySizesOffset},
				.numberOfQueries{numberOfQueries}
			};
			handler.depends_on(*kernelDependencies);
			handler.single_task<SyclMinimizerKernel>([minimizer_kernel]() [[intel::kernel_args_restrict]]
			{
				minimizer_kernel.template execute<pipe_id>();
			});
		});

		events.second = queue.submit([&](sycl::handler &handler)
		{
			sycl_ibf_kernel_t ibf_kernel{
				.ibfData{ibfData_device_ptr},
				.binSize{binSize},
				.hashShift{hashShift},
				.numberOfQueries{numberOfQueries},
				.minimalNumberOfMinimizers{minimalNumberOfMinimizers},
				.maximalNumberOfMinimizers{maximalNumberOfMinimizers},
				.thresholds{thresholds_device_ptr},
				.result{result_device_ptr},
			};
			handler.depends_on(*kernelDependencies);
			handler.single_task<SyclIbfKernel>([ibf_kernel]() [[intel::kernel_args_restrict]]
			{
				ibf_kernel.template execute<pipe_id>();
			});
		});

	});

	return events;
}

} // namespace min_ibf_fpga::backend_sycl
