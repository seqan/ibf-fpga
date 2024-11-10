#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include "compute_units.hpp"
#include "pipe_utils.hpp"

// Kernel includes
#include "kernel.hpp"
#include "kernel_ibf.hpp"
#include "kernel_minimizer.hpp"


namespace min_ibf_fpga::backend_sycl
{

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
template <std::size_t id> class MinimizerKernel;
template <std::size_t id> class IbfKernel;

void RunKernel(sycl::queue& queue,
	const char* queries_ptr,
	const HostSizeType queryLength,
	const HostSizeType totalNumberOfQueries,
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

	const QueryIndex queriesPerKernel = totalNumberOfQueries / NUMBER_OF_KERNELS;

	SubmitComputeUnits<NUMBER_OF_KERNELS, MinimizerKernel>(queue, [=](auto id) [[intel::kernel_args_restrict]] {
		#include "kernel_minimizer.cpp"
	});

	SubmitComputeUnits<NUMBER_OF_KERNELS, IbfKernel>(queue, [=](auto id) [[intel::kernel_args_restrict]] {
		#include "kernel_ibf.cpp"
	});
}

} // namespace min_ibf_fpga::backend_sycl
