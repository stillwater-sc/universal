# `efloat` Feature: Lossless Conversions

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Problem Description

Converting an `efloat` to a native floating-point type (`double`, `float`) is currently implemented using `std::pow`, which relies on `double`-precision arithmetic. This defeats the purpose of `efloat` by truncating high-precision values.

```cpp
// Current implementation in efloat_impl.hpp
v = Real(sign()) * std::pow(Real(2.0), Real(scale())) * Real(significant());
```

This is not a true conversion; it's a lossy reconstruction.

## Proposed Solution

Implement a direct, bit-level conversion from `efloat` to native IEEE 754 formats.
1.  **Extract Components**: Get the sign, exponent, and significand limbs from the `efloat`.
2.  **Round Significand**: Round the `efloat`'s significand to the target precision of the native type (e.g., 53 bits for `double`, 24 for `float`). This rounding must respect the active rounding mode.
3.  **Handle Overflow/Underflow**: Check if the `efloat`'s exponent is within the representable range of the target native type.
    *   If it overflows, return `+/- infinity`.
    *   If it underflows, return `+/- zero` or a subnormal number, depending on the rounding mode.
4.  **Assemble Bits**: Construct the final `float` or `double` by bit-packing the sign, rounded exponent, and rounded significand into the correct IEEE 754 layout. Use `std::bit_cast` to produce the final value.

## Acceptance Criteria

*   The `operator double()` and `operator float()` conversions are rewritten to perform a direct, correctly-rounded conversion.
*   The conversion correctly handles overflow, underflow, and subnormal generation.
*   The test suite is updated to verify the correctness of these conversions against edge cases.
