#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include "pipe_utils.hpp"
#include "unrolled_loop.hpp"

// Kernel includes
#include "kernel.hpp"
#include "kernel_ibf.hpp"
#include "kernel_minimizer.hpp"

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
class Minimizer_kernel;
class IBF_kernel;

void RunKernel(sycl::queue& queue,
	sycl::buffer<char, 1>& queries_buffer,
	const HostSizeType queriesOffset,
	sycl::buffer<HostSizeType, 1>& querySizes_buffer,
	const HostSizeType querySizesOffset,
	const HostSizeType numberOfQueries,
	sycl::buffer<Chunk, 1>& ibfData_buffer,
	const HostSizeType binSize,
	const HostSizeType hashShift,
	const HostSizeType minimalNumberOfMinimizers,
	const HostSizeType maximalNumberOfMinimizers,
	sycl::buffer<HostSizeType, 1>& thresholds_buffer,
	sycl::buffer<Chunk, 1>& result_buffer)
{
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, NUMBER_OF_KERNELS>;

	using PrefetchingLSU = sycl::ext::intel::lsu<sycl::ext::intel::prefetch<true>, sycl::ext::intel::statically_coalesce<false>>;

	fpga_tools::UnrolledLoop<NUMBER_OF_KERNELS>([&](auto id)
	{
		
	#include "kernel_minimizer.cpp"

	#include "kernel_ibf.cpp"

	});
}
