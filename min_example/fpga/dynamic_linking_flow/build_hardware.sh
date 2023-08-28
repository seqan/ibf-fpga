intelfpga="-fintelfpga"
fpga_hardware="-Xshardware -Xstarget=intel_s10sx_pac:pac_s10_usm"

set -x

export QUARTUS_ROOTDIR_OVERRIDE=/opt/intel/intelFPGA_pro/21.4/quartus/

icpx -fsycl -fPIC ${intelfpga} -c vector_add.cpp -o vector_add.hwd.o
icpx -fsycl -fPIC ${intelfpga} -c vector_mul.cpp -o vector_mul.hwd.o

# FPGA image compiles take a long time to complete
icpx -fsycl -fPIC -shared ${intelfpga} vector_add.hwd.o -o vector_add.hwd.so ${fpga_hardware} &
icpx -fsycl -fPIC -shared ${intelfpga} vector_mul.hwd.o -o vector_mul.hwd.so ${fpga_hardware} &

wait
cp vector_add.hwd.so{,.bak}
cp vector_mul.hwd.so{,.bak}


# Final link step
icpx -fsycl -DFPGA_HARDWARE -o main.hwd main.cpp ./vector_add.hwd.so ./vector_mul.hwd.so