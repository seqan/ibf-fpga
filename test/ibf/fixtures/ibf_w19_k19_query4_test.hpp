#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w19_k19_query4_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 19, 19),

    // of query "AGCGTAGCGATACGTCTCAAATATTATAACGGCATTACGAGTCAGCTGACTGACATCGTAGCCGT"
    .minimizer_bit_membership = {
        {117189996820, one_match},
        {69648130287, all_match},
        {25743306172, all_match},
        {81608504901, all_match},
        {3567837568, zero_one_match},
        {71993314496, two_three_match},
        {79332909635, zero_one_two_match},
        {27701269243, two_three_match},
        {82315325004, all_match},
        {193268666641, zero_one_two_match},
        {8939288255, all_match},
        {119069965184, all_match},
        {84404113313, one_match},
        {160459046996, all_match},
        {213534781157, all_match},
        {174050850429, all_match},
        {192592507757, two_three_match},
        {100852232387, three_match},
        {63937669663, all_match},
        {105303575720, three_match},
        {193761023237, three_match},
        {2070442862, three_match},
        {174050839621, three_match},
        {64027847080, three_match},
        {78400864011, three_match},
        {213534417089, three_match},
        {20790231124, three_match},
        {18393609814, two_three_match},
        {248006197153, three_match},
        {119043874600, one_two_three_match},
    },
    .counter = {
        13, 16, 17, 25, // 0 - 3
        13, 16, 17, 25, // 4 - 7
        13, 16, 17, 25, 13, 16, 17, 25, // 8 - 15
        13, 16, 17, 25, 13, 16, 17, 25, 13, 16, 17, 25, 13, 16, 17, 25, // 16 - 31
        13, 16, 17, 25, 13, 16, 17, 25, 13, 16, 17, 25, 13, 16, 17, 25, // 32 - 47
        13, 16, 17, 25, 13, 16, 17, 25, 13, 16, 17, 25, 13, 16, 17, 25, // 48 - 63
    },
    .threshold_table_per_error = {
        /*0 errors: */ threshold_tables::p65_w19_k19_e0,
        /*1 error: */ threshold_tables::p65_w19_k19_e1,
    },
    .threshold_per_error = {
        /*0 errors: */ 47,
        /*1 error: */ 28
    },
    .bin_bit_membership_per_error = {
        /*0 errors: */ none_match,
        /*1 error: */ none_match
    }
};

static_assert(47 == min_ibf_fpga::threshold::kmer_lemma_threshold(min_ibf_fpga::threshold::kmer_count_of_query(/*query_length*/ 65, /*kmer_size*/ 19), /*kmer_size*/ 19, /*error*/ 0));
static_assert(28 == min_ibf_fpga::threshold::kmer_lemma_threshold(min_ibf_fpga::threshold::kmer_count_of_query(/*query_length*/ 65, /*kmer_size*/ 19), /*kmer_size*/ 19, /*error*/ 1));