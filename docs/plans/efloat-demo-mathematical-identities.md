# `efloat` Demonstration: Numerical Verification of Mathematical Identities

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Goal

Showcase `efloat` as a tool for formal verification by writing a program that numerically verifies a mathematical identity where cumulative error in `double` precision would lead to failure.

## Demonstration

1.  **The Identity**: A good candidate is one that involves a long summation or sequence. For example, verifying that the sum of a specific telescoping series converges to its known value, or that a high-order numerical integration matches the exact analytical result.

    A simple but effective example is **Sala's identity for $\pi$**:
    $\pi = \sum_{n=0}^{\infty} \frac{1}{16^n} \left( \frac{4}{8n+1} - \frac{2}{8n+4} - \frac{1}{8n+5} - \frac{1}{8n+6} ight)$
    This is the basis for the BBP formula, which allows for computing the n-th hexadecimal digit of $\pi$ without computing the preceding digits.

2.  **The Problem**: While this series converges quickly, summing many terms using `double` will accumulate rounding errors. The goal is to compute the sum to a precision that exceeds what `double` can offer.

3.  **Implementation**:
    *   Write a program that calculates $\pi$ using this formula.
    *   **Step 1 (Double Precision)**:
        *   Sum the first, say, 50 terms of the series using `double`.
        *   Compare the result to a known high-precision value of $\pi$ and print the error.
    *   **Step 2 (efloat Precision)**:
        *   Sum the same number of terms using `efloat` with a high precision (e.g., 400 bits).
        *   Compare the result to the known value of $\pi$ and print the error.

## Expected Outcome

The program's output will show:
*   The `double`-based calculation matches the true value of $\pi$ to about 15-16 decimal digits and then diverges.
*   The `efloat`-based calculation matches the true value of $\pi$ to a much higher number of digits (e.g., ~100 digits), demonstrating its ability to control and reduce computational error.

This highlights `efloat`'s role in computational mathematics and as a tool for verifying the correctness of algorithms.

## Acceptance Criteria

*   A C++ source file is created in `applications/efloat_demonstrations/mathematical_identities.cpp`.
*   The program computes $\pi$ using the specified formula with both `double` and `efloat`.
*   The output clearly shows the error of each computation relative to a known reference value.
