#include <min_ibf_fpga/test/assert_equal.hpp>

int main()
{
    using min_ibf_fpga::test::assert_equal;

    assert_equal<int>(5, 5, "5 == 5 should be equal");

    bool did_throw = false;
    try {
        assert_equal<int>(5, 6, "5 == 6 should be unequal");
    } catch(std::runtime_error const & e) {
        did_throw = true;
    }
    assert_equal(did_throw, true, "unequal should have thrown");

    return 0;
}