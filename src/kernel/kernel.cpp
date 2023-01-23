#include <sycl/ext/intel/fpga_extensions.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>

#include "kernel.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Asserts
////////////////////////////////////////////////////////////////////////////////////////////////////

static_assert(WINDOW_SIZE >= K, "Window size needs to be greater or equal K-mer size");

static_assert(sizeof(seeds) / sizeof(HostHash) >= HASH_COUNT,
	"The number of hash functions must be smaller or equal the number of seeds");

static_assert(sizeof(HostHash) >= sizeof(MINIMIZER_SEED), "Minimizer seed doesn't fit Hash type");
static_assert(sizeof(HostHash) * 8 >= 2 * K, "K-mer doesn't fit Hash type");

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static Helper
////////////////////////////////////////////////////////////////////////////////////////////////////

template<class LHS, class RHS>
constexpr auto INTEGER_DIVISION_CEIL(const LHS lhs, const RHS rhs)
{
	return (lhs + rhs - 1) / rhs;
}

constexpr auto NUMBER_OF_KMERS_PER_WINDOW = WINDOW_SIZE - K + 1;

constexpr auto INITIALIZATION_ITERATIONS = (K - 1) + (NUMBER_OF_KMERS_PER_WINDOW - 1);

constexpr auto MINIMIZER_SEED_ADJUSTED = MINIMIZER_SEED >> (64 - 2 * K);

constexpr auto TECHNICAL_BIN_COUNT_WORDS = INTEGER_DIVISION_CEIL(BIN_COUNT, HOST_SIZE_TYPE_BITS);
constexpr auto TECHNICAL_BIN_COUNT = TECHNICAL_BIN_COUNT_WORDS * HOST_SIZE_TYPE_BITS;

constexpr auto CHUNK_BITS = TECHNICAL_BIN_COUNT > MAX_BUS_WIDTH? MAX_BUS_WIDTH : TECHNICAL_BIN_COUNT;
constexpr auto CHUNKS = TECHNICAL_BIN_COUNT > MAX_BUS_WIDTH? INTEGER_DIVISION_CEIL(TECHNICAL_BIN_COUNT, MAX_BUS_WIDTH) : 1;
constexpr auto CHUNKS_PER_BIN = INTEGER_DIVISION_CEIL(TECHNICAL_BIN_COUNT, MAX_BUS_WIDTH);

static_assert((bool)(TECHNICAL_BIN_COUNT < MAX_BUS_WIDTH)
	|| (TECHNICAL_BIN_COUNT % MAX_BUS_WIDTH == 0),
	"Bin count needs to be less or a multiple of max bus width");

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types & Conversions
////////////////////////////////////////////////////////////////////////////////////////////////////

using HostSizeType = ac_int<HOST_SIZE_TYPE_BITS, false>;

using Element = ac_int<2, false>;
using Hash = ac_int<2 * K, false>;

using QueryIndex = ac_int<25, true>;

using Minimizer = struct
{
	Hash hash;
	unsigned char position;
};

using Chunk = ac_int<CHUNK_BITS, false>;

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
		kmerComplement |= (Hash)(~value) << elementIndex * 2;
	}

	kmer ^= (Hash)MINIMIZER_SEED_ADJUSTED;
	kmerComplement ^= (Hash)MINIMIZER_SEED_ADJUSTED;

	return kmer < kmerComplement? kmer : kmerComplement;
}

