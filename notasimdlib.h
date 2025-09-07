#include <stdint.h>

/* -------------------------------------------------------------------------
 * Macro concatenation utilities
 * ------------------------------------------------------------------------- */

/**
 * @brief Concatenate two tokens without expanding macros.
 *
 * @param A First token
 * @param B Second token
 * @return Concatenated token "AB"
 *
 * Example:
 *   PPCAT_NX(foo,bar) → foobar
 */
#define PPCAT_NX(A, B) A##B

/**
 * @brief Concatenate two tokens with macro expansion.
 *
 * @param A First token (expanded if macro)
 * @param B Second token (expanded if macro)
 * @return Concatenated token
 *
 * Example:
 *   #define X foo
 *   PPCAT(X,bar) → foobar
 */
#define PPCAT(A, B) PPCAT_NX(A, B)

/* -------------------------------------------------------------------------
 * SIMD vector configuration
 * ------------------------------------------------------------------------- */

/**
 * @brief Bit-width of the simulated SIMD register.
 *
 * Default is 128 bits if not explicitly defined by the user.
 */
#ifndef XLEN
#define XLEN 128
#endif

/**
 * @brief Number of elements of type T that fit into XLEN bits.
 *
 * @tparam T Scalar type (e.g., float, int32_t)
 * @return Number of elements
 *
 * Example:
 *   VLEN(float) → 128 / (8*4) = 4
 */
#define VLEN(T) (XLEN / (8 * sizeof(T)))

/* -------------------------------------------------------------------------
 * SIMD type declaration
 * ------------------------------------------------------------------------- */

/**
 * @brief Produce the name of the SIMD type for a given element type T.
 *
 * @tparam T Scalar type
 * @return Type name of form simd_v{T}{XLEN}_t
 *
 * Example:
 *   simd_t(float) → simd_vfloat128_t
 */
#define simd_t(T) PPCAT(simd_,PPCAT(v,PPCAT(T,PPCAT(XLEN,_t))))

/**
 * @brief Declare a SIMD type with elements of type T.
 *
 * @tparam T Scalar type (float, int32_t, etc.)
 *
 * Declares a struct with:
 *   - Name: simd_v{T}{XLEN}_t
 *   - Members: array v[VLEN(T)] of type T
 *
 * Example:
 *   decl_simd_t(float) →
 *   typedef struct { float v[4]; } simd_vfloat128_t;
 */
#define decl_simd_t(T) \
typedef struct simd_t(T) { \
    T v[VLEN(T)]; \
} simd_t(T);

/* -------------------------------------------------------------------------
 * SIMD operation naming
 * ------------------------------------------------------------------------- */

/**
 * @brief Generate the function name for a SIMD operation.
 *
 * @param T Scalar type
 * @param name Operation name
 * @return Function name "{name}_simd_v{T}{XLEN}_t"
 *
 * Example:
 *   simd_op_name(float, add) → add_simd_vfloat128_t
 */
#define simd_op_name(T,name) PPCAT(name,PPCAT(_,simd_t(T)))

/* -------------------------------------------------------------------------
 * SIMD binary operations
 * ------------------------------------------------------------------------- */

/**
 * @brief Invoke a binary SIMD operation function.
 *
 * @param name Operation name
 * @param T Scalar type
 * @param a First SIMD operand (simd_t(T))
 * @param b Second SIMD operand (simd_t(T))
 * @return Resulting SIMD vector (simd_t(T))
 *
 * Example:
 *   simd_bin_op(add,float,x,y) → add_simd_vfloat128_t(x,y)
 */
#define simd_bin_op(name, T, a, b) simd_op_name(T,name) (a, b)

/**
 * @brief Define a binary SIMD operation function with a given operator.
 *
 * @param name Operation name (used in function name)
 * @param T Scalar type
 * @param op Binary operator symbol (+, -, *, /)
 *
 * Declares a function:
 *   simd_t(T) name_simd_v{T}{XLEN}_t(simd_t(T) a, simd_t(T) b)
 * that applies `a.v[i] op b.v[i]` elementwise.
 *
 * Example:
 *   decl_simd_bin_op(add,float,+)
 *   → defines add_simd_vfloat128_t(a,b) that adds elementwise.
 */
#define decl_simd_bin_op(name, T, op) \
simd_t(T) simd_op_name(T,name) (simd_t(T) a, simd_t(T) b) { \
    simd_t(T) c; \
    for (int i = 0; i < VLEN(T); i++) { \
        c.v[i] = a.v[i] op b.v[i]; \
    } \
    return c; \
}

/* -------------------------------------------------------------------------
 * SIMD reductions
 * ------------------------------------------------------------------------- */

