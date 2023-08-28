
#include "vector_add.hpp"

#include <sycl/sycl.hpp>

class VectorAddID;

extern "C"
{

void add(
  int *const vec_a,
  int *const vec_b,
  int *const vec_c,
  int kVectSize,
  sycl::queue & queueA
)
{
  queueA.single_task<VectorAddID>(VectorAdd{vec_a, vec_b, vec_c, kVectSize}).wait();
}

}