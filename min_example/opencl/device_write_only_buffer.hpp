#pragma once

#include <iostream>

#include "cl_event_profiling.hpp"

struct device_write_only_buffer
{
    std::string_view buffer_name;

    void * host_ptr;
    size_t host_size;
    size_t host_size_bytes;

    cl::Buffer cl_device_buffer;
    cl::Event cl_device_buffer_transfer_event;

    cl_event_profiling profiling;

    template <typename value_type>
    device_write_only_buffer(std::string_view buffer_name, value_type * host_ptr, size_t const host_size, cl::Context & context) :
        buffer_name{buffer_name},
        host_ptr{(void *)host_ptr},
        host_size{host_size},
        host_size_bytes{host_size * sizeof(value_type)},
        cl_device_buffer{context, CL_MEM_WRITE_ONLY, host_size_bytes}
    {}

    void register_profiling()
    {
        std::string str = "Transfered " + std::string{buffer_name} + ": " + std::to_string(host_size_bytes) + " bytes";
        profiling = {str, cl_device_buffer_transfer_event};
    }

    void transfer2host(cl::CommandQueue & utilitiesQueue)
    {
        std::cerr << "Transfer " << buffer_name << ": " << host_size_bytes << " bytes" << std::endl;
        utilitiesQueue.enqueueReadBuffer(
            cl_device_buffer, CL_FALSE, 0,
            host_size_bytes, host_ptr,
            nullptr, &cl_device_buffer_transfer_event);
    }

    void wait()
    {
        cl_device_buffer_transfer_event.wait();
    }
};