#include <sycl/sycl.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>

// Note: This header only contains code that can easily be shared between host and device.

namespace min_ibf_fpga::backend_sycl
{

#define HOST_SIZE_TYPE_BITS 64
#define MAX_BUS_WIDTH 512

using HostSizeType = ac_int<HOST_SIZE_TYPE_BITS, false>;

} // namespace min_ibf_fpga::backend_sycl
