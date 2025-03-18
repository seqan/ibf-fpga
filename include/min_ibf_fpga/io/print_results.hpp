#pragma once

#include <iosfwd>
#include <string>
#include <vector>
#include <cstdint>

namespace min_ibf_fpga::io
{

// TODO: handle longer bit_results
// means sizeof(chunk_t) > sizeof(uint64_t)
template <typename chunk_t>
void print_results(std::vector<std::string> const & ids, std::vector<chunk_t> const & results, std::ostream & cout)
{
  static_assert(sizeof(chunk_t) <= sizeof(uint64_t));

  for (size_t i = 0; i < ids.size(); i++) {
    cout << ids[i].substr(1, std::string::npos) << "\t";
    uint64_t result = results[i];

    for (size_t byteOffset = 0; byteOffset < sizeof(uint64_t); ++byteOffset) {
      uint8_t& value = ((uint8_t*)&result)[byteOffset];

      uint8_t mask = 1;
      for (size_t bitOffset = 0; bitOffset < 8; ++bitOffset) {
        if (value & mask)
          cout << byteOffset * 8 + bitOffset << ",";
        mask <<= 1;
      }
    }
    cout << std::endl;
  }
}

} // namespace min_ibf_fpga::io