intelfpga="-fintelfpga"
# fpga_hardware="-Xshardware -Xstarget=intel_s10sx_pac:pac_s10_usm"

set -x

icpx -fsycl -fPIC ${intelfpga} -c vector_add.cpp -o vector_add.emu.o
icpx -fsycl -fPIC ${intelfpga} -c vector_mul.cpp -o vector_mul.emu.o

# FPGA image compiles take a long time to complete
icpx -fsycl -fPIC -shared ${intelfpga} vector_add.emu.o -o vector_add.emu.so ${fpga_hardware} &
icpx -fsycl -fPIC -shared ${intelfpga} vector_mul.emu.o -o vector_mul.emu.so ${fpga_hardware} &

wait

# Final link step
icpx -fsycl -DFPGA_EMULATOR -o main.emu main.cpp ./vector_add.emu.so ./vector_mul.emu.so