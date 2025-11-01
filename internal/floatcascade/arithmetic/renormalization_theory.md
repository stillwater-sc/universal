# Renormalization Theory for Multi-Component Floating-Point Arithmetic

**Date:** 2025-11-01
**Author:** Research for Universal floatcascade<N> improvement
**Purpose:** Document theoretical foundations and algorithm requirements for renormalization

---

## Executive Summary

This document presents findings from literature research on renormalization algorithms for multi-component floating-point arithmetic, specifically focusing on:
- Priest's normalization requirements (non-overlapping property)
- Hida-Li-Bailey QD library two-phase renormalization algorithm
- Requirements for generalizing to arbitrary N components

**Key Finding:** The QD library uses a sophisticated two-phase algorithm with conditional refinement that maintains strict non-overlapping properties, unlike our current single-pass approach which violates the property by 3.24x.

---

## 1. Background: Multi-Component Floating-Point Arithmetic

### 1.1 Floating-Point Expansion Definition

A **floating-point expansion** represents a high-precision number as an unevaluated sum of standard floating-point numbers:

```
x = x[0] + x[1] + x[2] + ... + x[N-1]
```

Where:
- Each `x[i]` is a standard IEEE-754 double (53-bit significand)
- The components are ordered: `|x[0]| >= |x[1]| >= ... >= |x[N-1]|`
- Total precision: approximately N × 53 bits

### 1.2 The Non-Overlapping Property (Priest's Invariant)

**Definition:** A floating-point expansion satisfies the **non-overlapping property** if:

```
|x[i+1]| ≤ ulp(x[i]) / 2
```

For all i ∈ [0, N-2], where `ulp(x[i])` is the unit in last place of component `x[i]`.

**Significance:**
- Ensures components represent distinct precision levels
- Prevents redundant representation of bits across components
- Required for predictable precision and error bounds
- Enables exact arithmetic operations

**Violation Consequences:**
- Loss of precision in intermediate calculations
- Error accumulation in iterative algorithms
- Unpredictable total precision (we observed 3.24x violations → 60-70% precision loss in pow())

### 1.3 Precision Calculations

For IEEE-754 double precision (binary64):
- **Double-Double (N=2)**: ~106 bits (31-32 decimal digits)
- **Triple-Double (N=3)**: ~159 bits (47-48 decimal digits)
- **Quad-Double (N=4)**: ~212 bits (63-64 decimal digits)
- **Octo-Double (N=8)**: ~424 bits (127-128 decimal digits)

---

## 2. Literature Review

### 2.1 Priest (1991)

**Citation:** Douglas M. Priest, "Algorithms for arbitrary precision floating point arithmetic," 10th IEEE Symposium on Computer Arithmetic (ARITH 1991), pp. 132-143, 1991.

**Key Contributions:**
- Defined normalization requirements for floating-point expansions
- Introduced error-free transformations: two_sum, two_prod
- Established theoretical foundation for exact arithmetic using floating-point
- Proved correctness under IEEE-754 assumptions

**Non-Overlapping Requirement:**
Priest's normalization ensures that components are separated by sufficient magnitude that their significant bits do not overlap. This is critical for:
1. Maintaining accuracy across operations
2. Enabling exact representation of intermediate results
3. Providing predictable error bounds

### 2.2 Hida-Li-Bailey (2000-2001)

**Citations:**
- Y. Hida, X.S. Li, D.H. Bailey, "Library for Double-Double and Quad-Double Arithmetic," LBNL Technical Report LBNL-46996, October 2000
- Y. Hida, X.S. Li, D.H. Bailey, "Algorithms for Quad-Double Precision Floating Point Arithmetic," 15th IEEE Symposium on Computer Arithmetic, pp. 155-162, 2001

**Key Contributions:**
- Practical implementation of Priest's theoretical work
- Developed efficient two-phase renormalization algorithm
- Created complete library with C++ and Fortran interfaces
- Demonstrated applications in high-precision scientific computing

