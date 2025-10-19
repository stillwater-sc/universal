# Understanding Expansion Algebra

An interactive tutorial on multi-component floating-point arithmetic and the algorithms behind Universal's cascade types (`dd_cascade`, `td_cascade`, `qd_cascade`).

## Overview

This tutorial teaches the fundamental concepts of expansion algebra—the mathematical foundation for high-precision arithmetic using multiple IEEE-754 doubles. You'll learn why standard floating-point loses precision, how error-free transformations capture lost bits, and how the sophisticated compression algorithms in Universal preserve precision.

## Learning Objectives

After completing this tutorial, you will understand:

1. **The Precision Problem**: Why IEEE-754 floating-point loses information and how errors accumulate
2. **Error-Free Transformations**: The mathematical foundation of `two_sum` and `fast_two_sum`
3. **Multi-Component Expansions**: How multiple doubles combine to achieve arbitrary precision
4. **Expansion Arithmetic**: Why operations cause growth and the need for compression
5. **The Naive Compression Trap**: Why the intuitive approach fails spectacularly
6. **Proper Compression**: The two-phase algorithm from Hida-Li-Bailey's QD library
7. **Scaling to Higher Precision**: How the pattern extends from dd to td to qd
8. **Real-World Applications**: When and why extended precision matters

## Building

**Note:** This is an interactive tutorial and is NOT part of CI builds. It requires a separate build flag.

```bash
cd build
cmake .. -DUNIVERSAL_BUILD_EDUCATION_INTERACTIVE=ON
make edu_expansion_algebra_expansion_algebra_tutorial
```

## Running

```bash
./education/interactive/expansion_algebra/edu_expansion_algebra_expansion_algebra_tutorial
```

**Why separate from regular education?** Interactive tutorials require user input and would hang CI builds. They live in `education/interactive/` to keep CI builds clean while still providing valuable learning resources.

The program presents an interactive menu with 8 progressive lessons. You can:
- Work through lessons sequentially (recommended for beginners)
- Jump to specific topics of interest
- Run all lessons in sequence (option 9)

## Tutorial Structure

### Lesson 1: The Rounding Error Problem
Foundation concepts showing why we need expansion algebra:
- The classic `0.1 + 0.2 ≠ 0.3` problem
- Catastrophic cancellation: `(1e16 + 1.0) - 1e16 = 0` instead of 1.0
- Error accumulation in summation

**Key Insight**: Standard floating-point loses information that we might need!

### Lesson 2: Error-Free Transformations
The breakthrough that makes expansion algebra possible:
- Dekker's `two_sum`: Capture rounding error from `a + b`
- Knuth's `fast_two_sum`: Optimized version for ordered inputs
- Bit-level visualization of where errors come from

**Key Insight**: We can perform addition EXACTLY using two doubles!

### Lesson 3: Multi-Component Expansions
Building higher precision from multiple components:
- Double-double (106 bits), triple-double (159 bits), quad-double (212 bits)
- The non-overlapping property
- Precision comparison with standard types

**Key Insight**: N components give N × 53 bits using standard hardware!

### Lesson 4: Expansion Addition
How arithmetic operations work on expansions:
- Component-wise addition with `two_sum`
- The growth problem: 2+2→4, 3+3→6, 4+4→8
- Why compression is necessary

**Key Insight**: Exact addition causes component growth—we need compression!

### Lesson 5: The Naive Compression Trap
Understanding the bug we fixed in `qd_cascade`:
- Why `compressed[3] = result[3] + result[4] + ...` is broken
- The identity test failure: `(a+b)-a ≠ b`
- Magnitude of precision loss

**Key Insight**: Naive compression destroys the precision we worked to preserve!

### Lesson 6: Proper Compression Algorithm
The solution from the QD library:
- Phase 1: Bottom-up accumulation with `fast_two_sum`
- Phase 2: Conditional extraction of non-overlapping components
- Why this preserves all significant bits

**Key Insight**: Error-free transformations throughout = preserved precision!

### Lesson 7: Scaling to Higher Precision
Recognizing the pattern across precision levels:
- `compress_4to2()`, `compress_6to3()`, `compress_8to4()`
- Performance vs precision tradeoffs
- When to use dd vs td vs qd

**Key Insight**: The same algorithm pattern scales to arbitrary precision!

### Lesson 8: Real-World Applications
Why expansion algebra matters in practice:
- Reproducible linear algebra (quires)
- Ill-conditioned systems (Hilbert matrices)
- Accurate polynomial evaluation near roots
- Iterative refinement for linear solvers
- Deep learning gradient accumulation

**Key Insight**: The performance cost is worth it when correctness matters!

## Interactive Features

Each lesson includes:
- **Clear Explanations**: Progressive concept development
- **Concrete Examples**: Actual numeric demonstrations
- **Bit-Level Views**: IEEE-754 representation visualization
- **Before/After Comparisons**: Naive vs proper algorithms
- **Pause Points**: "Press Enter to continue" for pacing

## Related Code

After completing the tutorial, explore these files for deeper understanding:

- **Compression Algorithms**: `/include/sw/universal/internal/floatcascade/floatcascade.hpp`
  - Lines 469-700+ contain the compression functions with extensive commentary

- **Cascade Implementations**:
  - `/include/sw/universal/number/dd_cascade/dd_cascade_impl.hpp`
  - `/include/sw/universal/number/td_cascade/td_cascade_impl.hpp`
  - `/include/sw/universal/number/qd_cascade/qd_cascade_impl.hpp`

- **Test Suites**:
  - `/static/dd_cascade/arithmetic/addition.cpp` (includes Corner Case 11: identity test)
  - `/static/dd_cascade/arithmetic/subtraction.cpp` (includes Corner Case 13: identity test)
  - Similar tests for td_cascade and qd_cascade

## References

The algorithms and concepts are based on:

1. **Dekker, T.J.** (1971): "A Floating-Point Technique for Extending the Available Precision"
   - Original `two_sum` algorithm

2. **Knuth, D.E.** (1974): "The Art of Computer Programming, Vol. 2: Seminumerical Algorithms"
   - `fast_two_sum` optimization

3. **Priest, D.M.** (1991): "Algorithms for Arbitrary Precision Floating Point Arithmetic"
   - Expansion algebra theory

4. **Hida, Y., Li, X.S., Bailey, D.H.** (2001): "Algorithms for Quad-Double Precision Floating Point Arithmetic"
   - QD library implementation (compression algorithms)
   - Available at: https://www.davidhbailey.com/dhbpapers/qd.pdf

## Next Steps

After completing this tutorial:

1. **Experiment**: Use dd, td, or qd in your own numerical code
2. **Read the Code**: Study the compression functions with your new understanding
3. **Run Tests**: Execute the cascade regression suites to see edge cases
4. **Apply**: Try solving an ill-conditioned problem with extended precision

## Educational Philosophy

This tutorial follows the Universal library's educational approach:
- **Progressive Learning**: Each lesson builds on previous concepts
- **Hands-On Examples**: See the algorithms in action
- **Real Code**: Uses actual Universal implementations
- **Practical Focus**: Connects theory to real-world problems

The goal is not just to use the cascade types, but to *understand* why they work and when to apply them.

## Feedback

Found this tutorial helpful? Have suggestions for improvement? Please open an issue or PR at:
https://github.com/stillwater-sc/universal
