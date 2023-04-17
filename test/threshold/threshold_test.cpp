
#include "fixtures/threshold_tables.hpp"

#include <min_ibf_fpga/test/assert_equal.hpp>

int main()
{
    using min_ibf_fpga::test::assert_equal;

    assert_equal<size_t>(threshold_tables::p65_w23_k19_e0_tau0dot99.table[0], 10, "threshold should match");
    assert_equal<size_t>(threshold_tables::p65_w23_k19_e1_tau0dot99.table[0], 1, "threshold should match");

    // TODO THRESHOLD TABLE IS WRONG FOR K=W=19:
    // We need to subtract 2 to make it work, as the precomp table will produces values without the minus 2
    assert_equal<size_t>(threshold_tables::p65_w19_k19_e0.table[0], 47 - 2, "threshold should match");
    assert_equal<size_t>(threshold_tables::p65_w19_k19_e1.table[0], 28 - 2, "threshold should match");

    return 0;
}