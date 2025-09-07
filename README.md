# not-a-simd-lib

**Not-A-SIMD-Lib** is a tiny C header-only helper library that generates SIMD-like code (or not…), depending on the compiler and optimization flags.

It uses macros to declare "SIMD vector" types of configurable width (`XLEN`) and provides macros to define and use elementwise operations, reductions, and dot products.

> ⚠️ Note: This does **not** guarantee actual SIMD instructions — it only makes your code more likely to be auto-vectorized by compilers with flags like `-O3`, `-march=native`, etc.

## Why?

Honestly, I don't really know... It seemed like a fun project...

Compilers are usually smart enough to vectorize loops automatically, but this library provides a clean way to express operations in a SIMD-like style, making it easier to write "vector-friendly" code.

## How it Works

The library is a collection of **macros** that generate structs and functions.

### Example

```c
#define XLEN 256 // default = 128 if not defined
#include "notasimdlib.h"

// Declare SIMD type with float elements
decl_simd_t(float)
```

Generates:

```c
typedef struct simd_vfloat256_t {
    float v[8]; // XLEN / (8 * sizeof(float)) = 8 elements
} simd_vfloat256_t;
```

## API Reference

### Macro Concatenation Utilities

```c
#define PPCAT_NX(A, B) A##B
#define PPCAT(A, B) PPCAT_NX(A, B)
```

* **`PPCAT_NX(A,B)`**: Concatenate tokens `A` and `B` without macro expansion.
* **`PPCAT(A,B)`**: Concatenate tokens with expansion.

Example:

```c
#define X foo
PPCAT(X,bar) // → foobar
```

---

### SIMD Vector Configuration

```c
#ifndef XLEN
#define XLEN 128
#endif

#define VLEN(T) (XLEN / (8 * sizeof(T)))
```

* **`XLEN`**: Register width in bits (default: 128).
* **`VLEN(T)`**: Number of elements of type `T` that fit in XLEN bits.

Example:

```c
VLEN(float) // → 4 when XLEN=128
```

---

### SIMD Type Declaration

```c
#define simd_t(T) ...
#define decl_simd_t(T) ...
```

* **`simd_t(T)`**: Expands to a type name like `simd_vfloat128_t`.
* **`decl_simd_t(T)`**: Declares the SIMD struct type.

Example:

```c
decl_simd_t(float)
// → typedef struct { float v[4]; } simd_vfloat128_t;
```

---

### Operation Naming

```c
#define simd_op_name(T, name) ...
```

* Builds names for operation functions:

```c
simd_op_name(float, add) // → add_simd_vfloat128_t
```

---

### Binary Operations

```c
#define simd_bin_op(name, T, a, b) ...
#define decl_simd_bin_op(name, T, op) ...
```

* **`simd_bin_op`**: Calls a SIMD binary function.
* **`decl_simd_bin_op`**: Declares a SIMD binary function with operator `op`.

Example:

```c
decl_simd_bin_op(add,float,+)
simd_t(float) z = simd_bin_op(add,float,x,y);
```

---

### Reductions

```c
#define simd_reduce_func(T, func, ...)
#define simd_reduce_expr(T, expr, ...)
#define simd_apply_sum(T, a)
```

* **`simd_reduce_func`**: Reduce with a custom function `(accum, i, ...)`.
* **`simd_reduce_expr`**: Reduce with an inline expression `(accum + vec.v[i])`.
* **`simd_apply_sum`**: Sum of elements in a vector.

---

### Elementwise Operations

```c
#define simd_apply_binop(T, a, b, op)
#define simd_apply_add(T, a, b)
#define simd_apply_sub(T, a, b)
#define simd_apply_mul(T, a, b)
#define simd_apply_div(T, a, b)
#define simd_apply_dot(T, a, b)
```

* **`simd_apply_add`**: `a.v[i] + b.v[i]`
* **`simd_apply_sub`**: `a.v[i] - b.v[i]`
* **`simd_apply_mul`**: `a.v[i] * b.v[i]`
* **`simd_apply_div`**: `a.v[i] / b.v[i]`
* **`simd_apply_dot`**: Dot product = sum of elementwise multiplies.

---

## Usage Example

```c
#define XLEN 256
#include "notasimdlib.h"

decl_simd_t(float)
decl_simd_t(int)

decl_simd_bin_op(add, float, +)
decl_simd_bin_op(add, int, +)

float dot_func(float accum, int idx, simd_t(float) a, simd_t(float) b) {
    return accum + (a.v[idx] * b.v[idx]);
}

int main() {
    simd_t(float) veca, vecb;
    simd_t(int) iveca, ivecb;

    // Built-in ops
    simd_t(float) addv = simd_apply_add(float, veca, vecb);
    simd_t(float) subv = simd_apply_sub(float, veca, vecb);

    // Custom ops
    simd_t(float) vecc = simd_bin_op(add,float,veca,vecb);

    // Dot product
    float dot1 = simd_apply_dot(float, veca, vecb);
    float dot2 = simd_reduce_func(float, dot_func, veca, vecb);
    float dot3 = simd_reduce_expr(float, accum + (veca.v[i]*vecb.v[i]), veca, vecb);

    return 0;
}
```

---

## Notes

* This library does not use intrinsics directly.
* Real SIMD depends on compiler auto-vectorization.
* More like “syntactic sugar for loops” than a true SIMD implementation.

---

👉 You can link directly to [**`notasimdlib.h`**](notasimdlib.h) for the full source with inline docs.
