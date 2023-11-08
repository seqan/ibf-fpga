# min-ibf-fpga-oneapi

- SYCL adaption of [min-ibf-fpga](https://git.zib.de/hpc-research/Projects/seqanatfpga/min-ibf-fpga)
- includes tests for OpenCL and SYCL variant

### clone:

```
git clone --recurse-submodules git@git.zib.de:hpc-research/Projects/seqanatfpga/min-ibf-fpga-oneapi.git
```

### build:

```
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=icpx ..
make
```
### run tests:

```
cd build/test
ctest --output-on-failure
```


## Folder Structure

- `src` - minimalistic SYCL host code
- `include/min_ibf_fpga/backend_sycl` - SYCL utilities and device code
- `include/min_ibf_fpga/backend_opencl` - OpenCL utilities
- `test` - SYCL and OpenCL tests

## Notes

- A comparable minimalistic OpenCL host code can be found in [min-ibf-fpga](https://git.zib.de/hpc-research/Projects/seqanatfpga/min-ibf-fpga)
- OpenCL tests require device code from [min-ibf-fpga](https://git.zib.de/hpc-research/Projects/seqanatfpga/min-ibf-fpga)
- IBF tests require [SeqAn3](https://github.com/seqan/seqan3), i.e. `cmake -DSEQAN3_ROOT_DIR=/path/to/seqan3 ..`
