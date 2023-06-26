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
	sycl::buffer<_types::MinimizerToIBFData, 1>& minimizerToIbf_buffer,
	sycl::buffer<_types::Chunk, 1>& ibfData_buffer,
	const _types::HostSizeType binSize,
	const _types::HostSizeType hashShift,
	const _types::HostSizeType numberOfQueries,
	const _types::HostSizeType minimalNumberOfMinimizers,
	const _types::HostSizeType maximalNumberOfMinimizers,
	sycl::buffer<_types::HostSizeType, 1>& thresholds_buffer,
	sycl::buffer<_types::Chunk, 1>& result_buffer)
{
	using constants = _constants;
	using types = _types;
	using ibf_kernel_t = ibf_kernel<constants, types>;
	using MinimizerToIBFData = _types::MinimizerToIBFData;
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, constants::number_of_kernels>;

	fpga_tools::UnrolledLoop<constants::number_of_kernels>([&](auto id)
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
