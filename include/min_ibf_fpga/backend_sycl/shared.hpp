#pragma once

#if __INTEL_LLVM_COMPILER < 20230000
  #include <CL/sycl.hpp>
  #include <CL/sycl/INTEL/ac_types/ac_int.hpp>
#else
  #include <sycl/sycl.hpp>
  #include <sycl/ext/intel/ac_types/ac_int.hpp>
#endif

// Note: This header only contains code that can easily be shared between host and device.

template<class LHS, class RHS>
constexpr auto INTEGER_DIVISION_CEIL(const LHS lhs, const RHS rhs)
{
	return (lhs + rhs - 1) / rhs;
}

namespace min_ibf_fpga::backend_sycl
{

#define HOST_SIZE_TYPE_BITS 64
#define MAX_BUS_WIDTH 512

using HostSizeType = ac_int<HOST_SIZE_TYPE_BITS, false>;

} // namespace min_ibf_fpga::backend_sycl
