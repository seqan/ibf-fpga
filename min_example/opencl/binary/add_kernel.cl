typedef int int32_t;
typedef unsigned long uint64_t;

kernel void add_range(
    uint64_t const size,
    __global int32_t const * const restrict input1_ptr,
    __global int32_t const * const restrict input2_ptr,
    __global int32_t * const restrict output_ptr
)
{
    for (int i = 0; i < size; ++i)
    {
        output_ptr[i] = input1_ptr[i] + input2_ptr[i];
    }
}