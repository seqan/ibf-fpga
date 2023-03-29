

#include "fixtures/all.hpp"

#include <seqan3/alphabet/nucleotide/dna4.hpp>
#include <seqan3/range/views/char_to.hpp>
#include <seqan3/range/views/minimiser_hash.hpp>
#include <seqan3/range/views/to.hpp>

void seqan3_test(minimizer_test_fixture test)
{
    auto query_sequence = test.query | seqan3::views::char_to<seqan3::dna4> | seqan3::views::to<seqan3::dna4_vector>;

    uint8_t W = test.w;
    uint8_t K = test.k;
    size_t seed = 0x8F3F73B5CF1C9ADE >> (64 - 2 * K);

    auto minimizer_view = seqan3::views::minimiser_hash(query_sequence, seqan3::shape{seqan3::ungapped{K}}, seqan3::window_size{W}, seqan3::seed{seed});

    test.compare_minimizer(seqan3::views::to<std::vector<uint64_t>>(minimizer_view));
}

int main()
{
    seqan3_test(minimizer_w23_k19_query0_test);
    seqan3_test(minimizer_w23_k19_query1_test);
    seqan3_test(minimizer_w23_k19_query2_test);
    seqan3_test(minimizer_w23_k19_query3_test);
    seqan3_test(minimizer_w23_k19_query4_test);

    seqan3_test(minimizer_w19_k19_query0_test);
    seqan3_test(minimizer_w19_k19_query1_test);
    seqan3_test(minimizer_w19_k19_query2_test);
    seqan3_test(minimizer_w19_k19_query3_test);
    seqan3_test(minimizer_w19_k19_query4_test);

    return 0;
}