#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w23_k19_query0_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 23, 19),

    // of query "ACGATCGACTAGGAGCGATTACGACTGACTACATCTAGCTAGCTAGAGATTCTTCAGAGCTTAGC"
    .minimizer_bit_membership = {
        {2920729423, all_match},
        {8690258194, all_match},
        {48707234548, all_match},
        {7182414703, zero_one_two_match},
        {163770686, zero_one_two_match},
        {21146517416, zero_one_two_match},
        {9363272175, zero_one_two_match},
        {8774513135, zero_one_two_match},
        {81486662225, zero_one_two_match},
        {13188277435, all_match},
        {14171228756, all_match},
        {20266627258, all_match},
        {8357700737, all_match},
    }, // size: 13
    .counter = {
        13, 13, 13, 7, // 0 - 3
        13, 13, 13, 7, // 4 - 7
        13, 13, 13, 7, 13, 13, 13, 7, // 8 - 15
        13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, // 16 - 31
        13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, // 32 - 47
        13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, // 48 - 63
    }, // size: 64
    .threshold_table_per_error = {
        /*0 errors: */ threshold_tables::p65_w23_k19_e0_tau0dot99,
        /*1 error: */ threshold_tables::p65_w23_k19_e1_tau0dot99,
    },
    .threshold_per_error = {
        /*0 errors: */ 15, // p65_w23_k19_e0_tau0dot99[13 - 10] + 2 = 13 + 2 ???? Investigate Formula
        /*1 error: */ 5 // p65_w23_k19_e0_tau1dot99[13 - 10] + 2 = 3 + 2 ???? Investigate Formula
    },
    .bin_bit_membership_per_error = {
        /*0 errors: */ none_match,
        /*1 error: */ all_match
    }
};
