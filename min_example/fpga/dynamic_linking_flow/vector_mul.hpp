#pragma once

struct VectorMul {
  int *const vec_a_in;
  int *const vec_b_in;
  int *const vec_c_out;
  int len;

  void operator()() const {
    for (int idx = 0; idx < len; idx++) {
      int a_val = vec_a_in[idx];
      int b_val = vec_b_in[idx];
      int mul = a_val * b_val;
      vec_c_out[idx] = mul;
    }
  }
};