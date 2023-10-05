#pragma once

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

#include <min_ibf_fpga/backend_sycl/kernel_ibf.hpp>

namespace min_ibf_fpga::backend_sycl
{

template <typename _MinimizerToIBFPipes, typename _constants, typename _types>
struct sycl_ibf_kernel
{
	using types = _types;
	using constants = _constants;

	using QueryIndex = typename types::QueryIndex;
	using HostSizeType = typename types::HostSizeType;
	using Counter = typename types::Counter;
	using Chunk = typename types::Chunk;
	using Hash = typename types::Hash;

	using ibf_kernel_t = ibf_kernel<constants, types>;

	using MinimizerToIBFData = typename types::MinimizerToIBFData;
	using MinimizerToIBFPipes = _MinimizerToIBFPipes;

	const Chunk* ibfData{};
	HostSizeType binSize{};
	HostSizeType hashShift{};
	HostSizeType numberOfQueries{};
	HostSizeType minimalNumberOfMinimizers{};
	HostSizeType maximalNumberOfMinimizers{};
	const HostSizeType* thresholds{};
	sycl::accessor<Chunk, 1, sycl::access_mode::write> result;

	template <size_t pipe_id>
	void execute() const
	{
		sycl::device_ptr<const HostSizeType> thresholds_ptr(thresholds);
		sycl::device_ptr<const Chunk> ibfData_ptr(ibfData);

		for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
		{
			ibf_kernel_t::compute_ibf(
				ibfData_ptr,
				binSize,
				hashShift,
			[&](const QueryIndex localNumberOfHashes){
				//threshold = getThreshold(localNumberOfHashes, minimalNumberOfMinimizers, maximalNumberOfMinimizers, thresholds);
				// manual inline of getThreshold() because of thresholds accessor
				const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

				HostSizeType index = localNumberOfHashes < minimalNumberOfMinimizers? 0 : localNumberOfHashes - minimalNumberOfMinimizers;
				index = index < maximalIndex? index : maximalIndex;

				Counter threshold = (thresholds_ptr[static_cast<size_t>(index)] + 2).to_uint();
				return threshold;
			}, [&]() -> MinimizerToIBFData {
				using pipe_t = typename MinimizerToIBFPipes::template PipeAt<pipe_id>;
				return pipe_t::read();
			}, [&](size_t const chunkIndex, Chunk const & localResult) {
				size_t result_idx = (size_t)queryIndex * constants::chunks + chunkIndex;
				result[result_idx] = localResult;
			}, [&](unsigned char const chunk_idx, Hash const & minimizer, Chunk const & minimizer_membership) {
				// no-op this is for debugging
			}, [&](unsigned char const chunk_idx, Counter const * counters, size_t const counters_size) {
				// no-op this is for debugging
			});
		}
	}
};

} // namespace min_ibf_fpga::backend_sycl