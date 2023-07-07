#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include "pipe_utils.hpp"
#include "unrolled_loop.hpp"

// Kernel includes
#include "kernel.hpp"
#include "kernel_ibf.hpp"
#include "kernel_minimizer.hpp"
#include "sycl_kernel_minimizer.hpp"


namespace min_ibf_fpga::backend_sycl
{

// Forward declaration of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
class MinimizerKernel;
class IbfKernel;

void RunKernel(sycl::queue& queue,
	sycl::buffer<char, 1>& queries_buffer,
	const _types::HostSizeType queriesOffset,
	sycl::buffer<_types::HostSizeType, 1>& querySizes_buffer,
	const _types::HostSizeType querySizesOffset,
	const _types::HostSizeType numberOfQueries,
	sycl::buffer<_types::Chunk, 1>& ibfData_buffer,
	const _types::HostSizeType binSize,
	const _types::HostSizeType hashShift,
	const _types::HostSizeType minimalNumberOfMinimizers,
	const _types::HostSizeType maximalNumberOfMinimizers,
	sycl::buffer<_types::HostSizeType, 1>& thresholds_buffer,
	sycl::buffer<_types::Chunk, 1>& result_buffer)
{
	using constants = _constants;
	using types = _types;
	using ibf_kernel_t = ibf_kernel<constants, types>;

	using sycl_minimizer_kernel_t = sycl_minimizer_kernel<constants, types>;
	using MinimizerToIBFData = types::MinimizerToIBFData;
	using MinimizerToIBFPipes = typename sycl_minimizer_kernel_t::MinimizerToIBFPipes;

	fpga_tools::UnrolledLoop<constants::number_of_kernels>([&](auto id)
	{

	#include "kernel_minimizer.cpp"

	#include "kernel_ibf.cpp"

	});
}

} // namespace min_ibf_fpga::backend_sycl
