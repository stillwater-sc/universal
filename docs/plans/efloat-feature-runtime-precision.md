# `efloat` Feature: Runtime Precision Management

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Problem Description

`efloat` is a class template parameterized by `nlimbs`, which suggests a compile-time precision limit. While this is useful, a true MPFR-like "Oracle" often requires precision to be managed at **runtime**. This allows a user to, for example, increase precision iteratively until a desired result is achieved, without recompiling.

## Proposed Solution

This can be implemented in two stages:

1.  **Global Default Precision**:
    *   Introduce a static, thread-local variable that holds a "default precision" in bits.
    *   Provide functions (`efloat::get_default_precision()`, `efloat::set_default_precision(bits)`) to manage it.
    *   When a new `efloat` is created (e.g., from a `double`), it is constructed with this default precision. The internal `std::vector` of limbs is sized accordingly.

2.  **Per-Instance Precision**:
    *   Add a `precision` member to each `efloat` instance.
    *   Provide a member function, `efloat::set_precision(bits)`, that can resize the limb vector and re-round the stored value to the new precision.
    *   The results of arithmetic operations should be rounded to a precision determined by the operands' precision (e.g., the maximum of the two).

## Acceptance Criteria

*   A user can set a global default precision for new `efloat` objects.
*   A user can change the precision of an individual `efloat` instance at runtime.
*   Arithmetic operations correctly produce a result with a well-defined precision.
*   Tests are written to verify that precision changes and rounding are handled correctly.
