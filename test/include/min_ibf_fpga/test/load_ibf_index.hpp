#pragma once

#include <filesystem>

#include <cereal/details/traits.hpp>
#include <cereal/archives/binary.hpp>

namespace min_ibf_fpga::test
{

auto load_ibf_index(std::filesystem::path const ibf_path)
{
    std::tuple<min_ibf_fpga::index::ibf_metadata, min_ibf_fpga::index::ibf_data> whole_ibf{};
    auto & [ibf_meta, ibf] = whole_ibf;

    std::ifstream archiveStream{ibf_path, std::ios::binary};
    cereal::BinaryInputArchive archive{archiveStream};

    archive(ibf_meta);
    archive(ibf);

    return whole_ibf;
};

} // namespace min_ibf_fpga::test