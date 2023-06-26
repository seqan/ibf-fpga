namespace min_ibf_fpga::backend_sycl
{

template <typename constants, typename types>
struct ibf_kernel
{

using HostSizeType = typename types::HostSizeType;
using HostHash = typename types::HostHash;

static inline HostSizeType mapTo(const HostHash hash, const HostSizeType binSize)
{
	using DoubleHostSizeType = ac_int<constants::host_size_type_bits * 2, false>;

	return ((DoubleHostSizeType)hash * (DoubleHostSizeType)binSize) >> constants::host_size_type_bits;
}

static inline HostSizeType calculateBinIndex(HostHash hash,
	const unsigned char seedIndex, const HostSizeType hashShift, const HostSizeType binSize)
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

template <typename ibfData_ptr_t, typename get_threshold_fn_t, typename on_minimizer_fn_t, typename on_result_fn_t>
static void compute_ibf(
	ibfData_ptr_t const & ibfData,
	const typename types::HostSizeType binSize,
	const typename types::HostSizeType hashShift,
	get_threshold_fn_t && get_threshold_fn,
	on_minimizer_fn_t && on_minimizer_fn,
	on_result_fn_t && on_result_fn)
{
	using QueryIndex = typename types::QueryIndex;
	using Counter = typename types::Counter;
	using Chunk = typename types::Chunk;
	using HostSizeType = typename types::HostSizeType;
	using MinimizerToIBFData = typename types::MinimizerToIBFData;

	#define UNSAFELEN 9 // LD singlepump pump (7) + Arithmetic (1) + Store (1)
	#define II (UNSAFELEN - CHUNKS)

	#if II > 1
		Counter counters[constants::chunks][constants::chunk_bits]; // __attribute__((register))
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
		data = on_minimizer_fn();

		const QueryIndex localNumberOfHashes = numberOfHashes++;

		Counter threshold;

		if (data.isLastElement) {
			//threshold = getThreshold(localNumberOfHashes, minimalNumberOfMinimizers, maximalNumberOfMinimizers, thresholds);
			// manual inline of getThreshold() because of thresholds accessor
			// const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

			// HostSizeType index = localNumberOfHashes < minimalNumberOfMinimizers? 0 : localNumberOfHashes - minimalNumberOfMinimizers;
			// index = index < maximalIndex? index : maximalIndex;

			// threshold = (thresholds[static_cast<size_t>(index)] + 2).to_uint();
			threshold = get_threshold_fn(localNumberOfHashes);
		}

		HostSizeType binOffsets[constants::hash_count];

		#pragma unroll
		for (unsigned char seedIndex = 0; seedIndex < constants::hash_count; ++seedIndex)
			binOffsets[seedIndex] = calculateBinIndex(data.hash,
				seedIndex, hashShift, binSize) * constants::chunks_per_bin;

		for (unsigned char chunkIndex = 0; chunkIndex < constants::chunks; chunkIndex++)
		{
			Chunk bitvector = ~(Chunk)0;
			Chunk localResult = 0;

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

				localResult[bitOffset] = counter >= threshold;

				/*if (data.isLastElement)
					printf("%d %d\n", counter, threshold);*/
			}

			if (data.isLastElement)
			{
				on_result_fn(chunkIndex, localResult);
			}
		}

		countersInitialized = 1;
	}
	while(!data.isLastElement);
}

};

} // namespace min_ibf_fpga::backend_sycl