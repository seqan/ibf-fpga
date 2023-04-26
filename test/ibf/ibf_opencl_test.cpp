

#include "fixtures/all.hpp"

#include <min_ibf_fpga/backend_opencl/opencl.hpp>
#include <min_ibf_fpga/backend_opencl/device_selector.hpp>
#include <min_ibf_fpga/backend_opencl/kernel_loader.hpp>
#include <min_ibf_fpga/index/ibf_metadata.hpp>
#include <min_ibf_fpga/index/ibf_data.hpp>

#include <min_ibf_fpga/test/assert_loaded_ibf.hpp>
#include <min_ibf_fpga/test/load_ibf_index.hpp>

void opencl_test(min_ibf_fpga::index::ibf_metadata const & ibf_meta, min_ibf_fpga::index::ibf_data const & ibf, cl::Program const & program, cl::Context const & context, std::vector<cl::Device> const & selected_devices, ibf_test_fixture const & test)
{
    cl::CommandQueue ibf_test_provider_kernel_queue{context, selected_devices.at(0), 0};
    cl::CommandQueue ibf_kernel_queue{context, selected_devices.at(0), 0};

    std::vector<uint64_t> minimizer = test.minimizer();

    cl::Buffer const ibfData(context, CL_MEM_READ_ONLY, ibf.size() * sizeof(size_t));
    ibf_kernel_queue.enqueueWriteBuffer(ibfData, CL_TRUE, 0, (ibf.size() * sizeof(size_t)), ibf.data(), nullptr, nullptr);

    auto compute_bin_containment_from_minimizer = [&](size_t const errors) -> std::bitset<64>
    {
        size_t const binSize = ibf_meta.bin_size();
        size_t const hashShift = ibf_meta.hash_shift();
        size_t const numberOfQueries = 1;
        size_t const minimalNumberOfMinimizers = test.minimizer_range.minimal;
        size_t const maximalNumberOfMinimizers = test.minimizer_range.maximal;
        min_ibf_fpga::threshold::threshold_table thresholds = test.threshold_table_per_error[errors];
        std::vector<size_t> result(numberOfQueries, 0);

        cl::Kernel ibf_test_provider_kernel(program, "ibf0_test_provider");
        ibf_test_provider_kernel.setArg(0, minimizer.size());
        ibf_test_provider_kernel.setArg(1, minimizer.data());

        cl::Kernel ibf_kernel(program, "ibf0");
        ibf_kernel.setArg(0, ibfData);
        ibf_kernel.setArg(1, binSize);
        ibf_kernel.setArg(2, hashShift);
        ibf_kernel.setArg(3, numberOfQueries);
        ibf_kernel.setArg(4, minimalNumberOfMinimizers);
        ibf_kernel.setArg(5, maximalNumberOfMinimizers);
        ibf_kernel.setArg(6, thresholds.table);
        ibf_kernel.setArg(7, result);

        cl::Event ibf_test_provider_kernel_event;
        ibf_test_provider_kernel_queue.enqueueNDRangeKernel(
                ibf_test_provider_kernel, cl::NullRange, cl::NDRange(1), cl::NullRange,
                nullptr,
                &ibf_test_provider_kernel_event);

        cl::Event ibf_kernel_event;
        ibf_kernel_queue.enqueueNDRangeKernel(
                ibf_kernel, cl::NullRange, cl::NDRange(1), cl::NullRange,
                nullptr,
                &ibf_kernel_event);

        ibf_test_provider_kernel_event.wait();
        ibf_kernel_event.wait();

        // TODO: this needs adjustment if bin_count > 64
        if (ibf_meta.bin_count() > 64) throw std::runtime_error{"NEEDS TO BE FIXED!!!!"};

        std::bitset<64> bin_bit_membership{result[0]};
        return bin_bit_membership;
    };

    // std::map<size_t, std::bitset<64>> minimizer_bit_membership_actual{};
    // std::array<size_t, 64> counter_actual{};
    std::vector<std::bitset<64>> bin_bit_membership_per_error{};
    for (size_t error = 0; error < test.threshold_per_error.size(); ++error)
    {
        std::bitset<64> bin_bit_membership = compute_bin_containment_from_minimizer(error);

        bin_bit_membership_per_error.push_back(bin_bit_membership);
        std::cout << "bin_bit_membership: " << bin_bit_membership << std::endl;
    }

    // test.compare_minimizer_membership(minimizer_bit_membership_actual);
    // test.compare_counter(counter_actual);
    test.compare_bin_bit_membership(bin_bit_membership_per_error);
}

