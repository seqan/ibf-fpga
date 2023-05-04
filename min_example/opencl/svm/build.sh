
source /opt/intel/oneapi/setvars.sh

source_dir=../../../..
compile_options=`aocl compile-config`
# link_options=`aocl link-config`
link_options="-l OpenCL"

ioc64 --device=fpga_fast_emu --cmd=compile --input=./add_kernel.cl --ir=add_kernel.aocx

g++ -I"${source_dir}/include" host.cpp $compile_options $link_options -O0 -g -o ./host

stdbuf -o 0 -e 0 ./host
