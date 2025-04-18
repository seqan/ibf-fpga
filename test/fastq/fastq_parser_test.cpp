#include <iostream>
#include <min_ibf_fpga/fastq/fastq_parser.hpp>

#include <vector>
#include <string>

#include <min_ibf_fpga/test/assert_equal.hpp>

int main()
{
    std::vector<std::string> ids{"@query0_e0_012_e1_0123", "@query1_e0_1_e1_1", "@query2_e0_3_e1_23", "@query3_never"};
    std::vector<std::string> sequences{"ACGATCGACTAGGAGCGATTACGACTGACTACATCTAGCTAGCTAGAGATTCTTCAGAGCTTAGC", "AGCGAGGCAGGCAGCGATCGAGCGAGCGCATGCAGCGACTAGCTACGACAGCTACTATCAGCAGC", "CGAGCGATATCTTATGGTAGGCATCGAGCATCGAGGAGCGATCTATCTATCTATCATCTATCTAT", "AGCGTAGCGATACGTCTCAAATATTATAACGGCATTACGAGTCAGCTGACTGACATCGTAGCCGT"};
    std::ifstream ifstream("query.fq", std::ios::in | std::ios::binary);

    min_ibf_fpga::test::assert_equal(ifstream.is_open(), true, "file `query.fq` exists");

    min_ibf_fpga::fastq::fastq_parser parse{.inputStream = std::move(ifstream)};

    int i = 0;
    parse([&](auto && id, auto && sequence)
    {
        min_ibf_fpga::test::assert_equal(id, ids[i], "ids[" + std::to_string(i) +"] mismatch");
        min_ibf_fpga::test::assert_equal(sequence, sequences[i], "sequences[" + std::to_string(i) +"] mismatch");

        ++i;
    });

    min_ibf_fpga::test::assert_equal(i, 4, "|sequences| should be 4");
    return 0;
}