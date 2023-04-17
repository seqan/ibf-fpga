#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <min_ibf/test/assert_equal.hpp>

struct minimizer_test_fixture
{
    size_t w{};
    size_t k{};
    std::string query{};
    std::vector<uint64_t> minimizer{};

    bool compare_minimizer(std::vector<uint64_t> const & result)
    {
        min_ibf::test::assert_equal(result.size(), minimizer.size(), "Sizes mismatch");
        for (int i = 0; i < result.size(); ++i)
        {
            min_ibf::test::assert_equal(result[i], minimizer[i], "Elements[i: " + std::to_string(i) + "] mismatch");
        }
        return true;
    }
};
