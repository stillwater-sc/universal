# `efloat` Feature: Mathematical Library

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Problem Description

An arbitrary-precision floating-point type is of limited use without a corresponding library of common mathematical functions. The `efloat` header currently has the include for a math library commented out, indicating it is unimplemented.

```cpp
// From efloat.hpp
//#include <universal/number/efloat/mathlib.hpp>
```

## Proposed Solution

Implement a comprehensive library of correctly-rounded mathematical functions for `efloat`. This is a significant undertaking that can be broken down into tiers based on complexity and priority.

*   **Tier 1 (Fundamental Functions)**:
    *   `sqrt`: Square root (e.g., using Newton's method).
    *   `log`: Natural logarithm.
    *   `exp`: Natural exponentiation.

*   **Tier 2 (Trigonometric Functions)**:
    *   `sin`, `cos`, `tan`.
    *   `asin`, `acos`, `atan`, `atan2`.
    *   Argument reduction for periodic functions must be handled with care at high precision.

*   **Tier 3 (Hyperbolic Functions)**:
    *   `sinh`, `cosh`, `tanh`, etc.

*   **Tier 4 (Special Functions)**:
    *   `gamma`, `erf`, etc.

Each function must be implemented using algorithms suitable for arbitrary precision (e.g., Taylor series, Newton's method, AGM, etc.) and must adhere to the `efloat`'s active rounding mode.

## Acceptance Criteria

*   A new header, `universal/number/efloat/mathlib.hpp`, is created.
*   At a minimum, the Tier 1 functions (`sqrt`, `log`, `exp`) are implemented and correctly rounded.
*   Each implemented function has a corresponding test in the regression suite that validates its correctness against known high-precision values.
