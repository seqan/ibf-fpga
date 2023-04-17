#pragma once

#include <iostream>
#include <sstream>

namespace min_ibf::test
{

template <typename value_t>
static auto assert_equal(value_t const & actual, value_t const & expected, std::string const message)
{
    if (expected == actual) return true;

    std::stringstream sstream{};
    sstream << "Got: " << actual << ", Expected: " << expected << ", " << message;

    std::string error_message = sstream.str();
    std::cerr << error_message << std::endl;

    throw std::runtime_error{error_message};

    return false;
};

} // namespace min_ibf::test