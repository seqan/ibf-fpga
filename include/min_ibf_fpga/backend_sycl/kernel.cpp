#include <sycl/ext/intel/fpga_extensions.hpp>

// Utilities
#include "pipe_utils.hpp"
#include "unrolled_loop.hpp"

// Kernel includes
#include "kernel.hpp"
#include "kernel_ibf.hpp"
#include "kernel_minimizer.hpp"
#include "sycl_kernel_ibf.hpp"
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

	using sycl_minimizer_kernel_t = sycl_minimizer_kernel<constants, types>;
	using MinimizerToIBFPipes = typename sycl_minimizer_kernel_t::MinimizerToIBFPipes;
	using sycl_ibf_kernel_t = sycl_ibf_kernel<MinimizerToIBFPipes, constants, types>;

	fpga_tools::UnrolledLoop<constants::number_of_kernels>([&](auto id)
	{
		static constexpr size_t pipe_id = id;

		queue.submit([&](sycl::handler &handler)
		{
			sycl_minimizer_kernel_t minimizer_kernel{
				.queries{queries_buffer, handler, sycl::read_only},
				.queriesOffset{queriesOffset},
				.querySizes{querySizes_buffer, handler, sycl::read_only},
				.querySizesOffset{querySizesOffset},
				.numberOfQueries{numberOfQueries}
			};
			handler.single_task<MinimizerKernel>([minimizer_kernel]() [[intel::kernel_args_restrict]]
			{
				minimizer_kernel.execute<pipe_id>();
			});
		});

		queue.submit([&](sycl::handler &handler)
		{
			sycl_ibf_kernel_t ibf_kernel{
				.ibfData{ibfData_buffer, handler, sycl::read_only},
				.binSize{binSize},
				.hashShift{hashShift},
				.numberOfQueries{numberOfQueries},
				.minimalNumberOfMinimizers{minimalNumberOfMinimizers},
				.maximalNumberOfMinimizers{maximalNumberOfMinimizers},
				.thresholds{thresholds_buffer, handler, sycl::read_only},
				.result{result_buffer, handler, sycl::write_only},
			};
			handler.single_task<IbfKernel>([ibf_kernel]() [[intel::kernel_args_restrict]]
			{
				ibf_kernel.execute<pipe_id>();
			});
		});

	});
}

} // namespace min_ibf_fpga::backend_sycl
