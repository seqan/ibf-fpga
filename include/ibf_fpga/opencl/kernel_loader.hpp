#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>

#include <ibf_fpga/opencl/opencl.hpp>

namespace ibf_fpga::opencl
{

struct kernel_loader
{
    static cl::Program load(std::filesystem::path path, cl::Context const & context, std::vector<cl::Device> const & selected_devices)
    {
        std::ifstream program_stream(path, std::ios::in | std::ios::binary);

        std::cerr << "File exists: " << (std::filesystem::exists(path) ? "true" : "false") << std::endl;

        std::vector<uint8_t> binary((std::istreambuf_iterator<char>(program_stream)),
            std::istreambuf_iterator<char>());

        std::cerr << "Loaded program of size " << binary.size() << " bytes" << std::endl;

        cl::Program::Binaries binaries{{binary.data(), binary.data() + binary.size()}};
        cl::Program program(context, selected_devices, binaries);

        program.build(selected_devices);

        return program;
    }
};

} // namespace ibf_fpga::opencl