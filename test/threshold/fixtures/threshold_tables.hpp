#pragma once

#include <min_ibf_fpga/threshold/threshold_table.hpp>
#include <min_ibf_fpga/threshold/kmer_lemma_threshold_table.hpp>

struct threshold_tables
{
    static min_ibf_fpga::threshold::threshold_table p65_w23_k19_e0_tau0dot99;
    static min_ibf_fpga::threshold::threshold_table p65_w23_k19_e1_tau0dot99;

    static min_ibf_fpga::threshold::kmer_lemma_threshold_table p65_w19_k19_e0;
    static min_ibf_fpga::threshold::kmer_lemma_threshold_table p65_w19_k19_e1;
};

min_ibf_fpga::threshold::threshold_table threshold_tables::p65_w23_k19_e0_tau0dot99{{10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43}};
min_ibf_fpga::threshold::threshold_table threshold_tables::p65_w23_k19_e1_tau0dot99{{1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 7, 7, 8, 8, 9, 10, 10, 11, 12, 12, 13, 14, 14, 15, 16, 17, 17, 18, 19, 20, 21, 22, 23, 24}};

min_ibf_fpga::threshold::kmer_lemma_threshold_table threshold_tables::p65_w19_k19_e0 = min_ibf_fpga::threshold::kmer_lemma_threshold_table(65, 19, 0);
min_ibf_fpga::threshold::kmer_lemma_threshold_table threshold_tables::p65_w19_k19_e1 = min_ibf_fpga::threshold::kmer_lemma_threshold_table(65, 19, 1);
