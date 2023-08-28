# Common Compilation Flags of host and device code
# This is a Windows-specific flag that enables exception handling in host code
if(WIN32)
    set(WIN_FLAG "/EHsc")
endif()

add_library(fpga_compilation_base INTERFACE)
target_compile_options(fpga_compilation_base INTERFACE -Wall "${WIN_FLAG}")

# NOTE: -fsycl -fintelfpga is needed for the host code as well; Maybe because of the extension sycl::ext::intel::fpga_selector?
# It compiles without, but an runtime error occurs
add_library(intel_fpga INTERFACE)
target_compile_options(intel_fpga INTERFACE -fsycl -fintelfpga)
target_link_options(intel_fpga INTERFACE -fsycl -fintelfpga)

add_subdirectory(emulator)
add_subdirectory(fpga EXCLUDE_FROM_ALL)
add_subdirectory(report)