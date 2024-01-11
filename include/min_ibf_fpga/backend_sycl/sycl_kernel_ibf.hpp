#pragma once

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>

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
	using HostHash = typename types::HostHash;

	using MinimizerToIBFData = typename types::MinimizerToIBFData;
	using MinimizerToIBFPipes = _MinimizerToIBFPipes;

	const Chunk* ibfData{};
	HostSizeType binSize{};
	HostSizeType hashShift{};
	HostSizeType numberOfQueries{};
	HostSizeType minimalNumberOfMinimizers{};
	HostSizeType maximalNumberOfMinimizers{};
	const HostSizeType* thresholds{};
	Chunk* result{};

	static inline HostSizeType mapTo(const HostHash hash, const HostSizeType binSize)
	{
		using DoubleHostSizeType = typename types::DoubleHostSizeType;

		return ((DoubleHostSizeType)hash * (DoubleHostSizeType)binSize) >> constants::host_size_type_bits;
	}

	static inline HostSizeType calculateBinIndex(HostHash hash, const unsigned char seedIndex, const HostSizeType hashShift, const HostSizeType binSize)
	{
		hash *= constants::seeds[seedIndex];
		hash ^= hash >> hashShift;
		hash *= 11400714819323198485u;

		return mapTo(hash, binSize);
	}

	// inline Counter getThreshold(const HostSizeType numberOfHashes,
	// 	const HostSizeType minimalNumberOfMinimizers,
	// 	const HostSizeType maximalNumberOfMinimizers,
	// 	HostSizeType* thresholds) //__global HostSizeType* restrict thresholds
	// {
	// 	const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

	// 	HostSizeType index = numberOfHashes < minimalNumberOfMinimizers?
	// 		0 : numberOfHashes - minimalNumberOfMinimizers;
	// 	index = index < maximalIndex? index : maximalIndex;

	// 	return (thresholds[index] + 2).to_uint();
	// }

	template <size_t pipe_id>
	void execute() const
	{
		sycl::device_ptr<const HostSizeType> thresholds_ptr(thresholds);
		sycl::device_ptr<const Chunk> ibfData_ptr(ibfData);
		sycl::device_ptr<Chunk> result_ptr(result);

		using pipe_t = typename MinimizerToIBFPipes::template PipeAt<pipe_id>;

		// TODO: adjust size
		[[intel::fpga_memory]] size_t idx_reg[100000];
		[[intel::fpga_memory]] Chunk result_reg[100000];

		[[intel::initiation_interval(1)]]
		for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
		{
			#define UNSAFELEN 9 // LD singlepump pump (7) + Arithmetic (1) + Store (1)
			#define II (UNSAFELEN - CHUNKS)

			#if II > 1
				Counter counters[constants::chunks][constants::chunk_bits]
		#if MIN_IBF_FPGA_INIT_EVERYTHING
					= {}
		#endif // MIN_IBF_FPGA_INIT_EVERYTHING
				; // __attribute__((register))
			#else
				Counter __attribute__((
						memory,
						numbanks(1),
						bankwidth(constants::chunk_bits * sizeof(Counter)),
						singlepump,//singlepump,
						max_replicates(1)))
					counters[constants::chunks][constants::chunk_bits];
			#endif

			bool countersInitialized = 0;

			QueryIndex numberOfHashes = 1;

			MinimizerToIBFData data;

			#if II <= 1
				#pragma ivdep array(counters)
			#endif
			do
			{
				data = pipe_t::read();

				const QueryIndex localNumberOfHashes = numberOfHashes++;

				Counter threshold;

		#if MIN_IBF_FPGA_INIT_EVERYTHING
				threshold = 0; // Without this the code below has UB on CPU
		#endif // MIN_IBF_FPGA_INIT_EVERYTHING

				if (data.isLastElement) {
					//threshold = getThreshold(localNumberOfHashes, minimalNumberOfMinimizers, maximalNumberOfMinimizers, thresholds);
					// manual inline of getThreshold() because of thresholds accessor
					const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

					HostSizeType index = localNumberOfHashes < minimalNumberOfMinimizers? 0 : localNumberOfHashes - minimalNumberOfMinimizers;
					index = index < maximalIndex? index : maximalIndex;

					threshold = (thresholds_ptr[static_cast<size_t>(index)] + 2).to_uint();
				}

				HostSizeType binOffsets[constants::hash_count]
		#if MIN_IBF_FPGA_INIT_EVERYTHING
					= {}
		#endif // MIN_IBF_FPGA_INIT_EVERYTHING
				;

				#pragma unroll
				for (unsigned char seedIndex = 0; seedIndex < constants::hash_count; ++seedIndex)
					binOffsets[seedIndex] = calculateBinIndex(data.hash,
						seedIndex, hashShift, binSize) * constants::chunks_per_bin;

				for (unsigned char chunkIndex = 0; chunkIndex < constants::chunks; chunkIndex++)
				{
					Chunk bitvector = ~(Chunk)0;
					Chunk localResult{0};

					// Unroll: Burst-coalesced over chunks per seed
					#pragma unroll
					for (unsigned char seedIndex = 0; seedIndex < constants::hash_count; ++seedIndex)
						bitvector &= //__burst_coalesced_cached_load(
							/*&*/ibfData[static_cast<size_t>(binOffsets[seedIndex]) + chunkIndex];//,
							//1048576); // 1 MiB = 8 megabit
							//65536); // 65536 byte = 512 kilobit (default)

					#pragma unroll
					for (ushort bitOffset = 0; bitOffset < constants::chunk_bits; ++bitOffset)
					{
						const Counter counter =
							// Avoid additional port for init
							(!countersInitialized? 0 : counters[chunkIndex][bitOffset])
							+ bitvector[bitOffset];

						counters[chunkIndex][bitOffset] = counter;

						// Note: threshold might be UNDEFINED if
						localResult[bitOffset] = counter >= threshold;

						/*if (data.isLastElement)
							printf("%d %d\n", counter, threshold);*/
					}

					if (data.isLastElement)
					{
						size_t result_idx = static_cast<size_t>(queryIndex) * constants::chunks + chunkIndex;

						idx_reg[queryIndex] = result_idx;
						result_reg[queryIndex] = localResult;
					}
				}

				countersInitialized = 1;
			}
			while(!data.isLastElement);
		}

		for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
		{
			result_ptr[idx_reg[queryIndex]] = result_reg[queryIndex];
		}
	}
};

} // namespace min_ibf_fpga::backend_sycl
