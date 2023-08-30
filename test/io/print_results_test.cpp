
#include <cstddef>
#include <sstream>
#include <vector>

#include <min_ibf_fpga/io/print_results.hpp>

#include <min_ibf_fpga/test/assert_equal.hpp>


int main()
{

    std::vector<std::string> ids{};
    std::vector<uint64_t> results{};
    std::string expected{};

    ids.push_back(">id0");
    results.push_back(0b0);
    expected += "id0\t\n";

    ids.push_back(">id1");
    results.push_back(0b0000'0001);
    expected += "id1\t7,\n";

    ids.push_back(">id2");
    results.push_back(0b0101'0101);
    expected += "id2\t1,3,5,7,\n";

    ids.push_back(">id3");
    results.push_back(0b1010'0011);
    expected += "id3\t0,2,6,7,\n";

    ids.push_back(">id4");
    results.push_back(0b0101'1100'1010'0011);
    expected += "id4\t0,2,6,7,9,11,12,13,\n";

    std::stringstream ostr{};
    min_ibf_fpga::io::print_results(ids, results, ostr);
    std::string actual{ostr.str().c_str()};

    min_ibf_fpga::test::assert_equal(actual, expected, "Output should be the same.");

    return 0;
}