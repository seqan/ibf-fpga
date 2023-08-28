
###############################################################################
### Generate Report
###############################################################################
sycl_fpga_report_of(run_min_ibf_fpga_w16_k8.fpga_kernel)
sycl_fpga_report_of(run_min_ibf_fpga_w16_k6.fpga_kernel)
sycl_fpga_report_of(run_min_ibf_fpga_w18_k6.fpga_kernel)

add_custom_target(host_device_split_code.report)
add_dependencies(host_device_split_code.report run_min_ibf_fpga_w16_k8.fpga_kernel.report.html)
add_dependencies(host_device_split_code.report run_min_ibf_fpga_w16_k6.fpga_kernel.report.html)
add_dependencies(host_device_split_code.report run_min_ibf_fpga_w18_k6.fpga_kernel.report.html)
