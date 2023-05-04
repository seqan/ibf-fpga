#pragma once

template <typename range_t>
void print_range(range_t && range)
{
    for (auto && value : range)
        std::cerr << value << ", ";
    std::cerr << std::endl;
}