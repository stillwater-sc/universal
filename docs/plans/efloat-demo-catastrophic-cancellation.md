# `efloat` Demonstration: Catastrophic Cancellation Analysis

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Goal

Create a demonstration program that shows how `efloat` can compute an accurate result for a numerically unstable formula where `double` precision fails due to catastrophic cancellation.

## Demonstration

1.  **The Unstable Formula**: Use the function `f(x) = (1 - cos(x)) / x^2`.
    *   For values of `x` close to zero, `cos(x)` is very close to `1`. The subtraction `1 - cos(x)` loses most of its significant digits when performed with fixed-precision `double`.

2.  **The Stable Formula**: The expression can be rewritten as `g(x) = 2 * sin(x/2)^2 / x^2`. This form is numerically stable for small `x`.

3.  **Implementation**:
    *   Write a program that takes a value for `x` (e.g., $10^{-10}$).
    *   Compute `f(x)` and `g(x)` using standard `double`. Show that the results diverge significantly.
    *   Compute `f(x)` and `g(x)` using `efloat` with high precision (e.g., 256 bits).
    *   Print all four results.

## Expected Outcome

The program's output should clearly demonstrate that:
*   `f(x)` computed with `double` is highly inaccurate.
*   `g(x)` computed with `double` provides the "correct" reference answer at `double`'s precision level.
*   `f(x)` and `g(x)` computed with `efloat` produce nearly identical results, showing that `efloat` is powerful enough to get the right answer even from the unstable formula.

This provides a compelling and easy-to-understand example of the value of arbitrary-precision arithmetic.

## Acceptance Criteria

*   A C++ source file is created in `applications/efloat_demonstrations/catastrophic_cancellation.cpp`.
*   The program compiles and runs, printing a clear, formatted output comparing the `double` and `efloat` results for both the stable and unstable formulas.
