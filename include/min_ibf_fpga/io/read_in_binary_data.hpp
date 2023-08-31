#pragma once

#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

#include <iostream> // TODO: remove me when std::cerr is not needed anymore

namespace min_ibf_fpga::io
{

template <typename value_t, size_t buffer_size = 1024>
void read_in_binary_data(std::string const & filename, std::vector<value_t> & output_data)
{
  std::ifstream ifs(filename, std::ios::binary);
  assert(ifs.is_open());

  char buffer[buffer_size];

  size_t bytes_read{};
  do {
    ifs.peek();
    bytes_read = ifs.readsome(buffer, buffer_size);

    size_t const data_size = bytes_read / sizeof(value_t); // bytes aligned to value_t
    value_t const * data_begin = reinterpret_cast<value_t *>(buffer);
    value_t const * data_end = data_begin + data_size;
    output_data.insert(output_data.end(), data_begin, data_end);
  } while (bytes_read == buffer_size);

  // TODO: do we want to allow unaligned raw-data?
  size_t const remaining_bytes = bytes_read % sizeof(value_t);
  if (remaining_bytes != 0)
  {
    std::cerr << "read_in_binary_data :: bytes are misaligned, expected " << sizeof(value_t) << " bytes, but only got " << remaining_bytes << " bytes " << std::endl;
    value_t data_value{};

    char * data_value_bytes = reinterpret_cast<char*>(&data_value);
    char * buffer_it = &buffer[bytes_read - remaining_bytes];
    for (size_t i = 0; i < remaining_bytes; ++i)
      data_value_bytes[i] = buffer_it[i];

    output_data.push_back(data_value);
  }
}

} // namespace min_ibf_fpga::io