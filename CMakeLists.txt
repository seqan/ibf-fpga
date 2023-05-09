if(UNIX)
    # Direct CMake to use icpx rather than the default C++ compiler/linker
    set(CMAKE_CXX_COMPILER icpx)
else() # Windows
    # Force CMake to use icx-cl rather than the default C++ compiler/linker
    # (needed on Windows only)
    include (CMakeForceCompiler)
    CMAKE_FORCE_CXX_COMPILER (icx-cl IntelDPCPP)
    include (Platform/Windows-Clang)
endif()

cmake_minimum_required (VERSION 3.4)

project(MinIBFFPGAoneAPI CXX)

add_subdirectory(src)
add_subdirectory(min_example/host_device_split_code)
add_subdirectory(test)