#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w23_k19_query2_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 23, 19),

    // of query "AGCGAGGCAGGCAGCGATCGAGCGAGCGCATGCAGCGACTAGCTACGACAGCTACTATCAGCAGC"
    .minimizer_bit_membership = {
        {2823518740, one_match},
        {5860396561, one_match},
        {20010994404, one_match},
        {24589818634, one_match},
        {2914945431, one_match},
        {7209486827, one_match},
        {7167571201, one_match},
        {20975887124, one_match},
        {23648889428, one_match},
        {30790688514, one_match},
        {31535832311, all_match},
        {19211298324, one_match},
        {8692367106, all_match},
        {3812767316, one_match},
        {18311190047, one_match},
        {12494847802, one_match},
    },
    .counter = {
        2, 16, 2, 2, // 0 - 3
        2, 16, 2, 2, // 4 - 7
        2, 16, 2, 2, 2, 16, 2, 2, // 8 - 15
        2, 16, 2, 2, 2, 16, 2, 2, 2, 16, 2, 2, 2, 16, 2, 2, // 16 - 31
        2, 16, 2, 2, 2, 16, 2, 2, 2, 16, 2, 2, 2, 16, 2, 2, // 32 - 47
        2, 16, 2, 2, 2, 16, 2, 2, 2, 16, 2, 2, 2, 16, 2, 2, // 48 - 63
    },
    .threshold_table_per_error = {
        /*0 errors: */ threshold_tables::p65_w23_k19_e0_tau0dot99,
        /*1 error: */ threshold_tables::p65_w23_k19_e1_tau0dot99,
    },
    .threshold_per_error = {
        /*0 errors: */ 18,
        /*1 error: */ 6
    },
    .bin_bit_membership_per_error = {
        /*0 errors: */ none_match,
        /*1 error: */ one_match
    }
};
