#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include "pipe_utils.hpp"
#include "unrolled_loop.hpp"

// Kernel includes
#include "kernel.hpp"
#include "kernel_minimizer.hpp"

namespace min_ibf_fpga::backend_sycl
{

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
class MinimizerKernel;
class PipeToHostKernel;

void RunMinimizerKernel(sycl::queue& queue,
	sycl::buffer<char, 1>& queries_buffer,
	const _types::HostSizeType queriesOffset,
	sycl::buffer<_types::HostSizeType, 1>& querySizes_buffer,
	const _types::HostSizeType querySizesOffset,
	const _types::HostSizeType numberOfQueries,
	sycl::buffer<_types::MinimizerToIBFData, 1>& minimizerToIbf_buffer)
{
	using constants = _constants;
	using types = _types;
	using minimizer_kernel_t = minimizer_kernel<constants, types>;

	using MinimizerToIBFData = types::MinimizerToIBFData;
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, constants::number_of_kernels>;

	using PrefetchingLSU = sycl::ext::intel::lsu<sycl::ext::intel::prefetch<true>, sycl::ext::intel::statically_coalesce<false>>;

	fpga_tools::UnrolledLoop<constants::number_of_kernels>([&](auto id)
	{
		#include "kernel_minimizer.cpp"

		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor minimizerToIbf(minimizerToIbf_buffer, handler, sycl::write_only);

			handler.single_task<PipeToHostKernel>([=]() [[intel::kernel_args_restrict]]
			{
				size_t numberofMinimizersFound = 0;
				MinimizerToIBFData data;
				do
				{
					data = MinimizerToIBFPipes::PipeAt<id>::read();
					minimizerToIbf[numberofMinimizersFound] = data;
					numberofMinimizersFound++;
				}
				while(!data.isLastElement);
			});
		});
	});
}

} // namespace min_ibf_fpga::backend_sycl
