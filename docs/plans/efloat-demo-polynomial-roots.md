# `efloat` Demonstration: High-Degree Polynomial Root Finding

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Goal

Demonstrate `efloat`'s ability to solve a numerically sensitive problem by accurately finding the roots of a polynomial where `double` precision fails due to the roots' high sensitivity to coefficient perturbation.

## Demonstration

1.  **The Polynomial**: **Wilkinson's polynomial** is the canonical example. The 20th-degree version is defined as:
    $W(x) = \prod_{i=1}^{20} (x - i) = (x-1)(x-2)...(x-20)$
    The roots are obviously the integers 1, 2, ..., 20. However, when the polynomial is expanded into its power series form ($W(x) = x^{20} - 210x^{19} + ...$), the roots become extremely sensitive to tiny changes in the coefficients.

2.  **The Problem**: If the coefficients are represented as `double`, the small representation errors are enough to drastically alter the computed roots. Standard root-finding algorithms using `double` will fail to find the integer roots accurately.

3.  **Implementation**:
    *   **Step 1**: Write a function to expand the polynomial to get its coefficients. This step itself must be done with high precision.
    *   **Step 2**: Implement a root-finding algorithm, such as Newton's method or the Durand-Kerner method.
    *   **Step 3 (Double Precision Failure)**:
        *   Store the expanded coefficients as `double`.
        *   Run the root-finding algorithm using `double` arithmetic.
        *   Print the computed roots. They will be wildly inaccurate, and some may even be complex.
    *   **Step 4 (efloat Success)**:
        *   Store the coefficients as high-precision `efloat` (e.g., 512 bits).
        *   Run the same root-finding algorithm using `efloat` arithmetic.
        *   Print the computed roots.

## Expected Outcome

The output will starkly contrast the two results:
*   The `double`-based computation will produce roots that are far from the integers 1-20, demonstrating numerical failure.
*   The `efloat`-based computation will produce roots that are extremely close to the integers 1-20, demonstrating its ability to handle the numerical sensitivity of the problem.

This is a classic and powerful example of numerical instability and the need for high-precision arithmetic.

## Acceptance Criteria

*   A C++ source file is created in `applications/efloat_demonstrations/polynomial_roots.cpp`.
*   The program computes and prints the roots of Wilkinson's polynomial using both `double` and `efloat`.
*   The `README.md` for the demo explains the numerical sensitivity of the problem and interprets the results.
