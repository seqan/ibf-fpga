#include <min_ibf_fpga/test/assert_equal.hpp>
#include <min_ibf_fpga/test/minimizer_range_t.hpp>

int main()
{
    using min_ibf_fpga::test::minimizer_range_t;
    using min_ibf_fpga::test::assert_equal;

    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 19).minimal, 10, "There must be at least 10 (23,19)-minimizer in a pattern of length 65");
    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 19).maximal, 43, "There can be at most 43 (23,19)-minimizer in a pattern of length 65");

    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 20).minimal, 12, "There must be at least 12 (23,20)-minimizer in a pattern of length 65");
    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 20).maximal, 43, "There can be at most 43 (23,20)-minimizer in a pattern of length 65");

    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 21).minimal, 15, "There must be at least 15 (23,21)-minimizer in a pattern of length 65");
    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 21).maximal, 43, "There can be at most 43 (23,21)-minimizer in a pattern of length 65");

    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 22).minimal, 22, "There must be at least 22 (23,22)-minimizer in a pattern of length 65");
    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 22).maximal, 43, "There can be at most 43 (23,22)-minimizer in a pattern of length 65");

    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 23).minimal, 43, "There must be at least 43 (23,23)-minimizer in a pattern of length 65");
    assert_equal<size_t>(minimizer_range_t::of_query(65, 23, 23).maximal, 43, "There can be at most 43 (23,23)-minimizer in a pattern of length 65");

    assert_equal<size_t>(minimizer_range_t::of_query(65, 19, 19).minimal, 47, "There must be at least 47 (19,19)-minimizer in a pattern of length 65");
    assert_equal<size_t>(minimizer_range_t::of_query(65, 19, 19).maximal, 47, "There can be at most 47 (19,19)-minimizer in a pattern of length 65");

    return 0;
}