**QD Library Implementation:**
- Open source: Available from David Bailey's website and GitHub
- Language support: C++, Fortran-90, with bindings for other languages
- Performance: Optimized error-free transformations with unrolled loops
- Validation: Extensively tested in computational physics and mathematics

---

## 3. QD Library Renormalization Algorithms

### 3.1 Error-Free Transformation: quick_two_sum

**Precondition:** `|a| >= |b|` (critical assumption)

```cpp
inline double quick_two_sum(double a, double b, double &err) {
    double s = a + b;
    err = b - (s - a);
    return s;
}
```

**Properties:**
- Exact: `s + err = a + b` (mathematically exact)
- No rounding error lost
- Requires ordered inputs: |a| >= |b|
- Faster than two_sum (no fabs needed)

### 3.2 Four-Component Renormalization (Standard)

**Source:** QD library `qd_inline.h`

**Algorithm:**

```cpp
inline void renorm(double &c0, double &c1, double &c2, double &c3) {
    double s0, s1, s2 = 0.0, s3 = 0.0;

    // Handle infinity
    if (QD_ISINF(c0)) return;

    // ===== PHASE 1: Compression =====
    // Accumulate from bottom to top using quick_two_sum
    s0 = quick_two_sum(c2, c3, c3);    // c2 + c3 → s0 + c3
    s0 = quick_two_sum(c1, s0, c2);    // c1 + s0 → s0 + c2
    c0 = quick_two_sum(c0, s0, c1);    // c0 + s0 → c0 + c1

    // At this point: c0 + c1 + c2 + c3 = original sum

    // ===== PHASE 2: Conditional Refinement =====
    // Propagate carries with zero detection
    s0 = c0;
    s1 = c1;

    if (s1 != 0.0) {
        // Normal path: propagate c2 through s1
        s1 = quick_two_sum(s1, c2, s2);
        if (s2 != 0.0)
            s2 = quick_two_sum(s2, c3, s3);
        else
            s1 = quick_two_sum(s1, c3, s2);
    } else {
        // s1 is zero: try to propagate c2 into s0
        s0 = quick_two_sum(s0, c2, s1);
        if (s1 != 0.0)
            s1 = quick_two_sum(s1, c3, s2);
        else
            s0 = quick_two_sum(s0, c3, s1);
    }

    // Write back results
    c0 = s0;
    c1 = s1;
    c2 = s2;
    c3 = s3;
}
```

**Algorithm Analysis:**

1. **Phase 1 (Compression):**
   - Performs 3 quick_two_sum operations
   - Accumulates from least significant to most significant
   - Creates initial normalized structure
   - Time complexity: O(N) where N=4

2. **Phase 2 (Conditional Refinement):**
   - Uses zero detection to skip unnecessary work
   - Propagates carries that may have been created in Phase 1
   - Ensures final components satisfy non-overlapping property
   - Handles edge cases where intermediate components are zero
   - Time complexity: O(N) worst case, but often faster due to early exits

3. **Key Differences from floatcascade:**
   - **Two phases** vs single pass
   - **Conditional branching** vs unconditional accumulation
   - **Zero detection** to handle edge cases
   - **Multiple refinement passes** to ensure property holds

### 3.3 Five-Component Quick Renormalization

**Source:** QD library `qd_inline.h`

This variant handles 5 input components (typical after multiplication) and normalizes to 4 outputs:

```cpp
inline void quick_renorm(double &c0, double &c1,
                         double &c2, double &c3, double &c4) {
    double t0, t1, t2, t3;
    double s;

    // Phase 1: Bottom-up compression
    s  = quick_two_sum(c3, c4, t3);
    s  = quick_two_sum(c2, s , t2);
    s  = quick_two_sum(c1, s , t1);
    c0 = quick_two_sum(c0, s , t0);

    // Phase 2: Refinement of error terms
    s  = quick_two_sum(t2, t3, t2);
    s  = quick_two_sum(t1, s , t1);
    c1 = quick_two_sum(t0, s , t0);

    // Phase 3: Final compression
    s  = quick_two_sum(t1, t2, t1);
    c2 = quick_two_sum(t0, s , t0);

    c3 = t0 + t1;  // Final component (may not be fully normalized)
}
```

