#include <filesystem>
#include <future>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

#include "../cl_event_profiling.hpp"

#include "../device_read_only_buffer.hpp"
#include "../device_write_only_buffer.hpp"
#include <ibf_fpga/opencl/device_selector.hpp>
#include <ibf_fpga/opencl/kernel_loader.hpp>


// debug
#include "../print_range.hpp"
#include "../check_result.hpp"

int main()
{
    constexpr bool profile = true;

    using value_type = int32_t;

    std::vector<cl::Device> all_devices = ibf_fpga::opencl::device_selector::select_devices();
    std::vector<cl::Device> selected_devices{{all_devices.at(0)}};
    cl::Context context{selected_devices};
    cl::CommandQueue utilitiesQueue{context, selected_devices.at(0), profile? CL_QUEUE_PROFILING_ENABLE : 0};

    std::vector<value_type> input1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<value_type> input2{10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 110};
    std::cerr << "input1: "; print_range(input1);
    std::cerr << "input2: "; print_range(input2);

    using OutputAllocator = std::allocator<value_type>;
    OutputAllocator const svmWriteOnlyAlloc{};
    std::vector<value_type, OutputAllocator> output(svmWriteOnlyAlloc);
    output.resize(input1.size());
    std::cerr << "output: "; print_range(output);

    device_read_only_buffer input1_cl_device_buffer{"input1", input1.data(), input1.size(), context};
    device_read_only_buffer input2_cl_device_buffer{"input2", input2.data(), input2.size(), context};
    device_write_only_buffer output_cl_device_buffer{"output", output.data(), output.size(), context};

    input1_cl_device_buffer.transfer2device(utilitiesQueue);
    input2_cl_device_buffer.transfer2device(utilitiesQueue);

    input1_cl_device_buffer.register_profiling();
    input2_cl_device_buffer.register_profiling();

    input1_cl_device_buffer.wait();
    input2_cl_device_buffer.wait();

    std::filesystem::path program_file{"add_kernel.aocx"};
    std::cerr << "Load Program: " << program_file << std::endl;
    cl::Program program = ibf_fpga::opencl::kernel_loader::load(program_file, context, selected_devices);

    cl::Kernel add_kernel(program, "add_range");
    add_kernel.setArg(0, input1.size());
    add_kernel.setArg(1, input1_cl_device_buffer.cl_device_buffer);
    add_kernel.setArg(2, input2_cl_device_buffer.cl_device_buffer);
    add_kernel.setArg(3, output_cl_device_buffer.cl_device_buffer);

    cl::CommandQueue kernelQueue{context, selected_devices.at(0), profile? CL_QUEUE_PROFILING_ENABLE : 0};

    std::vector<cl::Event> add_kernel_event_wait_list{/*output_unmap_event*/};
    cl::Event add_kernel_event;
    kernelQueue.enqueueNDRangeKernel(
            add_kernel, cl::NullRange, cl::NDRange(1), cl::NullRange,
            &add_kernel_event_wait_list,
            &add_kernel_event);

    add_kernel_event.wait();

    output_cl_device_buffer.transfer2host(utilitiesQueue);
    output_cl_device_buffer.register_profiling();
    output_cl_device_buffer.wait();

    std::cerr << "output: "; print_range(output);

    std::cout << "reached end of file1 (cout)" << std::endl;
    std::cerr << "reached end of file2 (cerr)" << std::endl;

    check_result(output);

    utilitiesQueue.finish();
    kernelQueue.finish();

    std::cout.flush();
    std::cerr.flush();

    return 0;
}