# `efloat` Feature: Correct Rounding

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Problem Description

A key requirement for a reliable arbitrary-precision library like MPFR is that all operations are **correctly rounded**. The result of any operation must be the same as if it were computed to infinite precision and then rounded to the target precision according to a specified mode.

The current `efloat` implementation has no mechanism for specifying or applying rounding modes.

## Proposed Solution

1.  **Define Rounding Modes**: Create an `enum` for the standard IEEE 754 rounding modes:
    *   `RoundToNearest` (ties to even)
    *   `RoundToZero` (truncation)
    *   `RoundTowardPositive` (+infinity)
    *   `RoundTowardNegative` (-infinity)

2.  **Integrate Rounding into Arithmetic**: The arithmetic algorithms (especially multiplication and division) must be designed to produce a result with extra precision (guard bits, sticky bit) to enable correct rounding. The final step of each arithmetic operation will be to apply the rounding logic based on these extra bits and the active rounding mode.

3.  **Provide Rounding Control**:
    *   Allow a rounding mode to be specified per operation.
    *   Alternatively, support a thread-local or global default rounding mode that can be set by the user.

## Acceptance Criteria

*   The four standard rounding modes are implemented.
*   All arithmetic operations (`+`, `-`, `*`, `/`) respect the active rounding mode.
*   The math library functions (once implemented) are also correctly rounded.
*   A test suite is created to verify that rounding is correct for all modes and for difficult cases (e.g., halfway cases for round-to-nearest).
