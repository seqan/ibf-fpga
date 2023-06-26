#include "shared.hpp"

// Note: This header only contains code that depends of WINDOW_SIZE and MIN_IBF_K. All other code is in shared.hpp.

namespace min_ibf_fpga::backend_sycl
{

#ifndef WINDOW_SIZE
	#define WINDOW_SIZE 23
#endif

#ifndef MIN_IBF_K
	#define MIN_IBF_K 19
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Asserts
////////////////////////////////////////////////////////////////////////////////////////////////////

// static_assert(WINDOW_SIZE >= MIN_IBF_K, "Window size needs to be greater or equal K-mer size");
// static_assert(sizeof(HostHash) * 8 >= 2 * MIN_IBF_K, "K-mer doesn't fit Hash type");

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static Helper
////////////////////////////////////////////////////////////////////////////////////////////////////

// constexpr auto NUMBER_OF_KMERS_PER_WINDOW = WINDOW_SIZE - MIN_IBF_K + 1;

// constexpr auto INITIALIZATION_ITERATIONS = (MIN_IBF_K - 1) + (NUMBER_OF_KMERS_PER_WINDOW - 1);

// constexpr auto MINIMIZER_SEED_ADJUSTED = MINIMIZER_SEED >> (64 - 2 * MIN_IBF_K);

////////////////////////////////////////////////////////////////////////////////////////////////////
// Types & Conversions
////////////////////////////////////////////////////////////////////////////////////////////////////

// using Hash = ac_int<2 * MIN_IBF_K, false>;

// using Minimizer = struct
// {
// 	Hash hash;
// 	unsigned char position;
// };

// using MinimizerToIBFData = struct //__attribute__((__packed__))
// {
// 	bool isLastElement;
// 	Hash hash;
// };

} // namespace min_ibf_fpga::backend_sycl
