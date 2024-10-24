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

void RunKernel(sycl::queue& queue,
	const char* queries_ptr,
	const HostSizeType* querySizes_ptr,
	const HostSizeType numberOfQueries,
	const Chunk* ibfData_ptr,
	const HostSizeType binSize,
	const HostSizeType hashShift,
	const HostSizeType minimalNumberOfMinimizers,
	const HostSizeType maximalNumberOfMinimizers,
	const HostSizeType* thresholds_ptr,
	Chunk* result_ptr,
	std::vector<sycl::event>& kernelEvents)
{
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, NUMBER_OF_KERNELS>;

	using PrefetchingLSU = sycl::ext::intel::lsu<sycl::ext::intel::prefetch<true>, sycl::ext::intel::statically_coalesce<false>>;

	fpga_tools::UnrolledLoop<NUMBER_OF_KERNELS>([&](auto id)
	{
		kernelEvents.push_back( queue.submit([&](sycl::handler &handler)
		{
			#include "kernel_minimizer.cpp"
		}) );

		kernelEvents.push_back( queue.submit([&](sycl::handler &handler)
		{
			#include "kernel_ibf.cpp"
		}) );
	});
}

} // namespace min_ibf_fpga::backend_sycl
