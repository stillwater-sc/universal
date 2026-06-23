# `efloat` Demonstration: Solving Ill-Conditioned Linear Systems

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Goal

Demonstrate how `efloat` can be used to obtain an accurate solution to a notoriously ill-conditioned linear system where standard `double` precision yields a result with high error.

## Demonstration

1.  **The Ill-Conditioned System**: Use the Hilbert matrix, where $A_{ij} = 1 / (i + j - 1)$. This matrix is a classic example of an ill-conditioned matrix, meaning small input errors lead to large output errors.
2.  **The Problem**: For a known vector `x` (e.g., `x = [1, 1, ..., 1]`), compute `b = Ax`. Then, attempt to solve for `x_computed` given `A` and `b`. The goal is to see how close `x_computed` is to the original `x`.

3.  **Implementation**:
    *   Choose a matrix size, for example, `n = 12`.
    *   **Step 1 (Double Precision)**:
        *   Construct the `n x n` Hilbert matrix `A` and the solution vector `x` using `double`.
        *   Compute `b = Ax`.
        *   Solve `Ax_computed = b` for `x_computed` using a standard linear algebra library (e.g., a simple Gaussian elimination implementation).
        *   Compute and print the relative error `||x - x_computed|| / ||x||`.
    *   **Step 2 (efloat Oracle)**:
        *   Repeat the entire process using `efloat` with high precision (e.g., 512 bits) instead of `double`.
        *   Compute and print the relative error for the `efloat`-based solution.

## Expected Outcome

The output will show:
*   The `double`-based solution has a very large relative error, indicating it is essentially garbage.
*   The `efloat`-based solution has a very small relative error, demonstrating its ability to handle the numerical instability of the problem and produce a correct "Oracle" result.

This highlights the use of `efloat` in high-precision scientific computing and as a tool for verifying the stability of numerical algorithms.

## Acceptance Criteria

*   A C++ source file is created in `applications/efloat_demonstrations/ill_conditioned_systems.cpp`.
*   The program implements the described steps for both `double` and `efloat`.
*   The program prints a clear summary comparing the relative error of the two solutions.
