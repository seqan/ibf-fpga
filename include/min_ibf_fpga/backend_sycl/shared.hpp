#pragma once

#if __INTEL_LLVM_COMPILER < 20230000
  #include <CL/sycl.hpp>
  #include <CL/sycl/INTEL/ac_types/ac_int.hpp>
#else
  #include <sycl/sycl.hpp>
  #include <sycl/ext/intel/ac_types/ac_int.hpp>
#endif

// Note: This header contains code that is shared between host and device.

#define INTEGER_DIVISION_CEIL(lhs, rhs) ((lhs + rhs - 1) / rhs)
#define MAX_QUERY_LENGTH 250
// Note: MAX_QUERY_LENGTH is an upper bound for THRESHOLDS_CACHE_SIZE
#define THRESHOLDS_CACHE_SIZE MAX_QUERY_LENGTH
#define HOST_SIZE_TYPE_BITS 64
#define MAX_BUS_WIDTH 512

namespace min_ibf_fpga::backend_sycl
{

using HostSizeType = ac_int<HOST_SIZE_TYPE_BITS, false>;

} // namespace min_ibf_fpga::backend_sycl
