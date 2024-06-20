#pragma once

#include "shared.hpp"

namespace min_ibf_fpga::backend_sycl
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////////////////////////

// Host types
typedef ulong HostHash;

// Internal types
typedef ushort Counter;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WINDOW_SIZE
	#error "WINDOW_SIZE not defined"
#endif

#ifndef MIN_IBF_K
	#error "MIN_IBF_K not defined"
#endif

#ifndef BIN_COUNT
	#error "BIN_COUNT not defined"
#endif

#ifndef HASH_COUNT
	#define HASH_COUNT 2
#endif

#ifndef NUMBER_OF_KERNELS
	#define NUMBER_OF_KERNELS 1
#endif

#define MINIMIZER_SEED 0x8F3F73B5CF1C9ADE

static const HostHash seeds[5] = {
	13572355802537770549u, // 2**64 / (e/2)
	13043817825332782213u, // 2**64 / sqrt(2)
	10650232656628343401u, // 2**64 / sqrt(3)
	16499269484942379435u, // 2**64 / (sqrt(5)/2)
	4893150838803335377u}; // 2**64 / (3*pi/5)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Asserts
////////////////////////////////////////////////////////////////////////////////////////////////////

static_assert(sizeof(seeds) / sizeof(HostHash) >= HASH_COUNT, "The number of hash functions must be smaller or equal the number of seeds");

static_assert(sizeof(HostHash) >= sizeof(MINIMIZER_SEED), "Minimizer seed doesn't fit Hash type");

static_assert(WINDOW_SIZE > MIN_IBF_K, "Window size needs to be greater than K-mer size");

static_assert(sizeof(HostHash) * 8 >= 2 * MIN_IBF_K, "K-mer doesn't fit Hash type");

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static Helper
////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr auto NUMBER_OF_KMERS_PER_WINDOW = WINDOW_SIZE - MIN_IBF_K + 1;

constexpr auto INITIALIZATION_ITERATIONS = (MIN_IBF_K - 1) + (NUMBER_OF_KMERS_PER_WINDOW - 1);

constexpr auto MINIMIZER_SEED_ADJUSTED = MINIMIZER_SEED >> (64 - 2 * MIN_IBF_K);

constexpr auto TECHNICAL_BIN_COUNT_WORDS = INTEGER_DIVISION_CEIL(BIN_COUNT, HOST_SIZE_TYPE_BITS);
constexpr auto TECHNICAL_BIN_COUNT = TECHNICAL_BIN_COUNT_WORDS * HOST_SIZE_TYPE_BITS;

constexpr auto CHUNK_BITS = TECHNICAL_BIN_COUNT > MAX_BUS_WIDTH? MAX_BUS_WIDTH : TECHNICAL_BIN_COUNT;
constexpr auto CHUNKS = TECHNICAL_BIN_COUNT > MAX_BUS_WIDTH? INTEGER_DIVISION_CEIL(TECHNICAL_BIN_COUNT, MAX_BUS_WIDTH) : 1;
constexpr auto CHUNKS_PER_BIN = INTEGER_DIVISION_CEIL(TECHNICAL_BIN_COUNT, MAX_BUS_WIDTH);

static_assert((bool)(TECHNICAL_BIN_COUNT < MAX_BUS_WIDTH) || (TECHNICAL_BIN_COUNT % MAX_BUS_WIDTH == 0), "Bin count needs to be less or a multiple of max bus width");

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types & Conversions
////////////////////////////////////////////////////////////////////////////////////////////////////

using Hash = ac_int<2 * MIN_IBF_K, false>;

using Element = ac_int<2, false>;

using QueryIndex = ac_int<25, true>;

using Chunk = ac_int<CHUNK_BITS, false>;

using Minimizer = struct
{
	Hash hash;
	unsigned char position;
};

using MinimizerToIBFData = struct //__attribute__((__packed__))
{
	bool isLastElement;
	Hash hash;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{
	void RunKernel(sycl::queue& queue,
		const char* queries_ptr,
		const HostSizeType queriesOffset,
		const HostSizeType* querySizes_ptr,
		const HostSizeType querySizesOffset,
		const HostSizeType numberOfQueries,
		const Chunk* ibfData_ptr,
		const HostSizeType binSize,
		const HostSizeType hashShift,
		const HostSizeType minimalNumberOfMinimizers,
		const HostSizeType maximalNumberOfMinimizers,
		const HostSizeType* thresholds_ptr,
		Chunk* result_ptr,
		std::pair<sycl::event, sycl::event>* kernelEvents);
}

} // namespace min_ibf_fpga::backend_sycl
