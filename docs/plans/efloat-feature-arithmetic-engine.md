# `efloat` Feature: Core Arithmetic Engine

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Problem Description

The `efloat` number system is intended to be an arbitrary-precision floating-point type, but its core arithmetic operators (`+`, `-`, `*`, `/`) are currently unimplemented stubs. They do not perform any computation, making the type unusable for any mathematical operations.

```cpp
// Current implementation in efloat_impl.hpp
constexpr efloat& operator+=(const efloat& /* rhs */) noexcept {
    return *this;
}
```

This is the most critical gap in the `efloat` implementation.

## Proposed Solution

Implement the fundamental algorithms for arbitrary-precision floating-point arithmetic. This involves:
1.  **Significand Alignment**: For addition/subtraction, align the significands of the two operands based on their exponents.
2.  **Limb-Based Arithmetic**: Perform the arithmetic on the `std::vector<uint32_t>` limbs.
    *   Addition/subtraction will require handling carry propagation across limbs.
    *   Multiplication will use a long-multiplication algorithm (e.g., Karatsuba for larger precision) to produce a full-precision product.
    *   Division will use a long-division algorithm (e.g., Newton-Raphson iteration for high precision).
3.  **Normalization**: After the operation, normalize the resulting significand and adjust the exponent accordingly.
4.  **Rounding**: Apply the specified rounding mode to truncate the result to the target precision. (This depends on the "Correct Rounding" feature).

## Acceptance Criteria

*   The `+`, `-`, `*`, `/` operators and their compound-assignment versions are fully implemented.
*   The implementation passes a rigorous test suite covering a wide range of values, including edge cases like subnormals and large exponent differences.
*   Basic identities are satisfied (e.g., `x + y - y == x`).
