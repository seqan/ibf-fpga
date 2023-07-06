#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w23_k19_query3_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 23, 19),

    // of query "CGAGCGATATCTTATGGTAGGCATCGAGCATCGAGGAGCGATCTATCTATCTATCATCTATCTAT"
    .minimizer_bit_membership = {
        {7196058561, three_match},
        {4161953760, three_match},
        {1489514304, three_match},
        {20070529457, three_match},
        {1381151008, three_match},
        {38233885099, three_match},
        {19634756426, two_three_match},
        {2827051781, two_three_match},
        {3971723517, one_two_three_match},
        {3976170030, two_three_match},
        {3976187412, two_three_match},
        {1129341438, two_three_match},
        {3964690943, two_three_match},
    },
    .counter = {
        0, 1, 7, 13, // 0 - 3
        0, 1, 7, 13, // 4 - 7
        0, 1, 7, 13, 0, 1, 7, 13, // 8 - 15
        0, 1, 7, 13, 0, 1, 7, 13, 0, 1, 7, 13, 0, 1, 7, 13, // 16 - 31
        0, 1, 7, 13, 0, 1, 7, 13, 0, 1, 7, 13, 0, 1, 7, 13, // 32 - 47
        0, 1, 7, 13, 0, 1, 7, 13, 0, 1, 7, 13, 0, 1, 7, 13, // 48 - 63
    },
    .threshold_table_per_error = {
        /*0 errors: */ threshold_tables::p65_w23_k19_e0_tau0dot99,
        /*1 error: */ threshold_tables::p65_w23_k19_e1_tau0dot99,
    },
    .threshold_per_error = {
        /*0 errors: */ 15,
        /*1 error: */ 5
    },
    .bin_bit_membership_per_error = {
        /*0 errors: */ none_match,
        /*1 error: */ two_three_match
    }
};
