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
template <std::size_t id> class IbfKernel;
class Collector;

void RunIBFKernel(sycl::queue& queue,
	sycl::buffer<MinimizerToIBFData, 1>& minimizerToIbf_buffer,
	const Chunk* ibfData_ptr,
	const HostSizeType binSize,
	const HostSizeType hashShift,
	const HostSizeType numberOfQueries,
	const HostSizeType minimalNumberOfMinimizers,
	const HostSizeType maximalNumberOfMinimizers,
	const HostSizeType* thresholds_ptr,
	Chunk* result_ptr)
{
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, KERNEL_COPYS>;
	using CollectorPipes = fpga_tools::PipeArray<class CollectorPipe, Chunk, 25, KERNEL_COPYS>;

	fpga_tools::UnrolledLoop<KERNEL_COPYS>([&](auto id)
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

		const HostSizeType localNumberOfQueries = numberOfQueries;

		queue.submit([&](sycl::handler &handler)
		{
			#include "kernel_ibf.cpp"
		});

	});

	queue.submit([&](sycl::handler &handler)
	{
		#include "collector.cpp"
	});
}

} // namespace min_ibf_fpga::backend_sycl
