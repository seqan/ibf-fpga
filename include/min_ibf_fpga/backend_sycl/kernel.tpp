#pragma once

#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include <min_ibf_fpga/backend_sycl/unrolled_loop.hpp>

// Kernel includes
#include <min_ibf_fpga/backend_sycl/kernel.hpp>


namespace min_ibf_fpga::backend_sycl
{

template <typename SyclMinimizerKernel, typename SyclIbfKernel>
void RunKernel(sycl::queue& queue,
	sycl::buffer<char, 1>& queries_buffer,
	const typename SyclMinimizerKernel::type::HostSizeType queriesOffset,
	sycl::buffer<typename SyclMinimizerKernel::type::HostSizeType, 1>& querySizes_buffer,
	const typename SyclMinimizerKernel::type::HostSizeType querySizesOffset,
	const typename SyclMinimizerKernel::type::HostSizeType numberOfQueries,
	sycl::buffer<typename SyclIbfKernel::type::Chunk, 1>& ibfData_buffer,
	const typename SyclIbfKernel::type::HostSizeType binSize,
	const typename SyclIbfKernel::type::HostSizeType hashShift,
	const typename SyclIbfKernel::type::HostSizeType minimalNumberOfMinimizers,
	const typename SyclIbfKernel::type::HostSizeType maximalNumberOfMinimizers,
	sycl::buffer<typename SyclIbfKernel::type::HostSizeType, 1>& thresholds_buffer,
	sycl::buffer<typename SyclIbfKernel::type::Chunk, 1>& result_buffer)
{
	using sycl_minimizer_kernel_t = typename SyclMinimizerKernel::type;
	using sycl_ibf_kernel_t = typename SyclIbfKernel::type;
	using constants = typename sycl_minimizer_kernel_t::constants;

	fpga_tools::UnrolledLoop<constants::number_of_kernels>([&](auto id)
	{
		static constexpr size_t pipe_id = id;

		queue.submit([&](sycl::handler &handler)
		{
			sycl_minimizer_kernel_t minimizer_kernel{
				.queries{queries_buffer, handler, sycl::read_only},
				.queriesOffset{queriesOffset},
				.querySizes{querySizes_buffer, handler, sycl::read_only},
				.querySizesOffset{querySizesOffset},
				.numberOfQueries{numberOfQueries}
			};
			handler.single_task<SyclMinimizerKernel>([minimizer_kernel]() [[intel::kernel_args_restrict]]
			{
				minimizer_kernel.template execute<pipe_id>();
			});
		});

		queue.submit([&](sycl::handler &handler)
		{
			sycl_ibf_kernel_t ibf_kernel{
				.ibfData{ibfData_buffer, handler, sycl::read_only},
				.binSize{binSize},
				.hashShift{hashShift},
				.numberOfQueries{numberOfQueries},
				.minimalNumberOfMinimizers{minimalNumberOfMinimizers},
				.maximalNumberOfMinimizers{maximalNumberOfMinimizers},
				.thresholds{thresholds_buffer, handler, sycl::read_only},
				.result{result_buffer, handler, sycl::write_only},
			};
			handler.single_task<SyclIbfKernel>([ibf_kernel]() [[intel::kernel_args_restrict]]
			{
				ibf_kernel.template execute<pipe_id>();
			});
		});

	});
}

} // namespace min_ibf_fpga::backend_sycl
