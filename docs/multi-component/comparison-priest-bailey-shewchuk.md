# Comparison: Priest vs. Bailey/Hida vs. Shewchuk

## Overview

This document compares three foundational approaches to multi-component arbitrary precision floating-point arithmetic that form the basis for the Universal library's dd, qd, td, and priest implementations.

## Historical Context

The field of multi-component arithmetic evolved through three key contributions:
1. **Douglas Priest (1991)** - Theoretical foundations
2. **David Bailey / Yozo Hida (2000-2008)** - Production implementations
3. **Jonathan Shewchuk (1996-1997)** - Adaptive precision for geometry

---

## 1. Douglas Priest (1991) - Foundational Theory

**Dissertation**: "On Properties of Floating Point Arithmetics: Numerical Stability and the Cost of Accurate Computations"

### Key Contributions

**Error-Free Transformations (EFTs)**: Pioneered the fundamental primitives that enable exact floating-point arithmetic:

- **`two_sum(a, b, s, e)`**: Computes `a + b` exactly, returning:
  - `s` = rounded sum (high-order result)
  - `e` = rounding error (low-order correction)
  - Property: `a + b = s + e` exactly

- **`two_prod(a, b, p, e)`**: Computes `a × b` exactly, returning:
  - `p` = rounded product (high-order result)
  - `e` = rounding error (low-order correction)
  - Property: `a × b = p + e` exactly

- **`quick_two_sum(a, b, s, e)`**: Faster variant when `|a| ≥ |b|` is known

These operations are the **building blocks** for all multi-component arithmetic systems.

### Multi-Component Representation

A real value is represented as an **expansion**: an unevaluated sum of non-overlapping floating-point components:

```
x = x₀ + x₁ + x₂ + ... + xₙ
```

where:
- Each `xᵢ` is a standard IEEE-754 floating-point number
- Components are **non-overlapping**: `|xᵢ| < ulp(xᵢ₋₁)`
- Components are ordered by magnitude: `|x₀| ≥ |x₁| ≥ |x₂| ≥ ...`

### Theoretical Framework

- Proved rigorous error bounds for multi-component operations
- Established properties of floating-point arithmetic beyond typical roundoff analysis
- Demonstrated that accurate computation is more cost-effective than commonly believed
- Required **pre-specification** of precision levels before computation

### Philosophy

Academic foundation proving what's mathematically possible with careful floating-point manipulation. Priest showed that IEEE-754 arithmetic has exploitable structure beyond its basic guarantees.

---

## 2. David Bailey / Yozo Hida (2000-2008) - Fixed-Precision Implementation

**Papers**:
- "Algorithms for Quad-Double Precision Floating Point Arithmetic" (Hida, Li, Bailey, 2000)
- "Library for Double-Double and Quad-Double Arithmetic" (Bailey, Hida, 2008)

**Institution**: Lawrence Berkeley National Laboratory (LBNL)

### Key Contributions

#### Fixed-Size Types

**Double-Double (dd)**:
- 2 components (high, low)
- ~31 decimal digits precision
- ~106 bits of significand
- Format: (1, 11, 106) - 1 sign bit, 11 exponent bits, 106 fraction bits

**Quad-Double (qd)**:
- 4 components
- ~62 decimal digits precision
- ~212 bits of significand
- Format: (1, 11, 212)

#### Hand-Crafted Algorithms

Bailey and Hida created optimized implementations for specific sizes, carefully orchestrating the sequence of EFT operations.

**From Universal's `dd_impl.hpp`** (lines 3-7):
```cpp
// dd_impl.hpp: implementation of the double-double floating-point number system described in
// Sherry Li, David Bailey, LBNL, "Library for Double-Double and Quad-Double Arithmetic", 2008
// https://www.researchgate.net/publication/228570156_Library_for_Double-Double_and_Quad-Double_Arithmetic
// Adapted core subroutines from QD library by Yozo Hida
```

**Addition** (lines 168-176):
```cpp
dd& operator+=(const dd& rhs) {
    double s2;
    hi = two_sum(hi, rhs.hi, s2);
    if (std::isfinite(hi)) {
        double t2, t1 = two_sum(lo, rhs.lo, t2);
        lo = two_sum(s2, t1, t1);
        t1 += t2;
        three_sum(hi, lo, t1);
    }
    return *this;
}
```

