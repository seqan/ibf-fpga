cmake_minimum_required(VERSION 3.13)

project(dynamic_linking_flow CXX)

foreach(build_type emu hwd)
add_library(vector_add.${build_type} SHARED vector_add.cpp)
target_compile_options(vector_add.${build_type} PRIVATE -fsycl -fintelfpga)
target_link_options(vector_add.${build_type} PRIVATE -fsycl -fintelfpga)

add_library(vector_mul.${build_type} SHARED vector_mul.cpp)
target_compile_options(vector_mul.${build_type} PRIVATE -fsycl -fintelfpga)
target_link_options(vector_mul.${build_type} PRIVATE -fsycl -fintelfpga)

add_executable(main.${build_type} main.cpp)
target_link_libraries(main.${build_type} PRIVATE vector_add.${build_type} vector_mul.${build_type})
target_compile_options(main.${build_type} PRIVATE -fsycl)
target_link_options(main.${build_type} PRIVATE -fsycl)
endforeach()

target_compile_definitions(main.emu PRIVATE -DFPGA_EMULATOR)

target_link_options(vector_add.hwd PRIVATE -Xshardware -Xstarget=intel_s10sx_pac:pac_s10_usm)
target_link_options(vector_mul.hwd PRIVATE -Xshardware -Xstarget=intel_s10sx_pac:pac_s10_usm)
target_compile_definitions(main.hwd PRIVATE -DFPGA_HARDWARE)
