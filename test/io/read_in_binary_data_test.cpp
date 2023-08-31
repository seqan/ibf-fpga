#include <fstream>

#include <min_ibf_fpga/io/read_in_binary_data.hpp>

#include <min_ibf_fpga/test/assert_equal.hpp>

// xxd -i thresholds_0e.bin (modified for uint64_t)
uint64_t thresholds_0e_bin[] = {
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2a, 0x2b,
};
constexpr size_t thresholds_0e_bin_len = 34;

// xxd -i thresholds_1e.bin (modified for uint64_t)
uint64_t thresholds_1e_bin[] = {
    0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05,
    0x05, 0x06, 0x07, 0x07, 0x08, 0x08, 0x09, 0x0a,
    0x0a, 0x0b, 0x0c, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f,
    0x10, 0x11, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18
};
constexpr size_t thresholds_1e_bin_len = 34;

// xxd -i three_char.bin (modified for uint64_t)
size_t three_char_bin[] = {
  0x0a4241
};
constexpr size_t three_char_bin_len = 1;

int main()
{
    min_ibf_fpga::test::assert_equal<size_t>((272 * sizeof(char) * 8) % 64, 0, "272 % 8 should be 0");
    min_ibf_fpga::test::assert_equal<size_t>(272 / 8, 34, "272 / 8 should be 34");

    {
        std::vector<uint64_t> thresholds_0e;
        std::ifstream ifs("thresholds_0e.bin", std::ios::binary);
        assert(ifs.is_open());
        min_ibf_fpga::io::read_in_binary_data(ifs, thresholds_0e);

        min_ibf_fpga::test::assert_equal(thresholds_0e.size(), thresholds_0e_bin_len, "sizes of thresholds_0e and thresholds_0e_bin should match");
        for(size_t i = 0; i < thresholds_0e_bin_len; ++i)
        {
            min_ibf_fpga::test::assert_equal(thresholds_0e[i], thresholds_0e_bin[i], "i: " + std::to_string(i));
        }
    }

    {
        std::vector<uint64_t> thresholds_1e;
        std::ifstream ifs("thresholds_1e.bin", std::ios::binary);
        assert(ifs.is_open());
        min_ibf_fpga::io::read_in_binary_data<uint64_t, 64>(ifs, thresholds_1e);

        min_ibf_fpga::test::assert_equal(thresholds_1e.size(), thresholds_1e_bin_len, "sizes of thresholds_1e and thresholds_1e_bin should match");
        for(size_t i = 0; i < thresholds_1e_bin_len; ++i)
        {
            min_ibf_fpga::test::assert_equal(thresholds_1e[i], thresholds_1e_bin[i], "i: " + std::to_string(i));
        }
    }

    // special case: input is 3 byte and smaller than 8 byte
    {
        std::vector<uint64_t> three_char;
        std::ifstream ifs("three_char.bin", std::ios::binary);
        assert(ifs.is_open());
        min_ibf_fpga::io::read_in_binary_data(ifs, three_char);

        min_ibf_fpga::test::assert_equal(three_char.size(), three_char_bin_len, "sizes of three_char and three_char_bin should match");
        for(size_t i = 0; i < three_char_bin_len; ++i)
        {
            min_ibf_fpga::test::assert_equal(three_char[i], three_char_bin[i], "i: " + std::to_string(i));
        }
    }

    return 0;
}