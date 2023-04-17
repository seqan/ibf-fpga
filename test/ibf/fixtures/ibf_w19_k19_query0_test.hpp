#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w19_k19_query0_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 19, 19),

    // of query "ACGATCGACTAGGAGCGATTACGACTGACTACATCTAGCTAGCTAGAGATTCTTCAGAGCTTAGC"
    .minimizer_bit_membership = {
        {163103919888, all_match},
        {140265937733, all_match},
        {2920729423, all_match},
        {68613121552, all_match},
        {93742599861, all_match},
        {90407940549, all_match},
        {8690258194, all_match},
        {158498823412, all_match},
        {210802667521, all_match},
        {74194351371, all_match},
        {63928284324, all_match},
        {48707234548, all_match},
        {7182414703, all_match},
        {66495065620, all_match},
        {25564089527, all_match},
        {193350566213, all_match},
        {163770686, zero_one_two_match},
        {185277635504, all_match},
        {77123928259, zero_one_two_match},
        {78523399681, zero_one_two_match},
        {21146517416, zero_one_two_match},
        {180509235653, zero_one_two_match},
        {9363272175, zero_one_two_match},
        {155840388865, all_match},
        {21088919457, all_match},
        {119167240788, all_match},
        {8774513135, all_match},
        {158321254145, all_match},
        {89808162497, zero_one_two_match},
        {81486662225, zero_one_two_match},
        {89081683784, zero_one_two_match},
        {197891486205, all_match},
        {140542462544, all_match},
        {13188277435, all_match},
        {188855787265, all_match},
        {76196794028, all_match},
        {14171228756, all_match},
        {21088882823, all_match},
        {265347112705, all_match},
        {76202062395, all_match},
        {180511543377, all_match},
        {20266627258, all_match},
        {180687788545, all_match},
        {76974106092, all_match},
        {82984070660, all_match},
        {8357700737, all_match},
        {20501947716, all_match},
    },
    .counter = {
        38 + 9, 38 + 9, 38 + 9, 38, // 0 - 3
        47, 47, 47, 38, // 4 - 7
        47, 47, 47, 38, 47, 47, 47, 38, // 8 - 15
        47, 47, 47, 38, 47, 47, 47, 38, 47, 47, 47, 38, 47, 47, 47, 38, // 16 - 31
        47, 47, 47, 38, 47, 47, 47, 38, 47, 47, 47, 38, 47, 47, 47, 38, // 32 - 47
        47, 47, 47, 38, 47, 47, 47, 38, 47, 47, 47, 38, 47, 47, 47, 38, // 48 - 63
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