**Multiplication** (lines 202-226):
```cpp
dd& operator*=(const dd& rhs) {
    double p[7];
    // e powers in p = 0, 1, 1, 1, 2, 2, 2
    p[0] = two_prod(hi, rhs.hi, p[1]);
    if (std::isfinite(p[0])) {
        p[2] = two_prod(hi, rhs.lo, p[4]);
        p[3] = two_prod(lo, rhs.hi, p[5]);
        p[6] = lo * rhs.lo;

        // Carefully accumulate and renormalize
        three_sum(p[1], p[2], p[3]);
        p[2] += p[4] + p[5] + p[6];
        three_sum(p[0], p[1], p[2]);

        hi = p[0];
        lo = p[1];
    }
    return *this;
}
```

**Quad-Double Multiplication** (lines 1177-1209):
```cpp
inline void qd_mul(const dd& a, const dd& b, double p[4]) {
    double p4, p5, p6, p7;

    // powers of e - 0, 1, 1, 1, 2, 2, 2, 3
    p[0] = two_prod(a.high(), b.high(), p[1]);
    if (std::isfinite(p[0])) {
        p[2] = two_prod(a.high(), b.low(), p4);
        p[3] = two_prod(a.low(), b.high(), p5);
        p6 = two_prod(a.low(), b.low(), p7);

        // Systematic accumulation with error tracking
        three_sum(p[1], p[2], p[3]);
        three_sum(p4, p5, p6);
        p[2] = two_sum(p[2], p4, p4);
        three_sum(p[3], p4, p5);
        p[3] = two_sum(p[3], p7, p7);
        p4 += (p6 + p7);

        renorm(p[0], p[1], p[2], p[3], p4);
    }
}
```

#### Renormalization

Functions like `renorm()` maintain the non-overlapping property after arithmetic operations, ensuring components stay properly aligned.

### Production Library

The QD library provides:
- C++ and Fortran-90 interfaces
- Drop-in replacement for `double` in scientific codes
- Comprehensive math library (transcendental functions, etc.)
- Optimized for performance in HPC environments

### Philosophy

**Production-ready, fast, predictable** - Fixed precision for scientific computing where requirements are known upfront. Optimized for scenarios where the precision level is determined before computation begins.

---

## 3. Jonathan Shewchuk (1996-1997) - Adaptive Precision

**Papers**:
- "Robust Adaptive Floating-Point Geometric Predicates" (ACM Symposium on Computational Geometry, 1996)
- "Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates" (Discrete & Computational Geometry, 1997)

**Institution**: University of California, Berkeley

### Key Contributions

#### Adaptive Algorithms

Precision grows **on-demand** based on actual computational need rather than predetermined bounds:

- **`grow_expansion(e, elen, b, h)`**: Incrementally adds component `b` to expansion `e`
  - Only allocates/computes additional precision when needed
  - Returns new expansion length

- **`scale_expansion(e, elen, b, h)`**: Multiplies expansion by scalar
  - Adaptive growth during multiplication

- **`expansion_sum(e, elen, f, flen, h)`**: Adds two expansions
  - Result precision adapts to input characteristics

#### Dynamic Behavior

- **Typical case**: Most geometric predicates terminate with just 2-3 components
- **Pathological case**: Automatically extends precision when near-degenerate configurations detected
- **Lazy evaluation**: Doesn't pre-allocate worst-case storage

#### Application Domain

Designed specifically for **computational geometry** predicates:

**Orientation Test**: Determines whether a point lies to the left of, right of, or on a line/plane
```
sign(det([x1 y1 1]
         [x2 y2 1]
         [x3 y3 1]))
```

**Incircle Test**: Determines whether a point lies inside, outside, or on a circle
```
sign(det([x1 y1 x1²+y1² 1]
         [x2 y2 x2²+y2² 1]
         [x3 y3 x3²+y3² 1]
         [x4 y4 x4²+y4² 1]))
```

These predicates must be **exact** - wrong answers break Delaunay triangulation.

#### Error-Bound Termination

From Shewchuk's paper: "The error in the computed result is bounded by the magnitude of the final expansion term."

Computation stops when:
1. Error bound satisfied, OR
2. Result is definitively non-zero (sign determined)

### Relationship to Priest

Shewchuk explicitly acknowledges building on Priest's work:

> "This approach uses an unusual approach to exact arithmetic, **pioneered by Douglas Priest** and sped by me."

Key differences:
- **Priest**: Defined the theory and primitives
- **Shewchuk**: Made them adaptive and practical for geometry

### Public Code

Freely available C implementations:
- 2D and 3D orientation predicates
- 2D and 3D incircle predicates
- Robust Delaunay triangulation (Triangle mesh generator - 2003 Wilkinson Prize winner)

**URLs**:
- https://www.cs.cmu.edu/~quake/robust.html
- https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf

### Philosophy

**Adaptive and efficient** - Do only as much work as necessary to guarantee correctness. Optimize for the common case while handling pathological cases correctly.

