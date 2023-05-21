#include <sycl/sycl.hpp>

// Note: This header only contains code that can easily be shared between host and device.

namespace min_ibf_fpga::backend_sycl
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////////////////////////

// Host types
#define HOST_SIZE_TYPE_BITS 64
typedef ulong HostHash;

// Internal types
typedef ushort Counter;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef HASH_COUNT
	#define HASH_COUNT 2
#endif

#ifndef BIN_COUNT
	#define BIN_COUNT 64
#endif

#define MINIMIZER_SEED 0x8F3F73B5CF1C9ADE

static const HostHash seeds[5] = {
	13572355802537770549u, // 2**64 / (e/2)
	13043817825332782213u, // 2**64 / sqrt(2)
	10650232656628343401u, // 2**64 / sqrt(3)
	16499269484942379435u, // 2**64 / (sqrt(5)/2)
	4893150838803335377u}; // 2**64 / (3*pi/5)

#define MAX_BUS_WIDTH 512

#ifndef NUMBER_OF_KERNELS
	#define NUMBER_OF_KERNELS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Asserts
////////////////////////////////////////////////////////////////////////////////////////////////////

static_assert(sizeof(seeds) / sizeof(HostHash) >= HASH_COUNT, "The number of hash functions must be smaller or equal the number of seeds");

static_assert(sizeof(HostHash) >= sizeof(MINIMIZER_SEED), "Minimizer seed doesn't fit Hash type");

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static Helper
////////////////////////////////////////////////////////////////////////////////////////////////////

template<class LHS, class RHS>
constexpr auto INTEGER_DIVISION_CEIL(const LHS lhs, const RHS rhs)
{
	return (lhs + rhs - 1) / rhs;
}

constexpr auto TECHNICAL_BIN_COUNT_WORDS = INTEGER_DIVISION_CEIL(BIN_COUNT, HOST_SIZE_TYPE_BITS);
constexpr auto TECHNICAL_BIN_COUNT = TECHNICAL_BIN_COUNT_WORDS * HOST_SIZE_TYPE_BITS;

constexpr auto CHUNK_BITS = TECHNICAL_BIN_COUNT > MAX_BUS_WIDTH? MAX_BUS_WIDTH : TECHNICAL_BIN_COUNT;
constexpr auto CHUNKS = TECHNICAL_BIN_COUNT > MAX_BUS_WIDTH? INTEGER_DIVISION_CEIL(TECHNICAL_BIN_COUNT, MAX_BUS_WIDTH) : 1;
constexpr auto CHUNKS_PER_BIN = INTEGER_DIVISION_CEIL(TECHNICAL_BIN_COUNT, MAX_BUS_WIDTH);

static_assert((bool)(TECHNICAL_BIN_COUNT < MAX_BUS_WIDTH) || (TECHNICAL_BIN_COUNT % MAX_BUS_WIDTH == 0), "Bin count needs to be less or a multiple of max bus width");

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types & Conversions
////////////////////////////////////////////////////////////////////////////////////////////////////

using HostSizeType = unsigned _BitInt(HOST_SIZE_TYPE_BITS);

using Element = unsigned _BitInt(2);

using QueryIndex = _BitInt(25);

using Chunk = std::bitset<CHUNK_BITS>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

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
       sycl::buffer<Chunk, 1>& result_buffer);

} // namespace min_ibf_fpga::backend_sycl
