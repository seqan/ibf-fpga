#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w23_k19_query1_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 23, 19),

    // of query "GCTAGCGATCTGATTCACGAGCGTACGTGACGTACGTATCGTACTACGTATCGTACTACATGCAT"
    .minimizer_bit_membership = {
        {19894750740, all_match},
        {3072888607, all_match},
        {508910795, all_match},
        {63699236592, zero_one_two_match},
        {67155522728, zero_one_two_match},
        {7041381992, zero_one_two_match},
        {3569855413, zero_one_two_match},
        {59404442548, zero_one_two_match},
        {10711171330, zero_one_two_match},
        {3572534888, all_match},
        {64772669864, all_match},
        {66543714754, all_match},
        {30659141919, all_match},
    },
    .counter = {
        13, 13, 13, 7, // 0 - 3
        13, 13, 13, 7, // 4 - 7
        13, 13, 13, 7, 13, 13, 13, 7, // 8 - 15
        13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, // 16 - 31
        13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, // 32 - 47
        13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, 13, 13, 13, 7, // 48 - 63
    },
    .threshold_table_per_error = {
        /*0 errors: */threshold_tables::p65_w23_k19_e0_tau0dot99,
        /*1 error: */threshold_tables::p65_w23_k19_e1_tau0dot99,
    },
    .threshold_per_error = {
        /*0 errors: */ 15,
        /*1 error: */ 5
    },
    .bin_bit_membership_per_error = {
        /*0 errors: */ none_match,
        /*1 error: */ all_match
    }
};
