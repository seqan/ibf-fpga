#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w19_k19_query2_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 19, 19),

    // of query "AGCGAGGCAGGCAGCGATCGAGCGAGCGCATGCAGCGACTAGCTACGACAGCTACTATCAGCAGC"
    .minimizer_bit_membership = {
        {2823518740, one_three_match},
        {25349704517, one_three_match},
        {94382691333, one_match},
        {5860396561, one_match},
        {20077837028, one_match},
        {46824872789, one_three_match},
        {31864403435, all_match},
        {212026232593, zero_one_match},
        {20010994404, all_match},
        {46874151765, all_match},
        {24589818634, all_match},
        {113049166061, zero_one_two_match},
        {181501519072, one_three_match},
        {25668799293, all_match},
        {30763817797, zero_one_two_match},
        {2914945431, one_three_match},
        {198748503777, one_two_match},
        {18727908523, zero_one_two_match},
        {249833364229, one_match},
        {7209486827, one_two_three_match},
        {61498100244, one_match},
        {25393538292, one_three_match},
        {73387215621, all_match},
        {7167571201, one_two_three_match},
        {20975887124, one_match},
        {23648889428, one_match},
        {76185667989, one_match},
        {30790688514, one_match},
        {207731106997, one_three_match},
        {90296519392, one_match},
        {62238110871, one_match},
        {31535832311, all_match},
        {213374020449, one_match},
        {19211298324, one_three_match},
        {25019059284, all_match},
        {76045906373, one_match},
        {8692367106, all_match},
        {158524023988, all_match},
        {73118342913, all_match},
        {76207911168, one_match},
        {3812767316, all_match},
        {20796746311, one_two_three_match},
        {266239017153, all_match},
        {18311190047, one_match},
        {112423105448, all_match},
        {255653042657, one_match},
        {12494847802, one_match},
    },
    .counter = {
        18, 47, 21, 25, // 0 - 3
        18, 47, 21, 25, // 4 - 7
        18, 47, 21, 25, 18, 47, 21, 25, // 8 - 15
        18, 47, 21, 25, 18, 47, 21, 25, 18, 47, 21, 25, 18, 47, 21, 25, // 16 - 31
        18, 47, 21, 25, 18, 47, 21, 25, 18, 47, 21, 25, 18, 47, 21, 25, // 32 - 47
        18, 47, 21, 25, 18, 47, 21, 25, 18, 47, 21, 25, 18, 47, 21, 25, // 48 - 63
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
        /*0 errors: */ one_match,
        /*1 error: */ one_match
    }
};

static_assert(47 == min_ibf_fpga::threshold::kmer_lemma_threshold(min_ibf_fpga::threshold::kmer_count_of_query(/*query_length*/ 65, /*kmer_size*/ 19), /*kmer_size*/ 19, /*error*/ 0));
static_assert(28 == min_ibf_fpga::threshold::kmer_lemma_threshold(min_ibf_fpga::threshold::kmer_count_of_query(/*query_length*/ 65, /*kmer_size*/ 19), /*kmer_size*/ 19, /*error*/ 1));