Minimizer inline findMinimizer(const Hash* hashBuffer)
{
	Minimizer minimizer = {~(Hash)0, 0};

	#pragma unroll
	for (ushort kmerIndex = 0; kmerIndex < NUMBER_OF_KMERS_PER_WINDOW; ++kmerIndex)
	{
		const Minimizer current = {hashBuffer[kmerIndex], static_cast<unsigned char>(kmerIndex)};

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
}VectorAdd

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

// Forward declation of the kernel names. FPGA best practice to reduce compiler name mangling in the optimization reports.
// Disabled for now.
//class Minimizer;
//class IBF;

void RunKernel(sycl::queue& queue,
	sycl::buffer<char, 1>& querys_buffer,
	const HostSizeType queriesOffset,
	sycl::buffer<HostSizeType, 1>& querySizes_buffer,
	const HostSizeType querySizesOffset,
	const HostSizeType numberOfQuerys,
	sycl::buffer<Chunk, 1>& ibfData_buffer,
	const HostSizeType binSize,
	const HostSizeType hashShift,
	const HostSizeType minimalNumberOfMinimizers,
	const HostSizeType maximalNumberOfMinimizers,
	sycl::buffer<HostSizeType, 1>& thresholds_buffer,
	sycl::buffer<Chunk, 1>& result_buffer)
{
	using MinimizerToIBFData = struct //__attribute__((__packed__))
	{
		bool isLastElement;
		Hash hash;
	};

	// TODO: Add headers from https://github.com/oneapi-src/oneAPI-samples/tree/master/DirectProgramming/DPC%2B%2BFPGA/Tutorials/DesignPatterns/pipe_array
	using MinimizerToIBFPipes = PipeArray<class MinimizerToIBFPipe, MinimizerToIBFData, 25, NUMBER_OF_KERNELS>;

	Unroller<0, NUMBER_OF_KERNELS>::Step([&](auto id)
	{
		queue.submit([&](sycl::handler &handler)
		{
			sycl::accessor querys(querys_buffer, handler, sycl::read_only);
			sycl::accessor querySizes(querySizes_buffer, handler, sycl::read_only);

			h.single_task<Minimizer>([=]() [[intel::kernel_args_restrict]]
			{
				QueryIndex queryOffset = queriesOffset;

				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQuerys; queryIndex++)
				{
					const QueryIndex querySize = __prefetching_load(&querySizes[querySizesOffset + queryIndex]);

					const QueryIndex localQueryOffset = queryOffset;
					queryOffset += querySize;

					const QueryIndex iterations =
						INITIALIZATION_ITERATIONS // Fill query and hash buffer initially
						+ querySize - WINDOW_SIZE + 1;

					char queryBuffer[K];
					Hash hashBuffer[NUMBER_OF_KMERS_PER_WINDOW];

					// Set inital element's position to 0, so the first real element will never be skipped
					Minimizer lastMinimizer = {0, 0};

					for (QueryIndex iteration = 0; iteration <= iterations; iteration++)
					{
						// Shift register: Query buffer

						#pragma unroll
						for (unsigned char i = 0; i < K - 1; ++i)
							queryBuffer[i] = queryBuffer[i + 1];

						// Query as long as elements are left, then only do calculations (end phase)
						if (iteration < querySize)
							queryBuffer[K - 1] = __prefetching_load(&querys[localQueryOffset + iteration]);

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
							Data data;
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

			h.single_task<IBF>([=]() [[intel::kernel_args_restrict]]
			{
				for (QueryIndex queryIndex = 0; queryIndex < (QueryIndex)numberOfQuerys; queryIndex++)
				{
					#define UNSAFELEN 9 // LD singlepump pump (7) + Arithmetic (1) + Store (1)
					#define II (UNSAFELEN - CHUNKS)

					#if II > 1
						Counter __attribute__((register)) counters[CHUNKS][CHUNK_BITS];
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

					Data data;

					#if II <= 1
						#pragma ivdep array(counters)
					#endif
					do
					{
						data = MinimizerToIBFPipes::PipeAt<id>::read();

						const QueryIndex localNumberOfHashes = numberOfHashes++;

						Counter threshold;

						if (data.isLastElement)
							threshold = getThreshold(localNumberOfHashes,
								minimalNumberOfMinimizers, maximalNumberOfMinimizers, thresholds);

						HostSizeType binOffsets[HASH_COUNT];

						#pragma unroll
						for (unsigned char seedIndex = 0; seedIndex < HASH_COUNT; ++seedIndex)
							binOffsets[seedIndex] = calculateBinIndex(data.hash,
								seedIndex, hashShift, binSize) * CHUNKS_PER_BIN;

						for (char chunkIndex = 0; chunkIndex < CHUNKS; chunkIndex++)
						{
							Chunk bitvector = ~(Chunk)0;
							Chunk localResult = 0;

							// Unroll: Burst-coalesced over chunks per seed
							#pragma unroll
							for (unsigned char seedIndex = 0; seedIndex < HASH_COUNT; ++seedIndex)
								bitvector &= //__burst_coalesced_cached_load(
									/*&*/ibfData[binOffsets[seedIndex] + chunkIndex];//,
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
								result[queryIndex * CHUNKS + chunkIndex] = localResult;
						}

						countersInitialized = 1;
					}
					while(!data.isLastElement);
				}
			});
		});
	});
}
