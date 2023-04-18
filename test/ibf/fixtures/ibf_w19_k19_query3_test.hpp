#pragma once

#include "ibf_test_fixture.hpp"

ibf_test_fixture ibf_w19_k19_query3_test =
{
    .minimizer_range = min_ibf_fpga::test::minimizer_range_t::of_query(65, 19, 19),

    // of query "CGAGCGATATCTTATGGTAGGCATCGAGCATCGAGGAGCGATCTATCTATCTATCATCTATCTAT"
    .minimizer_bit_membership = {
        {79523994373, all_match},
        {7196058561, three_match},
        {181512951737, two_three_match},
        {25713182298, three_match},
        {114866310725, three_match},
        {4161953760, three_match},
        {1489514304, three_match},
        {71726253125, three_match},
        {94332887376, three_match},
        {85923364369, one_three_match},
        {20070529457, three_match},
        {112058771523, all_match},
        {70388299232, three_match},
        {1381151008, all_match},
        {107467596869, three_match},
        {38233885099, three_match},
        {66219965969, two_three_match},
        {74570440953, all_match},
        {94879059684, two_three_match},
        {47143466449, all_match},
        {19634756426, all_match},
        {209070417749, all_match},
        {93315140112, two_three_match},
        {94137241360, one_two_three_match},
        {2827051781, all_match},
        {6481693508, all_match},
        {77203060116, two_three_match},
        {30458940928, two_three_match},
        {3971723517, all_match},
        {184801376784, two_three_match},
        {77274454187, two_three_match},
        {202992873477, one_two_three_match},
        {3976170030, two_three_match},
        {180018207300, two_three_match},
        {48704270767, one_two_three_match},
        {7192157696, two_three_match},
        {3976187412, one_two_three_match},
        {25651042887, two_three_match},
        {197533818693, two_three_match},
        {1129341438, two_three_match},
        {185438555728, two_three_match},
        {77159736763, all_match},
        {202969636673, two_three_match},
        {3964690943, two_three_match},
        {184805215824, two_three_match},
        {77274031547, two_three_match},
        {202992967489, all_match},
    },
    .counter = {
        12, 17, 35, 47, // 0 - 3
        12, 17, 35, 47, // 4 - 7
        12, 17, 35, 47, 12, 17, 35, 47, // 8 - 15
        12, 17, 35, 47, 12, 17, 35, 47, 12, 17, 35, 47, 12, 17, 35, 47, // 16 - 31
        12, 17, 35, 47, 12, 17, 35, 47, 12, 17, 35, 47, 12, 17, 35, 47, // 32 - 47
        12, 17, 35, 47, 12, 17, 35, 47, 12, 17, 35, 47, 12, 17, 35, 47, // 48 - 63
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
        /*0 errors: */ three_match,
        /*1 error: */ two_three_match
    }
};

static_assert(47 == min_ibf_fpga::threshold::kmer_lemma_threshold(min_ibf_fpga::threshold::kmer_count_of_query(/*query_length*/ 65, /*kmer_size*/ 19), /*kmer_size*/ 19, /*error*/ 0));
static_assert(28 == min_ibf_fpga::threshold::kmer_lemma_threshold(min_ibf_fpga::threshold::kmer_count_of_query(/*query_length*/ 65, /*kmer_size*/ 19), /*kmer_size*/ 19, /*error*/ 1));