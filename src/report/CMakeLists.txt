###############################################################################
### Generate Report
###############################################################################
# To compile manually:
#   icpx -fsycl -fintelfpga -qactypes -Xshardware -Xstarget=<FPGA_DEVICE> -fsycl-link=early host.cpp kernel/kernel.cpp -o fast_compile_report.a
set(REPORT_NAME "min-ibf-fpga-oneapi_report")
# The compile output is not an executable, but an intermediate compilation result unique to SYCL.
add_executable(${REPORT_NAME} ../host.cpp ../fpga_device.cpp)
target_include_directories(${REPORT_NAME} PRIVATE ../../include)
target_link_libraries(${REPORT_NAME} PRIVATE intel_fpga_device)
target_link_options(${REPORT_NAME} PRIVATE -fsycl-link=early)
# fsycl-link=early stops the compiler after RTL generation, before invoking Quartus®

cmake_policy(PUSH)
# https://cmake.org/cmake/help/latest/policy/CMP0112.html#policy:CMP0112
# Target file component generator expressions do not add target dependencies.
# CMake 3.19 and above prefer to not add this dependency.
cmake_policy(SET CMP0112 NEW)
set(report_html_full_path "$<TARGET_FILE_DIR:${REPORT_NAME}>/$<TARGET_FILE_BASE_NAME:${REPORT_NAME}>.prj/reports/report.html")
add_custom_target(
    "min-ibf-fpga-oneapi.report.html"
    ALL
    COMMAND
        ${CMAKE_COMMAND} -E
        create_symlink
            "'${report_html_full_path}'"
            "'min-ibf-fpga-oneapi.report.html'"
    COMMENT "Created min-ibf-fpga-oneapi.report.html"
    DEPENDS "${REPORT_NAME}"
)
cmake_policy(POP)

add_custom_target(report DEPENDS ${REPORT_NAME})