**Usage:** Called after operations that produce N+1 components (e.g., multiplication)

---

## 4. Current floatcascade Implementation Analysis

### 4.1 Current Algorithm

**Source:** `include/sw/universal/internal/floatcascade/floatcascade.hpp:532-546`

```cpp
template<size_t N>
floatcascade<N> renormalize(const floatcascade<N>& e) {
    floatcascade<N> result;
    double s = e[N-1];

    for (int i = N - 2; i >= 0; --i) {
        double hi, lo;
        two_sum(s, e[static_cast<size_t>(i)], hi, lo);
        result[static_cast<size_t>(i+1)] = lo;
        s = hi;
    }
    result[0] = s;
    return result;
}
```

**Algorithm Characteristics:**
- **Single pass**: One loop from N-2 down to 0
- **Uses two_sum**: More expensive than quick_two_sum (uses fabs)
- **No precondition**: two_sum works regardless of ordering
- **No refinement**: No second phase to ensure property
- **Generic**: Works for any N

### 4.2 Problems Identified

1. **Insufficient Compression:**
   - Single pass may not fully normalize components
   - Errors can accumulate without refinement
   - Observed: 3.24x violation of non-overlapping property

2. **Algorithm Complexity:**
   - Uses two_sum (O(1) but with higher constant factor than quick_two_sum)
   - No early exit optimization
   - Creates temporary floatcascade instead of in-place

3. **Missing Second Phase:**
   - QD algorithm's conditional refinement is critical
   - Without it, intermediate errors propagate uncorrected
   - Cumulative effect: 3.24^35 ≈ 10^18 loss after 35 multiplications

### 4.3 Why Single Pass Fails

**Mathematical Reason:**

After the first compression pass, we have:
```
x = x[0] + x[1] + x[2] + x[3]
```

But the property `|x[i+1]| ≤ ulp(x[i])/2` may not hold because:
1. The error terms from two_sum may exceed ulp/2
2. Rounding during the accumulation can create overlaps
3. No correction pass to redistribute bits

**Example:**
```
Input:  [1.0e100, 1.0e99, 1.0e98, 5.0e97]  // Potential overlap
Pass 1: [1.1e100, 4.9e97, ε₁, ε₂]         // Better, but not perfect
        |c[1]| may exceed ulp(c[0])/2 by 3.24x
```

QD's second phase would refine this:
```
Pass 2: [1.1e100, ~0, smaller, even_smaller]  // Non-overlapping achieved
```

---

## 5. Design Requirements for floatcascade<N>

### 5.1 Functional Requirements

1. **Correctness:**
   - Must maintain exact sum: Σ input[i] = Σ output[i]
   - Must satisfy non-overlapping property: |x[i+1]| ≤ ulp(x[i])/2
   - Must handle special values: infinity, NaN, zero
   - Must work for arbitrary N ≥ 2

2. **Performance:**
   - Should be O(N) time complexity
   - Should minimize number of expensive operations
   - Should use quick_two_sum where preconditions met
   - Should avoid unnecessary allocations

3. **Generality:**
   - Must work for N ∈ {2, 3, 4, 8, ...}
   - Should be template-based for compile-time optimization
   - Should allow both in-place and out-of-place variants

### 5.2 Algorithm Design Constraints

1. **Two-Phase Approach:**
   - Phase 1: Compression using bottom-up accumulation
   - Phase 2: Refinement with conditional carry propagation