---

## Key Algorithmic Differences

| Aspect | Priest (Theory) | Bailey/Hida (dd/qd) | Shewchuk (Adaptive) |
|--------|-----------------|---------------------|---------------------|
| **Precision** | Variable (conceptual) | Fixed (2 or 4 components) | Dynamic (grows as needed) |
| **Primitives** | Defined EFTs | Uses EFTs in fixed patterns | Uses EFTs with expansion growth |
| **Storage** | Theoretical | `std::array<double, N>` | `std::vector<double>` (dynamic) |
| **Performance** | N/A (theoretical) | **Fastest** - optimized for fixed size | Fast for typical, slower for pathological |
| **Predictability** | N/A | Constant time/space | Variable time/space |
| **Use Case** | Academic foundation | Scientific computing (HPC, physics) | Computational geometry |
| **Implementation** | Algorithms described | Production C++/Fortran library | C code for geometric predicates |
| **Error Control** | Proven bounds | Predetermined precision | Adaptive until bound met |
| **Optimization** | Proofs | Hand-tuned for N=2,4 | Optimized for common case |

---

## Primitive Operations - Shared Foundation

All three approaches use **Priest's error-free transformations** as building blocks.

### two_sum (Knuth, refined by Priest)

Exact addition of two floating-point numbers:

```cpp
double two_sum(double a, double b, double& err) {
    double s = a + b;
    double bb = s - a;
    err = (a - (s - bb)) + (b - bb);
    return s;
}
```

Properties:
- `a + b = s + err` exactly
- `|err| ≤ ulp(s)/2`
- Works for any IEEE-754 double

### quick_two_sum (Dekker)

Faster variant when magnitude ordering known (`|a| ≥ |b|`):

```cpp
double quick_two_sum(double a, double b, double& err) {
    double s = a + b;
    err = b - (s - a);
    return s;
}
```

### two_prod (Veltkamp/Dekker, refined by Priest)

Exact multiplication:

```cpp
double two_prod(double a, double b, double& err) {
    double p = a * b;
    err = fma(a, b, -p);  // Using FMA if available
    return p;
}
```

Without FMA, requires splitting:
```cpp
double two_prod(double a, double b, double& err) {
    const double split = 134217729.0;  // 2^27 + 1
    double t = a * split;
    double a_hi = t - (t - a);
    double a_lo = a - a_hi;
    t = b * split;
    double b_hi = t - (t - b);
    double b_lo = b - b_hi;

    double p = a * b;
    err = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
    return p;
}
```

### Usage in Universal Library

From `dd_impl.hpp`:

```cpp
// Line 170: Addition using two_sum
hi = two_sum(hi, rhs.hi, s2);

// Line 205: Multiplication using two_prod
p[0] = two_prod(hi, rhs.hi, p[1]);

// Line 1058: Normalization using quick_two_sum
hi = quick_two_sum(hi, lo, lo);

// Line 1109: Creating dd from double addition
s = two_sum(a, b, e);
return dd(s, e);
```

These are **Priest's primitives**, implemented by Bailey/Hida in fixed-size types, and used by Shewchuk in adaptive algorithms.

---

## Performance Trade-offs

### Bailey/Hida (dd/qd)

**Advantages**:
- ✅ **Fastest** for known precision requirements
- ✅ Predictable, constant-time performance
- ✅ Minimal memory overhead
- ✅ Cache-friendly (fixed size)
- ✅ Easy to vectorize/parallelize

**Disadvantages**:
- ❌ Wastes computation if less precision needed
- ❌ Insufficient if more precision required
- ❌ Must commit to precision level upfront

**Best for**: Scientific computing, numerical PDE solvers, physics simulations where precision requirements are well-understood.

### Shewchuk (Adaptive)

**Advantages**:
- ✅ **Optimal** for varying precision needs
- ✅ Fast for typical cases (2-3 components)
- ✅ Handles pathological cases automatically
- ✅ No precision commitment needed upfront
- ✅ Guaranteed correctness (exact predicates)

**Disadvantages**:
- ❌ 2-3.5× slower than Bailey when fully extended (based on literature)
- ❌ Variable performance (harder to predict)
- ❌ Dynamic allocation overhead
- ❌ Less cache-friendly

**Best for**: Computational geometry, mesh generation, any scenario where precision needs vary widely between common and rare cases.

### Priest (Theoretical)

**Role**: Foundation for both approaches
- Proved what's possible
- Established error bounds
- Created the algorithmic building blocks

---

## Implementation in Universal Library

### Current Status

