#pragma once

#include <iostream>
#include <vector>

#include <min_ibf_fpga/backend_opencl/opencl.hpp>

namespace min_ibf_fpga::backend_opencl
{

struct device_selector
{
    static std::vector<cl::Device> select_devices()
    {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        if (!platforms.size())
            throw std::runtime_error("No platform found");

        std::cerr << "Available OpenCL platforms:" << std::endl;

        int selected_platform = -1;

        int i = 0;
        for (cl::Platform& platform:platforms)
        {
            std::string platform_name = platform.getInfo<CL_PLATFORM_NAME>();
            std::cerr << "- " << i << ": " << platform_name << std::endl;

            if (platform_name.find("FPGA") != std::string::npos) {
                selected_platform = i;
                std::cerr << "- " << i << ": " << platform_name << " SELECTED" << std::endl;
            }

            i++;
        }

        if (selected_platform == -1)
        {
            std::cerr << "Please select platform: " << std::flush;
            std::cin >> selected_platform;
        }

        cl::Platform& defaultPlatform = platforms.at(selected_platform);
        cl::Platform::setDefault(defaultPlatform);

        std::cerr << "Using " << defaultPlatform.getInfo<CL_PLATFORM_NAME>() << std::endl;

        std::vector<cl::Device> devices;
        defaultPlatform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

        if (!devices.size())
            throw std::runtime_error("No device found");

        return devices;
    }
};

} // namespace min_ibf_fpga::backend_opencl