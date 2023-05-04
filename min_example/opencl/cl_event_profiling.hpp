#pragma once

#include <string>
#include <iostream>

#include <ibf_fpga/opencl/opencl.hpp>

struct cl_event_profiling
{
    std::string event_message;

    cl_event_profiling() = default;

    cl_event_profiling(std::string const & event_message, cl::Event & event) :
        event_message{event_message}
    {
        void * self_ptr = this;
        event.setCallback(CL_COMPLETE,
            [](cl_event event, cl_int, void * self_ptr)
            {
                auto & self = *((cl_event_profiling*) self_ptr);
                std::cerr << self.event_message << "\t"
                    << getRuntime(cl::Event(event, true))
                    << " ms\n";
            },
            self_ptr);
    }

    template <class Events>
    static double getRuntime(const Events& events)
    {
        const auto comparator =
            [](const auto& lhs, const auto& rhs, const auto flag)
            {
                cl_ulong lhsValue, rhsValue;

                lhs.getProfilingInfo(flag, &lhsValue);
                rhs.getProfilingInfo(flag, &rhsValue);

                return lhsValue < rhsValue;
            };

        const auto startEvent = *std::min_element(events.begin(), events.end(),
            [&](const auto& lhs, const auto& rhs)
            {
                return comparator(lhs, rhs, CL_PROFILING_COMMAND_START);
            });
        const auto endEvent = *std::max_element(events.begin(), events.end(),
            [&](const auto& lhs, const auto& rhs)
            {
                return comparator(lhs, rhs, CL_PROFILING_COMMAND_END);
            });

        return getRuntime(startEvent, endEvent);
    };

    static double getRuntime(const cl::Event& startEvent, const cl::Event& endEvent)
    {
        using timeUnit = std::chrono::duration<double, std::milli>;

        cl_ulong start, end;

        startEvent.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
        endEvent.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);

        const std::chrono::nanoseconds duration{end - start};

        return std::chrono::duration_cast<timeUnit>(duration).count();
    }

    static double getRuntime(const cl::Event& event)
    {
        return getRuntime(event, event);
    }
};