int main()
{
    // raptor_23_19.index
    size_t const bin_count{64};
    size_t const bin_size{1024};

    auto [ibf_w23_k19_meta, ibf_w23_k19] = min_ibf_fpga::test::load_ibf_index("raptor_23_19.index");
    auto [ibf_w19_k19_meta, ibf_w19_k19] = min_ibf_fpga::test::load_ibf_index("raptor_19_19.index");

    min_ibf_fpga::test::assert_loaded_ibf(ibf_w23_k19_meta, min_ibf_fpga::test::ibf_meta_assertion_t {
        .bin_count = bin_count,
        .bin_size = bin_size
    });

    min_ibf_fpga::test::assert_loaded_ibf(ibf_w19_k19_meta, min_ibf_fpga::test::ibf_meta_assertion_t {
        .bin_count = bin_count,
        .bin_size = bin_size
    });

    std::vector<cl::Device> all_devices = min_ibf_fpga::backend_opencl::device_selector::select_devices();
    std::vector<cl::Device> selected_devices{{all_devices.at(0)}};
    cl::Context context{selected_devices};
    cl::CommandQueue utilitiesQueue{context, selected_devices.at(0), 0};
    cl::CommandQueue::setDefault(utilitiesQueue);

    std::filesystem::path program_w23_k19_file{"ibf_w23_k19_h2_b64.aocx"};
    std::filesystem::path program_w19_k19_file{"ibf_w19_k19_h2_b64.aocx"};

    cl::Program program_w23_k19 = min_ibf_fpga::backend_opencl::kernel_loader::load(program_w23_k19_file, context, selected_devices);
    cl::Program program_w19_k19 = min_ibf_fpga::backend_opencl::kernel_loader::load(program_w19_k19_file, context, selected_devices);

    opencl_test(ibf_w23_k19_meta, ibf_w23_k19, program_w23_k19, context, selected_devices, ibf_w23_k19_query0_test);
    opencl_test(ibf_w23_k19_meta, ibf_w23_k19, program_w23_k19, context, selected_devices, ibf_w23_k19_query1_test);
    opencl_test(ibf_w23_k19_meta, ibf_w23_k19, program_w23_k19, context, selected_devices, ibf_w23_k19_query2_test);
    opencl_test(ibf_w23_k19_meta, ibf_w23_k19, program_w23_k19, context, selected_devices, ibf_w23_k19_query3_test);
    opencl_test(ibf_w23_k19_meta, ibf_w23_k19, program_w23_k19, context, selected_devices, ibf_w23_k19_query4_test);

    opencl_test(ibf_w19_k19_meta, ibf_w19_k19, program_w19_k19, context, selected_devices, ibf_w19_k19_query0_test);
    opencl_test(ibf_w19_k19_meta, ibf_w19_k19, program_w19_k19, context, selected_devices, ibf_w19_k19_query1_test);
    opencl_test(ibf_w19_k19_meta, ibf_w19_k19, program_w19_k19, context, selected_devices, ibf_w19_k19_query2_test);
    opencl_test(ibf_w19_k19_meta, ibf_w19_k19, program_w19_k19, context, selected_devices, ibf_w19_k19_query3_test);
    opencl_test(ibf_w19_k19_meta, ibf_w19_k19, program_w19_k19, context, selected_devices, ibf_w19_k19_query4_test);

    return 0;
}