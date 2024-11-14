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
class Distributor;
template <std::size_t id> class MinimizerKernel;
template <std::size_t id> class IbfKernel;
class Collector;

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
	using DistributorPipes = fpga_tools::PipeArray<class DistributorPipe, DistributorToMinimizerData, 2, NUMBER_OF_KERNELS>;
	using MinimizerToIBFPipes = fpga_tools::PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, NUMBER_OF_KERNELS>;
	using CollectorPipes = fpga_tools::PipeArray<class CollectorPipe, Chunk, 25, NUMBER_OF_KERNELS>;

	using PrefetchingLSU = sycl::ext::intel::lsu<sycl::ext::intel::prefetch<true>, sycl::ext::intel::statically_coalesce<false>>;

	kernelEvents.push_back( queue.submit([&](sycl::handler &handler)
	{
		handler.single_task<Distributor>([=]() [[intel::kernel_args_restrict]]
		{
			sycl::ext::intel::host_ptr<const char> queries_ptr_casted(queries_ptr);
			sycl::ext::intel::host_ptr<const HostSizeType> querySizes_ptr_casted(querySizes_ptr);

			size_t queries_ptr_index = 0;

			for (size_t i = 0; i < numberOfQueries; i++)
			{
				DistributorToMinimizerData data;
				data.size = PrefetchingLSU::load(querySizes_ptr_casted + i);

				for (size_t j = 0; j < data.size; j++)
				{
					data.query[j] = PrefetchingLSU::load(queries_ptr_casted + queries_ptr_index);
					queries_ptr_index++;
				}

				switch (i % NUMBER_OF_KERNELS)
				{
				case 0:
					DistributorPipes::PipeAt<0>::write(data);
					break;
#if NUMBER_OF_KERNELS > 1
				case 1:
					DistributorPipes::PipeAt<1>::write(data);
					break;
#endif
#if NUMBER_OF_KERNELS > 2
				case 2:
					DistributorPipes::PipeAt<2>::write(data);
					break;
#endif
#if NUMBER_OF_KERNELS > 3
				case 3:
					DistributorPipes::PipeAt<3>::write(data);
					break;
#endif
				}
			}
		});
	}) );

	fpga_tools::UnrolledLoop<NUMBER_OF_KERNELS>([&](auto id)
	{
		QueryIndex localNumberOfQueries = numberOfQueries / NUMBER_OF_KERNELS;
		QueryIndex remainder = numberOfQueries % NUMBER_OF_KERNELS;

		if (remainder > id) localNumberOfQueries++;

		kernelEvents.push_back( queue.submit([&](sycl::handler &handler)
		{
			#include "kernel_minimizer.cpp"
		}) );

		kernelEvents.push_back( queue.submit([&](sycl::handler &handler)
		{
			#include "kernel_ibf.cpp"
		}) );
	});

	kernelEvents.push_back( queue.submit([&](sycl::handler &handler)
	{
		handler.single_task<Collector>([=]() [[intel::kernel_args_restrict]]
		{
			sycl::ext::intel::host_ptr<Chunk> result_ptr_casted(result_ptr);

			for (QueryIndex queryIndex = 0; queryIndex < static_cast<QueryIndex>(numberOfQueries); queryIndex++)
			{
				unsigned char pipeIndex = queryIndex % NUMBER_OF_KERNELS;

				for (unsigned char chunkIndex = 0; chunkIndex < CHUNKS; chunkIndex++)
				{
					Chunk chunk;

					switch (pipeIndex)
					{
					case 0:
						chunk = CollectorPipes::PipeAt<0>::read();
						break;
#if NUMBER_OF_KERNELS > 1
					case 1:
						chunk = CollectorPipes::PipeAt<1>::read();
						break;
#endif
#if NUMBER_OF_KERNELS > 2
					case 2:
						chunk = CollectorPipes::PipeAt<2>::read();
						break;
#endif
#if NUMBER_OF_KERNELS > 3
					case 3:
						chunk = CollectorPipes::PipeAt<3>::read();
						break;
#endif
					}

					result_ptr_casted[static_cast<size_t>(queryIndex * CHUNKS + chunkIndex)] = chunk;
				}
			}
		});
	}) );
}

} // namespace min_ibf_fpga::backend_sycl
