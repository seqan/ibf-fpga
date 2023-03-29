#pragma once

#include <exception>

template <typename range_t>
void check_result(range_t output)
{
    auto assert_eq = [](auto value, auto expected, std::string_view message)
    {
        if (value == expected) return;

        std::cout << "'" << value << "' != '" << expected << "' mismatch: " << message << std::endl;
        throw std::runtime_error{"Assertion ERROR"};
    };

    assert_eq(output.size(), 11, "Output size should be 11.");
    assert_eq(output[0], 0 + 10, "Output[0] should be 0 + 10.");
    assert_eq(output[1], 1 + 11, "Output[1] should be 1 + 11.");
    assert_eq(output[2], 2 + 12, "Output[2] should be 2 + 12.");
    assert_eq(output[3], 3 + 13, "Output[3] should be 3 + 13.");
    assert_eq(output[4], 4 + 14, "Output[4] should be 4 + 14.");
    assert_eq(output[5], 5 + 15, "Output[5] should be 5 + 15.");
    assert_eq(output[6], 6 + 16, "Output[6] should be 6 + 16.");
    assert_eq(output[7], 7 + 17, "Output[7] should be 7 + 17.");
    assert_eq(output[8], 8 + 18, "Output[8] should be 8 + 18.");
    assert_eq(output[9], 9 + 19, "Output[9] should be 9 + 19.");
    assert_eq(output[10], 10 + 110, "Output[10] should be 10 + 110.");
}