2. **Error-Free Transformations:**
   - Use quick_two_sum when ordering guaranteed
   - Fall back to two_sum when ordering uncertain
   - Maintain numerical stability

3. **Edge Case Handling:**
   - Zero components (common in sparse results)
   - Infinity propagation
   - NaN propagation
   - Denormal numbers

### 5.3 Verification Requirements

1. **Unit Tests:**
   - Non-overlapping property verification
   - Exact sum preservation
   - Special value handling
   - Comparison with QD library results

2. **Integration Tests:**
   - Multiplication precision (should achieve N×53 bits)
   - pow() precision (should achieve 200+ bits for N=4)
   - Iterative algorithm stability

3. **Stress Tests:**
   - Random inputs across full dynamic range
   - Extreme exponent differences
   - Octo-double (N=8) as proof of generalization

---

## 6. Proposed Generalized Algorithm

### 6.1 High-Level Design

```cpp
template<size_t N>
void renormalize(floatcascade<N>& fc) {
    // Phase 1: Compression (bottom-up accumulation)
    //   - Combine components using quick_two_sum
    //   - Accumulate from least to most significant
    //   - Generate N error terms

    // Phase 2: Conditional Refinement
    //   - Detect zero components
    //   - Propagate carries through non-zero terms
    //   - Ensure non-overlapping property
    //   - Compress final representation
}
```

### 6.2 Key Design Decisions

1. **In-Place Modification:**
   - Modify input floatcascade directly
   - Avoid temporary allocations
   - Use stack-allocated arrays for error terms

2. **Template Specialization:**
   - Provide optimized versions for N=2, N=3, N=4
   - Generic version for arbitrary N
   - Compiler can inline and optimize

3. **Loop Structure:**
   - Unroll Phase 1 for small N (2,3,4)
   - Use loops for large N (8+)
   - Balance code size vs performance

### 6.3 Pseudocode for Generic N

```cpp
template<size_t N>
void renormalize(floatcascade<N>& fc) {
    // Handle infinity early
    if (std::isinf(fc[0])) return;

    // Temporary storage for error terms
    std::array<double, N> errors = {0.0};

    // ===== PHASE 1: Compression =====
    double sum = fc[N-1];
    for (int i = N-2; i >= 0; --i) {
        sum = quick_two_sum(fc[i], sum, errors[i+1]);
    }
    fc[0] = sum;
    // fc[0] + errors[1..N-1] represents the value

    // ===== PHASE 2: Conditional Refinement =====
    for (size_t i = 1; i < N; ++i) {
        if (fc[i-1] != 0.0) {
            // Propagate error through non-zero component
            fc[i] = quick_two_sum(errors[i], fc[i+1], errors[i+1]);
        } else {
            // Skip zero component, accumulate into previous
            // (Complex logic similar to QD conditional branches)
        }
    }

    // Handle final component
    fc[N-1] = errors[N-1];
}
```

**Note:** The actual implementation will need careful handling of the conditional logic in Phase 2, similar to QD's branching structure.

---

## 7. Validation Strategy

### 7.1 Property Verification

For each test case, verify:

```cpp
bool verify_renormalized(const floatcascade<N>& fc) {
    // 1. Check ordering
    for (size_t i = 0; i < N-1; ++i) {
        if (std::fabs(fc[i]) < std::fabs(fc[i+1])) return false;
    }

    // 2. Check non-overlapping property
    for (size_t i = 0; i < N-1; ++i) {
        if (fc[i] == 0.0) continue;

        int exponent = std::ilogb(fc[i]);
        double ulp = std::ldexp(1.0, exponent - 52);
        double threshold = ulp / 2.0;

        if (std::fabs(fc[i+1]) > threshold) {
            // Calculate violation factor
            double violation = std::fabs(fc[i+1]) / threshold;
            if (violation > 1.01) return false;  // Allow 1% tolerance
        }
    }

    return true;
}
```

### 7.2 Test Cases

