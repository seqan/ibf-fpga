
#pragma once

#include <dlfcn.h>

#include <filesystem>
#include <functional>
#include <string_view>

struct sycl_kernel_loader_base
{
    sycl_kernel_loader_base() = delete;
    sycl_kernel_loader_base(std::filesystem::path library_file_path, std::string_view const symbol_name)
    {
        handle = dlopen(library_file_path.c_str(), RTLD_LAZY);

        if (!handle) {
            throw std::runtime_error{dlerror()};
        }

        // Clear any existing error
        dlerror();

        char * error_message;

        std::string const symbol_name_str{symbol_name};
        function_ptr = dlsym(handle, symbol_name_str.c_str());

        if ((error_message = dlerror()) != NULL) {
            throw std::runtime_error{error_message};
        }
    }

    sycl_kernel_loader_base(sycl_kernel_loader_base const &) = delete;
    sycl_kernel_loader_base(sycl_kernel_loader_base && other)
        : handle{other.handle}, function_ptr{other.function_ptr}
    {
        other.handle = nullptr;
        other.function_ptr = nullptr;
    }
    sycl_kernel_loader_base & operator=(sycl_kernel_loader_base const &) = delete;
    sycl_kernel_loader_base & operator=(sycl_kernel_loader_base && other)
    {
        handle = other.handle;
        function_ptr = other.function_ptr;
        other.handle = nullptr;
        other.function_ptr = nullptr;
        return *this;
    }

    virtual ~sycl_kernel_loader_base()
    {
        if (handle)
        {
            dlclose(handle);
        }
    }

    template <typename function_signature_t>
    std::function<std::remove_pointer_t<function_signature_t>> get_function() const
    {
        function_signature_t * fn_ptr;
        *(void **) (&fn_ptr) = this->get();

        if constexpr (std::is_pointer_v<function_signature_t>) {
            // void (*)(int, int) signature
            return {*fn_ptr};
        } else {
            // void (int, int) signature
            return {fn_ptr};
        }
    }

    void * get() const
    {
        return function_ptr;
    }

private:
    void * handle{nullptr};
    void * function_ptr{nullptr};
};

template <typename function_signature_t>
struct sycl_kernel_loader;

template <typename return_t, typename ...args_t>
struct sycl_kernel_loader<return_t(args_t...)> : sycl_kernel_loader_base
{
    using sycl_kernel_loader_base::sycl_kernel_loader_base;

    return_t operator()(args_t... args) const
    {
        return this->get_function<return_t(args_t...)>()(args...);
    }
};


template <typename return_t, typename ...args_t>
struct sycl_kernel_loader<return_t(*)(args_t...)> : sycl_kernel_loader_base
{
    using sycl_kernel_loader_base::sycl_kernel_loader_base;

    return_t operator()(args_t... args) const
    {
        return this->get_function<return_t(*)(args_t...)>()(args...);
    }
};