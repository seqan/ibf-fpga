
#include "vector_mul.hpp"

#include <sycl/sycl.hpp>

class VectorMulID;

extern "C"
{

void mul(
  int *const vec_a,
  int *const vec_b,
  int *const vec_c,
  int kVectSize,
  sycl::queue & queueA
)
{
  queueA.single_task<VectorMulID>(VectorMul{vec_a, vec_b, vec_c, kVectSize}).wait();
}

}