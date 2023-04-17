#include <sycl/sycl.hpp>
#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>

#include <min_ibf_fpga/backend_sycl/pipe_utils.hpp>
#include <min_ibf_fpga/backend_sycl/unrolled_loop.hpp>

// TODO: use <ibf_fpga/sycl/commen.hpp>
// - commen.hpp does not contain function declarations
#include "kernel.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Minimizer
////////////////////////////////////////////////////////////////////////////////////////////////////

Element inline translateCharacterToElement(const char character)
{
	return character == 'A'? 0
		: character == 'C'? 1
		: character == 'G'? 2
		: 3;
}

Hash inline extractHash(const char* buffer)
{
	Hash kmer = 0;
	Hash kmerComplement = 0;

	#pragma unroll
	for (unsigned char elementIndex = 0; elementIndex < K; ++elementIndex)
	{
		const Element value = translateCharacterToElement(buffer[elementIndex]);

		kmer |= (Hash)(value) << (2 * (K - 1) - elementIndex * 2);
		kmerComplement |= (Hash)((Element)~value) << elementIndex * 2;
	}

	kmer ^= (Hash)MINIMIZER_SEED_ADJUSTED;
	kmerComplement ^= (Hash)MINIMIZER_SEED_ADJUSTED;

	return kmer < kmerComplement? kmer : kmerComplement;
}

