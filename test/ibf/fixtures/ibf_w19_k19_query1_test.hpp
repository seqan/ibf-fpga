#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w19_k19_query1_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 19, 19),

    // of query "GCTAGCGATCTGATTCACGAGCGTACGTGACGTACGTATCGTACTACGTATCGTACTACATGCAT"
    .minimizer_bit_membership = {
        {21104386613, all_match},
        {90983815937, all_match},
        {75910879505, all_match},
        {19894750740, all_match},
        {25641069397, all_match},
        {162021523269, all_match},
        {3072888607, all_match},
        {67254804560, all_match},
        {86983232437, all_match},
        {159075929505, all_match},
        {116966457629, all_match},
        {508910795, all_match},
        {63699236592, all_match},
        {132460836597, zero_one_two_match},
        {101698274152, all_match},
        {79384578589, all_match},
        {67155522728, all_match},
        {244085041925, all_match},
        {7041381992, zero_one_two_match},
        {79430189332, zero_one_two_match},
        {21629852864, zero_one_two_match},
        {3569855413, zero_one_two_match},
        {67138745925, zero_one_two_match},
        {79317467560, zero_one_two_match},
        {78445052008, all_match},
        {67323295285, zero_one_two_match},
        {59404442548, all_match},
        {73180994759, all_match},
        {10711171330, all_match},
        {159065922741, zero_one_two_match},
        {158993755061, zero_one_two_match},
        {67159716127, all_match},
        {79401360576, all_match},
        {3572534888, all_match},
        {244697527877, all_match},
        {64772669864, all_match},
        {71465524767, all_match},
        {67323281680, all_match},
        {90341628852, all_match},
        {73180995509, all_match},
        {66543714754, all_match},
        {77461544119, all_match},
        {158190938720, all_match},
        {208257342657, all_match},
        {30659141919, all_match},
        {115778555496, all_match},
        {195439359413, all_match},
    },
    .counter = {
        37 + 10, 37 + 10, 37 + 10, 37, // 0 - 3
        47, 47, 47, 37, // 4 - 7
        47, 47, 47, 37, 47, 47, 47, 37, // 8 - 15
        47, 47, 47, 37, 47, 47, 47, 37, 47, 47, 47, 37, 47, 47, 47, 37, // 16 - 31
        47, 47, 47, 37, 47, 47, 47, 37, 47, 47, 47, 37, 47, 47, 47, 37, // 32 - 47
        47, 47, 47, 37, 47, 47, 47, 37, 47, 47, 47, 37, 47, 47, 47, 37, // 48 - 63
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
        /*0 errors: */ zero_one_two_match,
        /*1 error: */ all_match
    }
};

static_assert(47 == min_ibf_fpga::threshold::kmer_lemma_threshold(min_ibf_fpga::threshold::kmer_count_of_query(/*query_length*/ 65, /*kmer_size*/ 19), /*kmer_size*/ 19, /*error*/ 0));
static_assert(28 == min_ibf_fpga::threshold::kmer_lemma_threshold(min_ibf_fpga::threshold::kmer_count_of_query(/*query_length*/ 65, /*kmer_size*/ 19), /*kmer_size*/ 19, /*error*/ 1));