#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include "pipe_utils.hpp"
#include "unrolled_loop.hpp"

// Kernel includes
#include "kernel.hpp"
#include "kernel_ibf.hpp"
#include "sycl_kernel_ibf.hpp"

namespace min_ibf_fpga::backend_sycl
{

template <typename SyclIbfKernel, typename HostToKernelPipe>
void RunIBFKernel(sycl::queue& queue,
	sycl::buffer<typename SyclIbfKernel::type::MinimizerToIBFData, 1>& minimizerToIbf_buffer,
	sycl::buffer<typename SyclIbfKernel::type::Chunk, 1>& ibfData_buffer,
	const typename SyclIbfKernel::type::HostSizeType binSize,
	const typename SyclIbfKernel::type::HostSizeType hashShift,
	const typename SyclIbfKernel::type::HostSizeType numberOfQueries,
	const typename SyclIbfKernel::type::HostSizeType minimalNumberOfMinimizers,
	const typename SyclIbfKernel::type::HostSizeType maximalNumberOfMinimizers,
	sycl::buffer<typename SyclIbfKernel::type::HostSizeType, 1>& thresholds_buffer,
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