1. **Basic Arithmetic:**
   - Simple additions with known results
   - Multiplications across range
   - Division by powers of 2

2. **Edge Cases:**
   - All components zero
   - Single non-zero component
   - Alternating signs
   - Very small + very large

3. **Stress Testing:**
   - 10,000 random multiplications
   - pow(a, b) for random a, b
   - Verify final precision matches theory

4. **Cross-Library Validation:**
   - Compare results with QD library
   - Verify bit-for-bit accuracy where possible
   - Document any acceptable differences

---

## 8. Implementation Plan

### 8.1 Phase 1: Infrastructure (Complete)

✅ Literature research
✅ Algorithm documentation
✅ Requirements definition

### 8.2 Phase 2: Prototype Implementation

- [ ] Implement generalized renormalize() for floatcascade<N>
- [ ] Create template specializations for N=2,3,4
- [ ] Implement octo-double (floatcascade<8>) as test case
- [ ] Add verification functions

### 8.3 Phase 3: Testing

- [ ] Unit tests for non-overlapping property
- [ ] Integration tests with multiplication
- [ ] pow() precision validation
- [ ] Comparison with QD library

### 8.4 Phase 4: Optimization

- [ ] Profile renormalize() performance
- [ ] Optimize hot paths
- [ ] Consider SIMD opportunities
- [ ] Benchmark against QD library

### 8.5 Phase 5: Documentation

- [ ] Update this theory document
- [ ] Add inline code comments
- [ ] Update CHANGELOG
- [ ] Create examples

---

## 9. Expected Outcomes

### 9.1 Quantitative Goals

| Metric | Current | Target | Improvement |
|--------|---------|--------|-------------|
| Non-overlapping violation | 3.24x | < 1.01x | 3.2x reduction |
| qd_cascade pow() precision | 77-92 bits | 200+ bits | 2.2-2.6x increase |
| Multiplication precision | 212 bits | 212 bits | Maintained |
| Renormalize() calls per pow() | ~35 | ~35 | Same |

### 9.2 Qualitative Goals

- Predictable precision across all operations
- Stable behavior in iterative algorithms
- Generalization to arbitrary N proven
- Code maintainability improved
- Alignment with published literature

---

## 10. References

### 10.1 Primary Sources

1. **Priest (1991):** Douglas M. Priest, "Algorithms for arbitrary precision floating point arithmetic," 10th IEEE Symposium on Computer Arithmetic, pp. 132-143, 1991.

2. **Hida-Li-Bailey (2000):** Y. Hida, X.S. Li, D.H. Bailey, "Library for Double-Double and Quad-Double Arithmetic," LBNL Technical Report LBNL-46996, October 2000.

3. **Hida-Li-Bailey (2001):** Y. Hida, X.S. Li, D.H. Bailey, "Algorithms for Quad-Double Precision Floating Point Arithmetic," 15th IEEE Symposium on Computer Arithmetic, pp. 155-162, 2001.

### 10.2 Implementation References

4. **QD Library:** https://www.davidhbailey.com/dhbsoftware/
   - Source: https://github.com/aoki-t/QD
   - File: `include/qd/qd_inline.h` (renorm implementation)

5. **Universal Library:**
   - Current: `include/sw/universal/internal/floatcascade/floatcascade.hpp`
   - Error-free ops: `include/sw/universal/numerics/error_free_ops.hpp`

### 10.3 Related Work

6. **Shewchuk (1997):** Jonathan Richard Shewchuk, "Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates," Discrete & Computational Geometry 18(3):305-363, 1997.

7. **Muller et al. (2018):** Jean-Michel Muller et al., "Handbook of Floating-Point Arithmetic," 2nd Edition, Birkhäuser, 2018.

---

## Document Status

**Status:** Research Complete - Ready for Implementation
**Next Step:** Implement generalized renormalize() algorithm
**Owner:** Universal library development team
**Last Updated:** 2025-11-01
