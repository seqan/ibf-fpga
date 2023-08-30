#pragma once

#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace min_ibf_fpga::io
{

template <typename value_t>
void read_in_binary_data(std::string const & filename, std::vector<value_t> & output_data)
{
  std::ifstream ifs(filename, std::ios::binary);

  size_t const file_size = std::filesystem::file_size(filename);
  assert(file_size > 0);

  value_t data_value{};
  size_t bytes_read = 0;
  do {
    size_t const bytes_to_read = std::min(file_size - bytes_read, sizeof(data_value));

    ifs.read(reinterpret_cast<char*>(&data_value), bytes_to_read);
    output_data.push_back(data_value);
    bytes_read += bytes_to_read;

  } while (bytes_read < file_size);
}

} // namespace min_ibf_fpga::io