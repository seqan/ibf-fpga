#include <sycl/sycl.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>

// Note: This header only contains code that can easily be shared between host and device.

#include <min_ibf_fpga/backend_sycl/min_ibf_constants.hpp>
#include <min_ibf_fpga/backend_sycl/min_ibf_types.hpp>

namespace min_ibf_fpga::backend_sycl
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////////////////////////

// Host types
// #define HOST_SIZE_TYPE_BITS 64
#define HOST_SIZE_TYPE_BITS HOST_SIZE_TYPE_BITS_macro
// typedef ulong HostHash;

// Internal types
// typedef ushort Counter;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef HASH_COUNT
	// #define HASH_COUNT 2
	#define HASH_COUNT hash_count_macro
#endif

#ifndef BIN_COUNT
	// #define BIN_COUNT 64
	#define BIN_COUNT bin_count_macro
#endif

// #define MINIMIZER_SEED 0x8F3F73B5CF1C9ADE
#define MINIMIZER_SEED minimizer_seed_macro

// static const HostHash seeds[5] = {
// 	13572355802537770549u, // 2**64 / (e/2)
// 	13043817825332782213u, // 2**64 / sqrt(2)
// 	10650232656628343401u, // 2**64 / sqrt(3)
// 	16499269484942379435u, // 2**64 / (sqrt(5)/2)
// 	4893150838803335377u}; // 2**64 / (3*pi/5)

// #define MAX_BUS_WIDTH 512
#define MAX_BUS_WIDTH MAX_BUS_WIDTH_macro

#ifndef NUMBER_OF_KERNELS
	// #define NUMBER_OF_KERNELS 1
	#define NUMBER_OF_KERNELS NUMBER_OF_KERNELS_macro
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Asserts
////////////////////////////////////////////////////////////////////////////////////////////////////

// static_assert(sizeof(seeds) / sizeof(HostHash) >= HASH_COUNT, "The number of hash functions must be smaller or equal the number of seeds");

// static_assert(sizeof(HostHash) >= sizeof(MINIMIZER_SEED), "Minimizer seed doesn't fit Hash type");

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static Helper
////////////////////////////////////////////////////////////////////////////////////////////////////

// template<class LHS, class RHS>
// constexpr auto INTEGER_DIVISION_CEIL(const LHS lhs, const RHS rhs)
// {
// 	return (lhs + rhs - 1) / rhs;
// }

// constexpr auto TECHNICAL_BIN_COUNT_WORDS = TECHNICAL_BIN_COUNT_WORDS_macro;
// constexpr auto TECHNICAL_BIN_COUNT = TECHNICAL_BIN_COUNT_macro;

// constexpr auto CHUNK_BITS = CHUNK_BITS_macro;
// constexpr auto CHUNKS = CHUNKS_macro;
// constexpr auto CHUNKS_PER_BIN = CHUNKS_PER_BIN_macro;

// constexpr auto TECHNICAL_BIN_COUNT_WORDS = INTEGER_DIVISION_CEIL(BIN_COUNT, HOST_SIZE_TYPE_BITS);
// constexpr auto TECHNICAL_BIN_COUNT = TECHNICAL_BIN_COUNT_WORDS * HOST_SIZE_TYPE_BITS;

// constexpr auto CHUNK_BITS = TECHNICAL_BIN_COUNT > MAX_BUS_WIDTH? MAX_BUS_WIDTH : TECHNICAL_BIN_COUNT;
// constexpr auto CHUNKS = TECHNICAL_BIN_COUNT > MAX_BUS_WIDTH? INTEGER_DIVISION_CEIL(TECHNICAL_BIN_COUNT, MAX_BUS_WIDTH) : 1;
// constexpr auto CHUNKS_PER_BIN = INTEGER_DIVISION_CEIL(TECHNICAL_BIN_COUNT, MAX_BUS_WIDTH);

// static_assert((bool)(TECHNICAL_BIN_COUNT < MAX_BUS_WIDTH) || (TECHNICAL_BIN_COUNT % MAX_BUS_WIDTH == 0), "Bin count needs to be less or a multiple of max bus width");

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types & Conversions
////////////////////////////////////////////////////////////////////////////////////////////////////

// using HostSizeType = ac_int<HOST_SIZE_TYPE_BITS, false>;

// using Element = ac_int<2, false>;

// using QueryIndex = ac_int<25, true>;

// using Chunk = ac_int<CHUNK_BITS, false>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

using _constants = min_ibf_constants<23, 19, 64>;
using _types = min_ibf_types<_constants, sycl_backend>;
// using HostHash = typename _types::HostHash;
// using HostSizeType = typename _types::HostSizeType;
// using Element = typename _types::Element;
// using QueryIndex = typename _types::QueryIndex;
// using Chunk = typename _types::Chunk;

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
       sycl::buffer<_types::Chunk, 1>& result_buffer);

} // namespace min_ibf_fpga::backend_sycl