Minimizer inline findMinimizer(const Hash* hashBuffer)
{
	Minimizer minimizer = {~(Hash)0, 0};

	#pragma unroll
	for (unsigned char kmerIndex = 0; kmerIndex < NUMBER_OF_KMERS_PER_WINDOW; ++kmerIndex)
	{
		const Minimizer current = {hashBuffer[kmerIndex], kmerIndex};

		if (current.hash < minimizer.hash)
			minimizer = current;
	}

	return minimizer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// IBF
////////////////////////////////////////////////////////////////////////////////////////////////////

inline HostSizeType mapTo(const HostHash hash, const HostSizeType binSize)
{
	using DoubleHostSizeType = ac_int<HOST_SIZE_TYPE_BITS * 2, false>;

	return ((DoubleHostSizeType)hash * (DoubleHostSizeType)binSize) >> HOST_SIZE_TYPE_BITS;
}

inline HostSizeType calculateBinIndex(HostHash hash,
	const unsigned char seedIndex, const HostSizeType hashShift, const HostSizeType binSize)
{
	hash *= seeds[seedIndex];
	hash ^= hash >> hashShift;
	hash *= 11400714819323198485u;

	return mapTo(hash, binSize);
}

inline Counter getThreshold(const HostSizeType numberOfHashes,
	const HostSizeType minimalNumberOfMinimizers,
	const HostSizeType maximalNumberOfMinimizers,
	HostSizeType* thresholds) //__global HostSizeType* restrict thresholds
{
	const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

	HostSizeType index = numberOfHashes < minimalNumberOfMinimizers?
		0 : numberOfHashes - minimalNumberOfMinimizers;
	index = index < maximalIndex? index : maximalIndex;

	return (thresholds[index] + 2).to_uint();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main kernels
////////////////////////////////////////////////////////////////////////////////////////////////////

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
		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor queries(queries_buffer, handler, sycl::read_only);
			sycl::accessor querySizes(querySizes_buffer, handler, sycl::read_only);

			handler.single_task<Minimizer_kernel>([=]() [[intel::kernel_args_restrict]]
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

					const QueryIndex iterations =
						INITIALIZATION_ITERATIONS // Fill query and hash buffer initially
						+ querySize - WINDOW_SIZE + 1;

					char queryBuffer[K] = {0};
					Hash hashBuffer[NUMBER_OF_KMERS_PER_WINDOW] = {0};

					// Set initial element's position to 0, so the first real element will never be skipped
					Minimizer lastMinimizer = {0, 0};

					for (QueryIndex iteration = 0; iteration <= iterations; iteration++)
					{
						// Shift register: Query buffer
						#pragma unroll
						for (unsigned char i = 0; i < K - 1; ++i)
							queryBuffer[i] = queryBuffer[i + 1];

						// Query as long as elements are left, then only do calculations (end phase)
						if (iteration < querySize)
							queryBuffer[K - 1] = PrefetchingLSU::load(queries_ptr + static_cast<size_t>(localQueryOffset + iteration));

						// Shift register: hash buffer
						#pragma unroll
						for (ushort i = 0; i < NUMBER_OF_KMERS_PER_WINDOW - 1; ++i)
							hashBuffer[i] = hashBuffer[i + 1];

						hashBuffer[NUMBER_OF_KMERS_PER_WINDOW - 1] = extractHash(queryBuffer);

						const Minimizer minimizer = findMinimizer(hashBuffer);
						const Minimizer localLastMinimizer = lastMinimizer;

						if (iteration >= INITIALIZATION_ITERATIONS)
							// Update the lastMinimizer every time, because it is either new or the same anyways
							lastMinimizer = minimizer;

						const bool skipMinimizer = localLastMinimizer.position != 0
							&& localLastMinimizer.hash == minimizer.hash;
						const bool lastElement = iteration > iterations - 1;

						if (// Skip the first element (> instead of >=), as lastMinimizer is not initialized yet
							iteration > INITIALIZATION_ITERATIONS
							// Send the lastMinimizer when a new one is found or the last element is reached
							&& (!skipMinimizer || lastElement))
						{
							MinimizerToIBFData data;
							data.isLastElement = lastElement;
							data.hash = localLastMinimizer.hash;

							MinimizerToIBFPipes::PipeAt<id>::write(data);
						}
					}
				}
			});
		});

		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor ibfData(ibfData_buffer, handler, sycl::read_only);
			sycl::accessor thresholds(thresholds_buffer, handler, sycl::read_only);
			sycl::accessor result(result_buffer, handler, sycl::write_only);

			handler.single_task<IBF_kernel>([=]() [[intel::kernel_args_restrict]]
			{
				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQueries; queryIndex++)
				{
					#define UNSAFELEN 9 // LD singlepump pump (7) + Arithmetic (1) + Store (1)
					#define II (UNSAFELEN - CHUNKS)

					#if II > 1
						Counter counters[CHUNKS][CHUNK_BITS]; // __attribute__((register)) 
					#else
						Counter __attribute__((
								memory,
								numbanks(1),
								bankwidth(CHUNK_BITS * sizeof(Counter)),
								singlepump,//singlepump,
								max_replicates(1)))
							counters[CHUNKS][CHUNK_BITS];
					#endif
					bool countersInitialized = 0;

					QueryIndex numberOfHashes = 1;

					MinimizerToIBFData data;

					#if II <= 1
						#pragma ivdep array(counters)
					#endif
					do
					{
						data = MinimizerToIBFPipes::PipeAt<id>::read();

						const QueryIndex localNumberOfHashes = numberOfHashes++;

						Counter threshold;

						if (data.isLastElement) {
							//threshold = getThreshold(localNumberOfHashes, minimalNumberOfMinimizers, maximalNumberOfMinimizers, thresholds);
							// manual inline of getThreshold() because of thresholds accessor
							const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

							HostSizeType index = localNumberOfHashes < minimalNumberOfMinimizers? 0 : localNumberOfHashes - minimalNumberOfMinimizers;
							index = index < maximalIndex? index : maximalIndex;

							threshold = (thresholds[static_cast<size_t>(index)] + 2).to_uint();
						}

						HostSizeType binOffsets[HASH_COUNT];

						#pragma unroll
						for (unsigned char seedIndex = 0; seedIndex < HASH_COUNT; ++seedIndex)
							binOffsets[seedIndex] = calculateBinIndex(data.hash,
								seedIndex, hashShift, binSize) * CHUNKS_PER_BIN;

						for (unsigned char chunkIndex = 0; chunkIndex < CHUNKS; chunkIndex++)
						{
							Chunk bitvector = ~(Chunk)0;
							Chunk localResult = 0;

							// Unroll: Burst-coalesced over chunks per seed
							#pragma unroll
							for (unsigned char seedIndex = 0; seedIndex < HASH_COUNT; ++seedIndex)
								bitvector &= //__burst_coalesced_cached_load(
									/*&*/ibfData[static_cast<size_t>(binOffsets[seedIndex]) + chunkIndex];//,
									//1048576); // 1 MiB = 8 megabit
									//65536); // 65536 byte = 512 kilobit (default)

							#pragma unroll
							for (ushort bitOffset = 0; bitOffset < CHUNK_BITS; ++bitOffset)
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
								result[static_cast<size_t>(queryIndex * CHUNKS + chunkIndex)] = localResult;
						}

						countersInitialized = 1;
					}
					while(!data.isLastElement);
				}
			});
		});
	});
}
