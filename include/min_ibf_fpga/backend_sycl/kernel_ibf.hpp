namespace min_ibf_fpga::backend_sycl
{

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
	const HostSizeType* thresholds)
{
	const HostSizeType maximalIndex = maximalNumberOfMinimizers - minimalNumberOfMinimizers;

	HostSizeType index = numberOfHashes < minimalNumberOfMinimizers? 0 : numberOfHashes - minimalNumberOfMinimizers;
	index = index < maximalIndex? index : maximalIndex;

	return (thresholds[index] + 2).to_uint();
}

inline void printLocalResult(const uint64_t* localResult, sycl::stream out)
{
	for (HostSizeType i = 0; i < CHUNK_BITS / 64; i++)
	{
		uint64_t element = localResult[i];
		out << " " << element;
	}
	out << sycl::endl;
}

} // namespace min_ibf_fpga::backend_sycl