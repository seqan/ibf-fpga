#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include "pipe_utils.hpp"
#include "unrolled_loop.hpp"

// Kernel includes
#include "kernel.hpp"
#include "kernel_ibf.hpp"
#include "kernel_minimizer.hpp"


namespace min_ibf_fpga::backend_sycl
{

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
class MinimizerKernel;
class IbfKernel;

std::pair<sycl::event, sycl::event> RunKernel(sycl::queue& queue,
	const char* queries_ptr,
	const HostSizeType queriesOffset,
	const HostSizeType* querySizes_ptr,
	const HostSizeType querySizesOffset,
	const HostSizeType numberOfQueries,
	const Chunk* ibfData_ptr,
	const HostSizeType binSize,
	const HostSizeType hashShift,
	const HostSizeType minimalNumberOfMinimizers,
	const HostSizeType maximalNumberOfMinimizers,
	const HostSizeType* thresholds_ptr,
	Chunk* result_ptr,
	std::vector<sycl::event>* kernelDependencies)
{
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, NUMBER_OF_KERNELS>;

	using PrefetchingLSU = sycl::ext::intel::lsu<sycl::ext::intel::prefetch<true>, sycl::ext::intel::statically_coalesce<false>>;

	std::pair<sycl::event, sycl::event> events;

	fpga_tools::UnrolledLoop<NUMBER_OF_KERNELS>([&](auto id)
	{
		events.first = queue.submit([&](sycl::handler &handler)
		{
			#include "kernel_minimizer.cpp"
		});

		events.second = queue.submit([&](sycl::handler &handler)
		{
			#include "kernel_ibf.cpp"
		});
	});

	return events;
}

} // namespace min_ibf_fpga::backend_sycl
