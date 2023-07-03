#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w23_k19_query4_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 23, 19),

    // of query "AGCGTAGCGATACGTCTCAAATATTATAACGGCATTACGAGTCAGCTGACTGACATCGTAGCCGT"
    .minimizer_bit_membership = {
        {21580775587, none_match},
        {25743306172, none_match},
        {3567837568, none_match},
        {55819046760, none_match},
        {27701269243, none_match},
        {68563492284, none_match},
        {8939288255, none_match},
        {18399417372, none_match},
        {31110702226, none_match},
        {63937669663, all_match},
        {2070442862, three_match},
        {20790231124, three_match},
        {18393609814, two_three_match},
    },
    .counter = {
        1, 1, 2, 4, // 0 - 3
        1, 1, 2, 4, // 4 - 7
        1, 1, 2, 4, 1, 1, 2, 4, // 8 - 15
        1, 1, 2, 4, 1, 1, 2, 4, 1, 1, 2, 4, 1, 1, 2, 4, // 16 - 31
        1, 1, 2, 4, 1, 1, 2, 4, 1, 1, 2, 4, 1, 1, 2, 4, // 32 - 47
        1, 1, 2, 4, 1, 1, 2, 4, 1, 1, 2, 4, 1, 1, 2, 4, // 48 - 63
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
        /*1 error: */ none_match
    }
};
