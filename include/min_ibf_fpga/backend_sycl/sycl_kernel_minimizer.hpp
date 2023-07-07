#pragma once

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

#include <min_ibf_fpga/backend_sycl/kernel_minimizer.hpp>

namespace min_ibf_fpga::backend_sycl
{

template <typename _constants, typename _types>
struct sycl_minimizer_kernel
{
	using types = _types;
	using constants = _constants;

	using HostSizeType = typename types::HostSizeType;
	using QueryIndex = typename types::QueryIndex;
	using minimizer_kernel_t = minimizer_kernel<constants, types>;

	using PrefetchingLSU = sycl::ext::intel::lsu<sycl::ext::intel::prefetch<true>, sycl::ext::intel::statically_coalesce<false>>;

	using MinimizerToIBFData = typename types::MinimizerToIBFData;
	using MinimizerToIBFPipes = fpga_tools::PipeArray<sycl_minimizer_kernel, MinimizerToIBFData, 25, constants::number_of_kernels>;

	sycl::accessor<char, 1, sycl::access_mode::read> queries;
	HostSizeType queriesOffset{};
	sycl::accessor<HostSizeType, 1, sycl::access_mode::read> querySizes;
	HostSizeType querySizesOffset{};
	HostSizeType numberOfQueries{};

	template <size_t pipe_id>
	void execute() const
	{
		// Prefetching requires pointer
		auto querySizes_ptr = querySizes.get_pointer();
		auto queries_ptr = queries.get_pointer();

		QueryIndex queryOffset = queriesOffset;

		for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
		{
			const QueryIndex querySize = PrefetchingLSU::load(querySizes_ptr + static_cast<size_t>(querySizesOffset) + static_cast<size_t>(queryIndex));

			const QueryIndex localQueryOffset = queryOffset;
			queryOffset += querySize;

			minimizer_kernel_t::compute_minimizer(querySize, [&](QueryIndex const iteration){
				return PrefetchingLSU::load(queries_ptr + static_cast<size_t>(localQueryOffset + iteration));
			}, [&](auto && data)
			{
				using pipe_t = typename MinimizerToIBFPipes::template PipeAt<pipe_id>;
				pipe_t::write(std::forward<decltype(data)>(data));
			});
		}
	}
};

} // namespace min_ibf_fpga::backend_sycl