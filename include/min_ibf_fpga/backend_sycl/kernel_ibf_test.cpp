#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include "pipe_utils.hpp"
#include "unrolled_loop.hpp"

// Kernel includes
#include "kernel.hpp"
#include "kernel_ibf.hpp"

namespace min_ibf_fpga::backend_sycl
{

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
class HostToKernelPipe;
class IbfKernel;

void RunIBFKernel(sycl::queue& queue,
	sycl::buffer<MinimizerToIBFData, 1>& minimizerToIbf_buffer,
	sycl::buffer<Chunk, 1>& ibfData_buffer,
	const HostSizeType binSize,
	const HostSizeType hashShift,
	const HostSizeType numberOfQueries,
	const HostSizeType minimalNumberOfMinimizers,
	const HostSizeType maximalNumberOfMinimizers,
	sycl::buffer<HostSizeType, 1>& thresholds_buffer,
	sycl::buffer<Chunk, 1>& result_buffer)
{
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, NUMBER_OF_KERNELS>;

	fpga_tools::UnrolledLoop<NUMBER_OF_KERNELS>([&](auto id)
	{
		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor minimizerToIbf(minimizerToIbf_buffer, handler, sycl::read_only);

			handler.single_task<HostToKernelPipe>([=]() [[intel::kernel_args_restrict]]
			{
				for (size_t i = 0; i < minimizerToIbf.size(); i++)
				{
					MinimizerToIBFPipes::PipeAt<id>::write(minimizerToIbf[i]);
				}
			});
		});

		#include "kernel_ibf.cpp"

	});
}

} // namespace min_ibf_fpga::backend_sycl
