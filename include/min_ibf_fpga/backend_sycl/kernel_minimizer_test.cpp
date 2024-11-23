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
class Distributor;
template <std::size_t id> class MinimizerKernel;
class PipeToHostKernel;

void RunMinimizerKernel(sycl::queue& queue,
	const char* queries_ptr,
	const HostSizeType queriesOffset,
	const HostSizeType* querySizes_ptr,
	const HostSizeType querySizesOffset,
	const HostSizeType numberOfQueries,
	sycl::buffer<MinimizerToIBFData, 1>& minimizerToIbf_buffer)
{
	using DistributorPipes = fpga_tools::PipeArray<class DistributorPipe, DistributorToMinimizerData, 2, KERNEL_COPYS>;
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, KERNEL_COPYS>;

	using PrefetchingLSU = sycl::ext::intel::lsu<sycl::ext::intel::prefetch<true>, sycl::ext::intel::statically_coalesce<false>>;

	queue.submit([&](sycl::handler &handler)
	{
		#include "distributor.cpp"
	});

	fpga_tools::UnrolledLoop<KERNEL_COPYS>([&](auto id)
	{
		const HostSizeType localNumberOfQueries = numberOfQueries;

		queue.submit([&](sycl::handler &handler)
		{
			#include "kernel_minimizer.cpp"
		});

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
