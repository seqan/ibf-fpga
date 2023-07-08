#pragma once

#include <sycl/sycl.hpp>
#include <sycl/ext/intel/ac_types/ac_int.hpp>

// Note: This header only contains code that can easily be shared between host and device.

#include <min_ibf_fpga/backend_sycl/min_ibf_constants.hpp>
#include <min_ibf_fpga/backend_sycl/min_ibf_types.hpp>

namespace min_ibf_fpga::backend_sycl
{

////////////////////////////////////////////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////////////////////////////////////////////

using _constants = min_ibf_constants<23, 19, 64>;
using _types = min_ibf_types<_constants, sycl_backend>;

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
