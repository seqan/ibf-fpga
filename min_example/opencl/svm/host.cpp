#include <filesystem>
#include <future>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>
#include <span>

#include "../cl_event_profiling.hpp"

#include "../device_read_only_buffer.hpp"
#include "../device_write_only_buffer.hpp"
#include <ibf_fpga/opencl/device_selector.hpp>
#include <ibf_fpga/opencl/kernel_loader.hpp>

// debug
#include "../print_range.hpp"
#include "../check_result.hpp"

// Avoid initialization on vector resize
template<typename T>
struct NoInit
{
    T value;

    NoInit()
    {
        static_assert(sizeof(*this) == sizeof(value), "Invalid size");
        static_assert(__alignof(*this) == __alignof(value), "Invalid alignment");
    }

    friend std::ostream & operator<< (std::ostream& out, NoInit const & obj)
    {
        out << obj.value;
        return out;
    }

    friend bool operator==(NoInit const & obj, T const & a)
    {
        return obj.value == a;
    }
};

void print_svm_capabilities(cl::Device const & device)
{
    cl_device_svm_capabilities caps;

    cl_int err = clGetDeviceInfo(
        device.get(),
        CL_DEVICE_SVM_CAPABILITIES,
        sizeof(cl_device_svm_capabilities),
        &caps,
        0
    );

    if (err == CL_INVALID_VALUE)
    {
        std::cout << "No SVM support" << std::endl;
    }
    if (err == CL_SUCCESS && (caps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER))
    {
        std::cout << "Coarse-grained buffer" << std::endl;
    }
    if (err == CL_SUCCESS && (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER))
    {
        std::cout << "Fine-grained buffer" << std::endl;
    }
    if (err == CL_SUCCESS && (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER) && (caps & CL_DEVICE_SVM_ATOMICS))
    {
        std::cout << "Fine-grained buffer with atomics" << std::endl;
    }
    if (err == CL_SUCCESS && (caps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM))
    {
        std::cout << "Fine-grained system" << std::endl;
    }
    if (err == CL_SUCCESS && (caps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM) && (caps & CL_DEVICE_SVM_ATOMICS))
    {
        std::cout << "Fine-grained system with atomics" << std::endl;
    }

    std::cout << "err: " << err << ", CL_INVALID_VALUE: " << CL_INVALID_VALUE << std::endl;
    std::cout << "caps: " << caps << std::endl;
}

int main()
{
    constexpr bool profile = true;

    using value_type = int32_t;

    std::vector<cl::Device> all_devices = ibf_fpga::opencl::device_selector::select_devices();
    std::vector<cl::Device> selected_devices{{all_devices.at(0)}};
    cl::Context context{selected_devices};
    cl::CommandQueue utilitiesQueue{context, selected_devices.at(0), profile? CL_QUEUE_PROFILING_ENABLE : 0};
    cl::CommandQueue::setDefault(utilitiesQueue); // cl::SVMAllocator does map memory (SVM) on first allocation on the default CommandQueue

    cl_int err;
    print_svm_capabilities(selected_devices.at(0));

    std::vector<value_type> input1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<value_type> input2{10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 110};
    std::cerr << "input1: "; print_range(input1);
    std::cerr << "input2: "; print_range(input2);

    using OutputAllocator =
        cl::SVMAllocator<value_type, cl::SVMTraitCoarse<cl::SVMTraitWriteOnly<>>>;

    OutputAllocator const svmWriteOnlyAlloc(context);

    // size_t output_ptr_size = input1.size() * sizeof(int32_t);
    // int32_t * output_ptr = (int32_t *) clSVMAlloc(context.get(), CL_MEM_WRITE_ONLY | CL_MEM_SVM_FINE_GRAIN_BUFFER, output_ptr_size, alignof(int32_t));
    // std::cout << "output_ptr: " << output_ptr << std::endl;

    std::vector<NoInit<value_type>, OutputAllocator> output(svmWriteOnlyAlloc);
    output.resize(input1.size());
    std::cerr << "output: "; print_range(output);

    device_read_only_buffer input1_cl_device_buffer{"input1", input1.data(), input1.size(), context};
    device_read_only_buffer input2_cl_device_buffer{"input2", input2.data(), input2.size(), context};

    input1_cl_device_buffer.transfer2device(utilitiesQueue);
    input2_cl_device_buffer.transfer2device(utilitiesQueue);

    input1_cl_device_buffer.register_profiling();
    input2_cl_device_buffer.register_profiling();

    input1_cl_device_buffer.wait();
    input2_cl_device_buffer.wait();

    std::cerr << "Map output onto device" << std::endl;
    cl::Event output_unmap_event;

    // cl_event output_unmap_cl_event;
    // err = clEnqueueSVMUnmap(utilitiesQueue.get(), output_ptr, 0, nullptr, &output_unmap_cl_event);
    // std::cout << "clEnqueueSVMUnmap: " << err << ", " << (err == CL_SUCCESS ? "SUCCESS!" : "ERROR!") << std::endl;
    // std::cout << "err: " << err << ", CL_INVALID_VALUE: " << CL_INVALID_VALUE << std::endl;

    utilitiesQueue.enqueueUnmapSVM(output, nullptr, &output_unmap_event);

    std::filesystem::path program_file{"add_kernel.aocx"};
    std::cerr << "Load Program: " << program_file << std::endl;
    cl::Program program = ibf_fpga::opencl::kernel_loader::load(program_file, context, selected_devices);

    cl::Kernel add_kernel(program, "add_range");
    add_kernel.setArg(0, input1.size());
    add_kernel.setArg(1, input1_cl_device_buffer.cl_device_buffer);
    add_kernel.setArg(2, input2_cl_device_buffer.cl_device_buffer);
    add_kernel.setArg(3, output);

    cl::CommandQueue kernelQueue{context, selected_devices.at(0), profile? CL_QUEUE_PROFILING_ENABLE : 0};

    std::vector<cl::Event> add_kernel_event_wait_list{output_unmap_event};
    cl::Event add_kernel_event;
    kernelQueue.enqueueNDRangeKernel(
            add_kernel, cl::NullRange, cl::NDRange(1), cl::NullRange,
            &add_kernel_event_wait_list,
            &add_kernel_event);

    // cl_event output_map_cl_event;
    // err = clEnqueueSVMMap(utilitiesQueue.get(), CL_TRUE, CL_MEM_WRITE_ONLY | CL_MEM_SVM_FINE_GRAIN_BUFFER, output_ptr, output_ptr_size, 0, nullptr, &output_map_cl_event);
    // std::cout << "clEnqueueSVMMap: " << err << ", " << (err == CL_SUCCESS ? "SUCCESS!" : "ERROR!") << std::endl;

    std::vector<cl::Event> output_map_event_wait_list{add_kernel_event};
    cl::Event output_map_event;
    utilitiesQueue.enqueueMapSVM(
            output,
            false,
            CL_MEM_WRITE_ONLY | CL_MEM_SVM_FINE_GRAIN_BUFFER,
            &output_map_event_wait_list,
            &output_map_event);

    output_map_event.wait();

    std::cerr << "output: "; print_range(output);

    std::cout << "reached end of file1 (cout)" << std::endl;
    std::cerr << "reached end of file2 (cerr)" << std::endl;

    check_result(output);

    // clSVMFree(context.get(), output_ptr);

    utilitiesQueue.finish();
    kernelQueue.finish();

    std::cout.flush();
    std::cerr.flush();

    return 0;
}