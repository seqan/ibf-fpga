#pragma once

#pragma once

#pragma once

#include <fstream>
#include <string>
#include <limits>

namespace min_ibf_fpga::fastq
{

struct fastq_parser
{
    template <typename on_sequence_fn_t>
    void operator()(on_sequence_fn_t && on_sequence_fn)
    {
        std::string id;
        std::string sequence;

        while (std::getline(inputStream, id))
        //for (auto && [id, sequence] : fin)
        {
            std::getline(inputStream, sequence);

            on_sequence_fn(id, sequence);

            for (size_t i = 0; i < 2; ++i)
                inputStream.ignore(std::numeric_limits<std::streamsize>::max(),
                    inputStream.widen('\n'));
        }
    }

    std::ifstream inputStream;
};

} // namespace min_ibf_fpga::fastq