Universal currently uses **Bailey/Hida's approach** for dd and qd (hand-crafted, fixed-precision implementations).

From `include/sw/universal/number/dd/dd_impl.hpp`:
```cpp
// dd is an unevaluated pair of IEEE-754 doubles
// that provides a (1,11,106) floating-point triple
class dd {
public:
    static constexpr unsigned nbits = 128;
    static constexpr unsigned es = 11;
    static constexpr unsigned fbits = 106;
    // ...
protected:
    double hi, lo;
};
```

### Proposed Architecture

The design documents in `docs/priest.md` and `docs/multi-component-arithmetic.md` propose a **hybrid approach**:

1. **Keep dd/qd** (Bailey/Hida) for fast, fixed-precision work
2. **Add td** (triple-double, 3 components) using same pattern
3. **Implement `priest` class** for adaptive precision (Shewchuk-style)
4. **Share common expansion machinery** across all types

This provides:
- **Performance** (Bailey) when precision is known
- **Flexibility** (Shewchuk/Priest) when precision varies
- **Code reuse** through shared EFT primitives
- **Smooth upgrade path** from fixed to adaptive precision

### Proposed Type Hierarchy

```
Priest EFTs (two_sum, two_prod, quick_two_sum)
        ↓
expansion<N> template (shared machinery)
        ↓
    ┌───┴───┬───────┬─────────┐
   dd(2)  td(3)   qd(4)    priest(dynamic)
```

**Fixed types** (dd, td, qd):
- Fast, predictable
- `std::array<double, N>` storage
- Hand-optimized algorithms

**Adaptive type** (priest):
- Flexible, correct
- `std::vector<double>` storage
- Dynamic precision growth

---

## References

### Papers

1. **Douglas M. Priest** (1991)
   "On Properties of Floating Point Arithmetics: Numerical Stability and the Cost of Accurate Computations"
   Ph.D. Dissertation, University of California, Berkeley
   Mathematics Genealogy Project: https://www.mathgenealogy.org/id.php?id=32602

2. **Yozo Hida, Xiaoye S. Li, David H. Bailey** (2000)
   "Algorithms for Quad-Double Precision Floating Point Arithmetic"
   Proceedings of the 15th IEEE Symposium on Computer Arithmetic (ARITH-15)
   https://www.davidhbailey.com/dhbpapers/quad-double.pdf

3. **David H. Bailey** (2008)
   "Library for Double-Double and Quad-Double Arithmetic"
   LBNL Technical Report
   https://www.davidhbailey.com/dhbpapers/qd.pdf
   https://www.researchgate.net/publication/228570156_Library_for_Double-Double_and_Quad-Double_Arithmetic

4. **Jonathan Richard Shewchuk** (1996)
   "Robust Adaptive Floating-Point Geometric Predicates"
   Proceedings of the Twelfth Annual Symposium on Computational Geometry, ACM
   https://dl.acm.org/doi/10.1145/237218.237337

5. **Jonathan Richard Shewchuk** (1997)
   "Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates"
   Discrete & Computational Geometry 18:305-363
   https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf
   https://link.springer.com/article/10.1007/PL00009321

### Software

1. **QD Library** (Bailey/Hida)
   https://www.davidhbailey.com/dhbsoftware/
   C++ and Fortran-90 implementation of dd and qd

2. **Shewchuk's Geometric Predicates**
   https://www.cs.cmu.edu/~quake/robust.html
   Public domain C code for robust geometric predicates

3. **Universal Numbers Library** (Stillwater Supercomputing)
   https://github.com/stillwater-sc/universal
   C++ template library including dd, qd, and (planned) priest implementations

### Additional Reading

- **William Kahan**: Various papers on floating-point arithmetic (IEEE-754 architect)
- **Dekker**: "A Floating-Point Technique for Extending the Available Precision" (1971)
- **Knuth**: "The Art of Computer Programming, Vol. 2: Seminumerical Algorithms" (Section 4.2.2)

---

## Conclusion

The three approaches are complementary rather than competing:

- **Priest** provided the theoretical foundation and primitive operations
- **Bailey/Hida** created production-quality fixed-precision implementations
- **Shewchuk** developed adaptive algorithms for geometry where precision needs vary

For Universal, the goal is to provide **all three paradigms**:
- Fixed precision (dd, td, qd) for performance
- Adaptive precision (priest) for flexibility
- Shared primitives for correctness and maintainability

This gives users the best tool for each scenario while maintaining a consistent API and shared implementation infrastructure.

---

**Document Version**: 1.0
**Date**: 2025-10-17
**Author**: Generated for Universal Numbers Library
**Status**: Reference documentation for multi-component arithmetic implementations
