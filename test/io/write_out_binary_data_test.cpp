
#include <cstddef>
#include <sstream>
#include <vector>

#include <min_ibf_fpga/io/write_out_binary_data.hpp>

#include <min_ibf_fpga/test/assert_equal.hpp>


int main()
{
    std::vector<uint64_t> hello_world{
        0x6f'57'20'6f'6c'6c'65'48, // "Hello W"
        0x0a'21'64'6c'72, // "ord!\n"
    };

    std::stringstream ostr{};
    min_ibf_fpga::io::write_out_binary_data(ostr, hello_world);

    std::string actual{ostr.str().c_str()};
    std::string expected{"Hello World!\n"};

    min_ibf_fpga::test::assert_equal(actual.size(), expected.size(), "Size");
    min_ibf_fpga::test::assert_equal(actual, expected, "String should be `Hello World\n`");

    return 0;
}