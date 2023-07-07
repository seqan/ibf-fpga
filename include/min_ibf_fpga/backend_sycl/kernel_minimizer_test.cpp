#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include "pipe_utils.hpp"
#include "unrolled_loop.hpp"

// Kernel includes
#include "kernel.hpp"
#include "sycl_kernel_minimizer.hpp"

namespace min_ibf_fpga::backend_sycl
{

template <typename SyclMinimizerKernel, typename pipe_to_host_kernel_t>
void RunMinimizerKernel(sycl::queue& queue,
	sycl::buffer<char, 1>& queries_buffer,
	const typename SyclMinimizerKernel::type::HostSizeType queriesOffset,
	sycl::buffer<typename SyclMinimizerKernel::type::HostSizeType, 1>& querySizes_buffer,
	const typename SyclMinimizerKernel::type::HostSizeType querySizesOffset,
	const typename SyclMinimizerKernel::type::HostSizeType numberOfQueries,
	sycl::buffer<typename SyclMinimizerKernel::type::MinimizerToIBFData, 1>& minimizerToIbf_buffer)
{
	using sycl_minimizer_kernel_t = typename SyclMinimizerKernel::type;
	using constants = typename sycl_minimizer_kernel_t::constants;

	using MinimizerToIBFData = typename sycl_minimizer_kernel_t::MinimizerToIBFData;
	using MinimizerToIBFPipes = typename sycl_minimizer_kernel_t::MinimizerToIBFPipes;

	fpga_tools::UnrolledLoop<constants::number_of_kernels>([&](auto id)
	{
		static constexpr size_t pipe_id = id;

		queue.submit([&](sycl::handler &handler)
		{
			sycl_minimizer_kernel_t minimizer_kernel{
				/*.queries*/sycl::accessor{queries_buffer, handler, sycl::read_only},
				/*.queriesOffset*/{queriesOffset},
				/*.querySizes*/sycl::accessor{querySizes_buffer, handler, sycl::read_only},
				/*.querySizesOffset*/{querySizesOffset},
				/*.numberOfQueries*/{numberOfQueries}
			};
			handler.single_task<SyclMinimizerKernel>([minimizer_kernel]() [[intel::kernel_args_restrict]]
			{
				minimizer_kernel.template execute<pipe_id>();
			});
		});

		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor minimizerToIbf(minimizerToIbf_buffer, handler, sycl::write_only);

			handler.single_task<pipe_to_host_kernel_t>([=]() [[intel::kernel_args_restrict]]
			{
				size_t numberofMinimizersFound = 0;
				MinimizerToIBFData data;
				do
				{
					using pipe_t = typename MinimizerToIBFPipes::template PipeAt<pipe_id>;
					data = pipe_t::read();
					minimizerToIbf[numberofMinimizersFound] = data;
					numberofMinimizersFound++;
				}
				while(!data.isLastElement);
			});
		});
	});
}

} // namespace min_ibf_fpga::backend_sycl
