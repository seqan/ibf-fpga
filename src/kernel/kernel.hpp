#include <sycl/sycl.hpp>

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

#ifndef WINDOW_SIZE
	#define WINDOW_SIZE 23
#endif

#ifndef K
	#define K 20
#endif

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
// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace sycl;

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
	sycl::buffer<Chunk, 1>& result_buffer);
