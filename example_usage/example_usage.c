#define XLEN 256 // XLEN = 128, if not defined before include
#include "notasimdlib.h"

// Declare types
decl_simd_t(float)
decl_simd_t(int)

// Declare custom binop's
decl_simd_bin_op(add, float,  +)
decl_simd_bin_op(add, int,  +)

// Custom function for reduce...
float dot_func(float accum, int idx, simd_t(float) a, simd_t(float) b){
    return accum + (a.v[idx] * b.v[idx]);
}

int main(){
    // Declaring Float types:
    simd_t(float) veca;
    simd_t(float) vecb;

    // Declaring Int types:
    simd_t(int) iveca;
    simd_t(int) ivecb;

    // "Internal" Binary Ops
    simd_t(float) internal_add = simd_apply_add(float, veca, vecb);
    simd_t(float) internal_sub = simd_apply_sub(float, veca, vecb);
    simd_t(float) internal_mul = simd_apply_mul(float, veca, vecb);
    simd_t(float) internal_div = simd_apply_div(float, veca, vecb);

    // "Custom" Float binary ops
    simd_t(float) vecc = simd_bin_op(add, float, veca, vecb);

    // "Custom" Integer binary ops
    simd_t(int) ivecc = simd_bin_op(add, int, iveca, ivecb);
    
    // Example: Some Ways to compute dot product...

    // Using internal implementation
    float dot_internal_impl = simd_apply_dot(float, veca, vecb);

    // Using function reduce
    float dot_func_reduce = simd_reduce_func(float, dot_func, veca, vecb);

    // Using an expression reduce
    float dot_expr_reduce = simd_reduce_expr(float, (accum + (veca.v[i] * vecb.v[i])), veca, vecb);

    // Using sum(mul(a,b))
    float dot_mul_sum = simd_apply_sum(float, simd_apply_mul(float, veca, vecb)); // == internal impl.

    return 0;
}
