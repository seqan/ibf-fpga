#pragma once

#include <iosfwd>
#include <vector>
#include <cstdint>

namespace min_ibf_fpga::io
{

template <typename value_t>
void write_out_binary_data(std::ostream & ostrm, std::vector<value_t> const & output_data)
{
  ostrm.write(reinterpret_cast<char const *>(output_data.data()), output_data.size() * sizeof(value_t));
}

} // namespace min_ibf_fpga::io