/**
 * @brief Reduce a SIMD vector using a custom function.
 *
 * @tparam T Scalar type
 * @param func Function of form func(T accum, int i, args...)
 * @param ... Additional arguments passed to func
 * @return Reduced scalar value of type T
 *
 * Example:
 *   simd_reduce_func(float, my_func, vec)
 *   where my_func(accum, i, vec) → accum + vec.v[i]
 */
#define simd_reduce_func(T,func,...) \
({ \
    T accum = 0; \
    for (int i = 0; i < VLEN(T); i++) { \
        accum = func(accum,i,__VA_ARGS__); \
    } \
    accum; \
})

/**
 * @brief Reduce a SIMD vector using a custom expression.
 *
 * @tparam T Scalar type
 * @param expr Expression involving (accum, i, ...)
 * @param ... Extra arguments (e.g., vectors)
 * @return Reduced scalar value
 *
 * Example:
 *   simd_reduce_expr(float, accum + vec.v[i], vec)
 */
#define simd_reduce_expr(T,expr,...) \
({ \
    T accum = 0; \
    for (int i = 0; i < VLEN(T); i++) { \
        accum = expr; \
    } \
    accum; \
})

/**
 * @brief Sum all elements of a SIMD vector.
 *
 * @tparam T Scalar type
 * @param a SIMD vector (simd_t(T))
 * @return Scalar sum of elements
 *
 * Example:
 *   simd_apply_sum(float, vec) → sum of vec.v[]
 */
#define simd_apply_sum(T, a) \
({ \
    T accum = 0; \
    for (int i = 0; i < VLEN(T); i++) { \
        accum += (a).v[i]; \
    } \
    accum; \
})

/* -------------------------------------------------------------------------
 * SIMD elementwise operations
 * ------------------------------------------------------------------------- */

/**
 * @brief Apply a binary operator elementwise on two SIMD vectors.
 *
 * @tparam T Scalar type
 * @param a First SIMD operand
 * @param b Second SIMD operand
 * @param op Binary operator (+, -, *, /)
 * @return SIMD vector result
 */
#define simd_apply_binop(T, a, b, op) \
({ \
    simd_t(T) c; \
    for (int i = 0; i < VLEN(T); i++) { \
        c.v[i] = (a).v[i] op (b).v[i]; \
    } \
    c; \
})

/**
 * @brief Elementwise addition of two SIMD vectors.
 *
 * @tparam T Scalar type
 * @param a First SIMD operand
 * @param b Second SIMD operand
 * @return SIMD vector where each element is (a.v[i] + b.v[i])
 *
 * Example:
 *   simd_apply_add(float, a, b) → { a.v[0]+b.v[0], ..., a.v[n]+b.v[n] }
 */
#define simd_apply_add(T, a, b) simd_apply_binop(T,a,b,+)

/**
 * @brief Elementwise subtraction of two SIMD vectors.
 *
 * @tparam T Scalar type
 * @param a First SIMD operand
 * @param b Second SIMD operand
 * @return SIMD vector where each element is (a.v[i] - b.v[i])
 *
 * Example:
 *   simd_apply_sub(float, a, b) → { a.v[0]-b.v[0], ..., a.v[n]-b.v[n] }
 */
#define simd_apply_sub(T, a, b) simd_apply_binop(T,a,b,-)

/**
 * @brief Elementwise multiplication of two SIMD vectors.
 *
 * @tparam T Scalar type
 * @param a First SIMD operand
 * @param b Second SIMD operand
 * @return SIMD vector where each element is (a.v[i] * b.v[i])
 *
 * Example:
 *   simd_apply_mul(float, a, b) → { a.v[0]*b.v[0], ..., a.v[n]*b.v[n] }
 */
#define simd_apply_mul(T, a, b) simd_apply_binop(T,a,b,*)

/**
 * @brief Elementwise division of two SIMD vectors.
 *
 * @tparam T Scalar type
 * @param a First SIMD operand
 * @param b Second SIMD operand
 * @return SIMD vector where each element is (a.v[i] / b.v[i])
 *
 * Example:
 *   simd_apply_div(float, a, b) → { a.v[0]/b.v[0], ..., a.v[n]/b.v[n] }
 */
#define simd_apply_div(T, a, b) simd_apply_binop(T,a,b,/)

/**
 * @brief Compute dot product of two SIMD vectors.
 *
 * @tparam T Scalar type
 * @param a First SIMD operand
 * @param b Second SIMD operand
 * @return Scalar dot product
 *
 * Example:
 *   simd_apply_dot(float, a, b) → Σ (a.v[i] * b.v[i])
 */
#define simd_apply_dot(T, a, b) \
    simd_apply_sum(T, simd_apply_mul(T, a, b))