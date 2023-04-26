cmake_minimum_required (VERSION 3.4)

project(FastRecompile CXX)

find_path (INTEL_DPCPP_CONFIG_ROOT
    NAMES IntelDPCPPConfig.cmake
    HINTS
        /opt/intel/oneapi/compiler/latest/linux/IntelDPCPP
        # /sw/compiler/intel/oneapi/mkl/latest/
)

find_package (
    IntelDPCPP # 202001
    REQUIRED
    HINTS "${INTEL_DPCPP_CONFIG_ROOT}"
)

add_subdirectory (src)
