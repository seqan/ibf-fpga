#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <map>
#include <vector>

#include <min_ibf_fpga/threshold/threshold_table.hpp>
#include <min_ibf_fpga/test/assert_equal.hpp>
#include <min_ibf_fpga/test/minimizer_range_t.hpp>

#include "../threshold/fixtures/threshold_tables.hpp"

struct ibf_test_fixture
{
    static constexpr size_t bin_count = 64;

    using hash_t = size_t;
    using bit_membership_t = std::bitset<bin_count>;

    using minimizer_bit_membership_t = std::vector<std::tuple<hash_t, bit_membership_t>>;
    using bin_bit_membership_t = bit_membership_t;

    using counter_t = std::array<size_t, bin_count>;

    min_ibf_fpga::test::minimizer_range_t minimizer_range;

    minimizer_bit_membership_t minimizer_bit_membership;
    counter_t counter;

    std::vector<min_ibf_fpga::threshold::threshold_table> threshold_table_per_error;
    std::vector<size_t> threshold_per_error;
    std::vector<bin_bit_membership_t> bin_bit_membership_per_error;

    std::vector<uint64_t> minimizer() const
    {
        std::vector<uint64_t> minimizer{};
        for (auto const & [hash, _] : minimizer_bit_membership)
        {
            minimizer.push_back(hash);
        }
        return minimizer;
    }

    bool compare_minimizer_membership(minimizer_bit_membership_t const & result) const
    {
        minimizer_bit_membership_t const & expected = minimizer_bit_membership;

        min_ibf_fpga::test::assert_equal(result.size(), expected.size(), "number of containments match");

        auto map_at = [](auto const & result, auto const & hash) -> auto const &
        {
            // result.at(hash)
            for (auto const & [key, value] : result)
            {
                if (key == hash) return value;
            }

            throw std::runtime_error{"KEY NOT FOUND"};
        };

        for (auto const & [hash, bit_membership_expected] : expected)
        {
            bit_membership_t const & bit_membership_actual = map_at(result, hash);
            min_ibf_fpga::test::assert_equal(bit_membership_actual, bit_membership_expected, "bit membership matches of minimizer: " + std::to_string(hash));
        }

        return true;
    }

    bool compare_counter(counter_t const & result) const
    {
        counter_t const & expected = counter;

        min_ibf_fpga::test::assert_equal(result.size(), expected.size(), "number of counter match");

        for (size_t i = 0; i < expected.size(); ++i)
        {
            min_ibf_fpga::test::assert_equal(result[i], expected[i], "counter[i: " + std::to_string(i) + "] matches");
        }

        return true;
    }

    bool compare_bin_bit_membership(std::vector<bin_bit_membership_t> const & result_per_error) const
    {
        std::vector<bin_bit_membership_t> const & expected = bin_bit_membership_per_error;

        for (size_t error = 0; error < expected.size(); ++error)
        {
            min_ibf_fpga::test::assert_equal(result_per_error[error], expected[error], "bin membership[error: " + std::to_string(error) + "] matches");
        }

        return true;
    }
};

static constexpr std::bitset<64> none_match
    = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ull; // 0ull
static constexpr std::bitset<64> all_match
    = 0b11111111'11111111'11111111'11111111'11111111'11111111'11111111'11111111ull; // 18446744073709551615ull
static constexpr std::bitset<64> zero_one_two_match
    = 0b01110111'01110111'01110111'01110111'01110111'01110111'01110111'01110111ull; // 8608480567731124087ull
static constexpr std::bitset<64> one_three_match
    = 0b10101010'10101010'10101010'10101010'10101010'10101010'10101010'10101010ull;
static constexpr std::bitset<64> one_match
    = 0b00100010'00100010'00100010'00100010'00100010'00100010'00100010'00100010ull;
static constexpr std::bitset<64> three_match
    = 0b10001000'10001000'10001000'10001000'10001000'10001000'10001000'10001000ull;
static constexpr std::bitset<64> zero_one_match
    = 0b00110011'00110011'00110011'00110011'00110011'00110011'00110011'00110011ull;
static constexpr std::bitset<64> one_two_match
    = 0b01100110'01100110'01100110'01100110'01100110'01100110'01100110'01100110ull;
static constexpr std::bitset<64> two_three_match
    = 0b11001100'11001100'11001100'11001100'11001100'11001100'11001100'11001100ull;
static constexpr std::bitset<64> one_two_three_match
    = 0b11101110'11101110'11101110'11101110'11101110'11101110'11101110'11101110ull;
