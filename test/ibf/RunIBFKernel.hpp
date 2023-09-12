#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include <min_ibf_fpga/backend_sycl/unrolled_loop.hpp>

// Kernel includes
#include <min_ibf_fpga/backend_sycl/min_ibf_constants.hpp>
#include <min_ibf_fpga/backend_sycl/min_ibf_types.hpp>
#include <min_ibf_fpga/backend_sycl/sycl_kernel_ibf.hpp>

namespace min_ibf_fpga::backend_sycl
{

template <typename SyclIbfKernel, typename HostToKernelPipe>
void RunIBFKernel(sycl::queue& queue,
	sycl::buffer<typename SyclIbfKernel::type::MinimizerToIBFData, 1>& minimizerToIbf_buffer,
	const typename SyclIbfKernel::type::Chunk * ibfData_device_ptr,
	const typename SyclIbfKernel::type::HostSizeType binSize,
	const typename SyclIbfKernel::type::HostSizeType hashShift,
	const typename SyclIbfKernel::type::HostSizeType numberOfQueries,
	const typename SyclIbfKernel::type::HostSizeType minimalNumberOfMinimizers,
	const typename SyclIbfKernel::type::HostSizeType maximalNumberOfMinimizers,
	const typename SyclIbfKernel::type::HostSizeType * thresholds_device_ptr,
	sycl::buffer<typename SyclIbfKernel::type::Chunk, 1>& result_buffer)
{
	using sycl_ibf_kernel_t = typename SyclIbfKernel::type;
	using constants = typename sycl_ibf_kernel_t::constants;

	using MinimizerToIBFPipes = typename sycl_ibf_kernel_t::MinimizerToIBFPipes;

	fpga_tools::UnrolledLoop<constants::number_of_kernels>([&](auto id)
	{
		static constexpr size_t pipe_id = id;

		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor minimizerToIbf(minimizerToIbf_buffer, handler, sycl::read_only);

			handler.single_task<HostToKernelPipe>([=]() [[intel::kernel_args_restrict]]
			{
				for (size_t i = 0; i < minimizerToIbf.size(); i++)
				{
					using pipe_t = typename MinimizerToIBFPipes::template PipeAt<pipe_id>;
					pipe_t::write(minimizerToIbf[i]);
				}
			});
		});

		queue.submit([&](sycl::handler &handler)
		{
			sycl_ibf_kernel_t ibf_kernel{
				.ibfData{ibfData_device_ptr},
				.binSize{binSize},
				.hashShift{hashShift},
				.numberOfQueries{numberOfQueries},
				.minimalNumberOfMinimizers{minimalNumberOfMinimizers},
				.maximalNumberOfMinimizers{maximalNumberOfMinimizers},
				.thresholds{thresholds_device_ptr},
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
