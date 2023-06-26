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
};

} // namespace min_ibf_fpga::backend_sycl