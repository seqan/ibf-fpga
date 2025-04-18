

#include "fixtures/all.hpp"

#include <min_ibf_fpga/backend_opencl/opencl.hpp>
#include <min_ibf_fpga/backend_opencl/device_selector.hpp>
#include <min_ibf_fpga/backend_opencl/kernel_loader.hpp>

// #include "CL/cl_ext_intelfpga.h"

struct minimizer_pipe_t//__attribute__((__packed__))
{
    bool isLastElement{false};
    uint64_t hash{};
};

void opencl_test(cl::Program const & program, cl::Context const & context, std::vector<cl::Device> const & selected_devices, minimizer_test_fixture test)
{
    cl::CommandQueue minimizer_kernel_queue{context, selected_devices.at(0), 0};
    cl::CommandQueue minimizer_kernel_test_queue{context, selected_devices.at(0), 0};

    char const * queries = test.query.c_str();
    size_t const queriesOffset = 0;
    size_t const querySizes[] = {test.query.size()};
    size_t const querySizesOffset = 0;
    size_t const numberOfQueries = 1;
    std::vector<minimizer_pipe_t> pipe_results(test.query.size(), minimizer_pipe_t{});
    std::vector<uint64_t> minimizer{};

    cl::Kernel minimizer_kernel(program, "minimizer0");
    minimizer_kernel.setArg(0, queries);
    minimizer_kernel.setArg(1, queriesOffset);
    minimizer_kernel.setArg(2, (size_t const *) querySizes);
    minimizer_kernel.setArg(3, querySizesOffset);
    minimizer_kernel.setArg(4, numberOfQueries);

    cl::Kernel minimizer_test_kernel(program, "minimizer0_test");
    minimizer_test_kernel.setArg(0, pipe_results.data());

    cl::Event minimizer_kernel_event;
    minimizer_kernel_queue.enqueueNDRangeKernel(
            minimizer_kernel, cl::NullRange, cl::NDRange(1), cl::NullRange,
            nullptr,
            &minimizer_kernel_event);

    cl::Event minimizer_test_kernel_event;
    minimizer_kernel_test_queue.enqueueNDRangeKernel(
            minimizer_test_kernel, cl::NullRange, cl::NDRange(1), cl::NullRange,
            nullptr,
            &minimizer_test_kernel_event);

    minimizer_kernel_event.wait();
    minimizer_test_kernel_event.wait();

    minimizer_pipe_t pipe_result{};
    for (int i = 0; i < pipe_results.size() && !pipe_result.isLastElement; ++i)
    {
        pipe_result = pipe_results[i];
        minimizer.push_back(pipe_result.hash);
    }

    test.compare_minimizer(minimizer);
}

int main()
{
    std::vector<cl::Device> all_devices = min_ibf_fpga::backend_opencl::device_selector::select_devices();
    std::vector<cl::Device> selected_devices{{all_devices.at(0)}};
    cl::Context context{selected_devices};
    cl::CommandQueue utilitiesQueue{context, selected_devices.at(0), 0};
    cl::CommandQueue::setDefault(utilitiesQueue);

    std::filesystem::path program_w23_k19_file{"ibf_w23_k19_h2_b64.aocx"};
    cl::Program program_w23_k19 = min_ibf_fpga::backend_opencl::kernel_loader::load(program_w23_k19_file, context, selected_devices);

    opencl_test(program_w23_k19, context, selected_devices, minimizer_w23_k19_query0_test);
    opencl_test(program_w23_k19, context, selected_devices, minimizer_w23_k19_query1_test);
    opencl_test(program_w23_k19, context, selected_devices, minimizer_w23_k19_query2_test);
    opencl_test(program_w23_k19, context, selected_devices, minimizer_w23_k19_query3_test);
    opencl_test(program_w23_k19, context, selected_devices, minimizer_w23_k19_query4_test);

    std::filesystem::path program_w19_k19_file{"ibf_w19_k19_h2_b64.aocx"};
    cl::Program program_w19_k19 = min_ibf_fpga::backend_opencl::kernel_loader::load(program_w19_k19_file, context, selected_devices);

    opencl_test(program_w19_k19, context, selected_devices, minimizer_w19_k19_query0_test);
    opencl_test(program_w19_k19, context, selected_devices, minimizer_w19_k19_query1_test);
    opencl_test(program_w19_k19, context, selected_devices, minimizer_w19_k19_query2_test);
    opencl_test(program_w19_k19, context, selected_devices, minimizer_w19_k19_query3_test);
    opencl_test(program_w19_k19, context, selected_devices, minimizer_w19_k19_query4_test);

    return 0;
}