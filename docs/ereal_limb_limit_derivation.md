# Ereal Maximum Limb Count: Mathematical Derivation

**Document**: Technical analysis of the 19-limb constraint for ereal multi-component arithmetic
**Date**: 2025-01-04
**Author**: Analysis based on Shewchuk's expansion arithmetic theory

---

## Executive Summary

The `ereal` type is limited to a maximum of **19 limbs** (not 38 as might be expected) due to a subtle but critical constraint in Shewchuk's expansion arithmetic: error terms from two-sum operations on the smallest limb must remain representable as normal IEEE-754 doubles.

**Key Finding**: The constraint applies to **all possible starting magnitudes** (including values near 1.0), not just the maximum representable double. This reduces the theoretical limit from ~38 limbs to 19 limbs.

---

## Table of Contents

1. [Background: Shewchuk's Expansion Arithmetic](#background)
2. [The Naive Analysis (Why 38 Seems Correct)](#naive-analysis)
3. [The Subtle Error (Why 19 Is Actually Correct)](#subtle-error)
4. [Mathematical Derivation](#mathematical-derivation)
5. [Practical Examples](#practical-examples)
6. [Implementation Constraints](#implementation-constraints)
7. [References](#references)

---

## Background: Shewchuk's Expansion Arithmetic {#background}

### What is an Expansion?

An **expansion** is a sequence of non-overlapping floating-point numbers that together represent a high-precision value:

```cpp
value = limb[0] + limb[1] + limb[2] + ... + limb[n]
```

**Key properties**:
1. **Non-overlapping**: Adjacent limbs differ by at least 53 bits (one mantissa width)
2. **Ordered**: |limb[0]| ≥ |limb[1]| ≥ |limb[2]| ≥ ... ≥ |limb[n]|
3. **Error-free**: Each limb captures rounding errors from the previous limb

### Two-Sum Algorithm

The foundation of expansion arithmetic is the **two-sum** algorithm, which computes the exact sum of two doubles:

```cpp
// Fast-Two-Sum (Dekker, 1971):
s = a + b;           // Rounded sum (what hardware gives you)
e = (a - s) + b;     // Exact error (what was lost to rounding)
```

**Critical requirement**: Both `s` and `e` must be **representable as normal IEEE-754 doubles**.

If `e` underflows below `DBL_MIN` (≈ 2^-1022), the error is lost and the expansion arithmetic breaks down.

---

## The Naive Analysis (Why 38 Seems Correct) {#naive-analysis}

### The Tempting Calculation

IEEE-754 double-precision has:
- **Largest normal**: `DBL_MAX` ≈ 2^1023 ≈ 10^308
- **Smallest normal**: `DBL_MIN` ≈ 2^-1022 ≈ 10^-308
- **Total range**: 2^(1023-(-1022)) = 2^2045 ≈ 10^616

Each limb adds approximately 53 bits of precision (the mantissa width), so:

```
Total range in bits: 2045 bits
Bits per limb: 53 bits
Maximum limbs: 2045 / 53 ≈ 38.6 limbs
```

**This suggests ~38 limbs should work!**

### The Flawed Assumption

This calculation assumes you can **directly use the full range** of doubles, spacing limbs from 2^1023 down to 2^-1022:

```
limb[0]:  2^1023      (top of range)
limb[1]:  2^(1023-54) (54 bits smaller, arbitrary spacing)
limb[2]:  2^(1023-108)
...
limb[37]: 2^-1022     (bottom of range)
```

**Why this is wrong**: This is **not** how Shewchuk's expansion arithmetic works!

---

## The Subtle Error (Why 19 Is Actually Correct) {#subtle-error}

### How Expansions Are Actually Built

Expansions are constructed through **repeated error extraction**, not by directly choosing exponents:

**Process**:
1. Start with a value `x` at some magnitude (could be near 1.0, 10^8, or any value)
2. Round it to create `limb[0]`
3. Compute the rounding error using two-sum → this becomes `limb[1]`
4. Repeat: the error from `limb[1]` becomes `limb[2]`, and so on

**Each successive limb is approximately 53 bits smaller** because:
- Rounding error is at most 1 ULP (unit in last place)
- For a value at exponent E, 1 ULP = 2^(E-52)
- Therefore, error magnitude ≈ 2^(E-53)

### The Critical Insight

**The expansion must work for ANY starting magnitude**, not just `DBL_MAX`!

Consider a value near 1.0:
```
Starting value: 1.0 = 2^0

limb[0]:  2^0       (magnitude ~1)
limb[1]:  2^-53     (error from limb[0])
limb[2]:  2^-106    (error from limb[1])
limb[3]:  2^-159
...
limb[18]: 2^-954
limb[19]: 2^-1007   ✓ Still above DBL_MIN (2^-1022)
limb[20]: 2^-1060   ✗ BELOW DBL_MIN! UNDERFLOW!
```

**At limb[20]**, the value underflows below `DBL_MIN` and:
- Cannot be represented as a normalized double
- May become denormalized (loses precision)
- Two-sum operations produce incorrect error terms
- **The entire expansion arithmetic breaks down**

---

## Mathematical Derivation {#mathematical-derivation}

### General Limb Magnitude Formula

For an expansion starting at magnitude M (where M = 2^E for some exponent E):

```
limb[0]: M × 2^0        = M
limb[1]: M × 2^-53      (error from limb[0])
limb[2]: M × 2^-106     (error from limb[1])
...
limb[n]: M × 2^(-53n)   (error from limb[n-1])
```

### Constraint for Normal Representation

For `limb[n]` to be representable as a normal double:

```
M × 2^(-53n) >= DBL_MIN
M × 2^(-53n) >= 2^-1022
```

### Case 1: Starting from DBL_MAX (M = 2^1023)

```
2^1023 × 2^(-53n) >= 2^-1022
2^(1023-53n) >= 2^-1022
1023 - 53n >= -1022
53n <= 2045
n <= 38.6
```

**Result**: Up to **38 limbs** would work if expansions always started from `DBL_MAX`.

### Case 2: Starting from 1.0 (M = 2^0)

```
2^0 × 2^(-53n) >= 2^-1022
2^(-53n) >= 2^-1022
-53n >= -1022
53n <= 1022
n <= 19.28
```

**Result**: Only **19 limbs** work for values near 1.0.

### Case 3: General Case (M = 2^E for arbitrary E)

For the expansion to work for **any** starting magnitude:

```
2^E × 2^(-53n) >= 2^-1022

For all possible E (where -1022 ≤ E ≤ 1023):
Worst case is E = 0 (magnitude ~1.0)

2^(-53n) >= 2^-1022
n <= 19.28
```

**Conclusion**: To guarantee correctness for **all possible starting values**, we must limit to **n ≤ 19 limbs**.

### Why Not Adapt Based on Magnitude?

One might ask: "Why not allow 38 limbs for large values and 19 for small values?"

**Answer**: Type safety and implementation complexity.

- `ereal<maxlimbs>` is a **compile-time fixed type**
- Operations between `ereal<19>` and `ereal<38>` would be undefined
- Arithmetic must preserve the expansion property for all intermediate results
- A multiplication of two large values could produce a small value, requiring the smaller limit
- **The type must work correctly for its entire value range**

---

## Practical Examples {#practical-examples}

### Example 1: Value Near 1.0 with 19 Limbs (WORKS)

```cpp
ereal<19> x(1.0);  // Start with 1.0

// Limb magnitudes after successive operations:
limb[0]:  1.0 × 2^0    = 2^0     = 1.0
limb[1]:  1.0 × 2^-53  = 2^-53   ≈ 1.11e-16
limb[2]:  1.0 × 2^-106 = 2^-106  ≈ 1.23e-32
...
limb[18]: 1.0 × 2^-954 = 2^-954  ≈ 1.89e-287
limb[19]: 1.0 × 2^-1007 = 2^-1007 ≈ 4.75e-304

DBL_MIN = 2^-1022 ≈ 2.23e-308

limb[19] = 2^-1007 > 2^-1022 ✓ SAFE
```

**All limbs remain representable as normal doubles.** ✓

### Example 2: Value Near 1.0 with 20 Limbs (FAILS)

```cpp
ereal<20> x(1.0);  // Hypothetical - would fail at compile time

limb[19]: 1.0 × 2^-1007 = 2^-1007 ≈ 4.75e-304  ✓ OK
limb[20]: 1.0 × 2^-1060 = 2^-1060 ≈ 9.21e-320  ✗ BELOW DBL_MIN!

DBL_MIN = 2^-1022 ≈ 2.23e-308

limb[20] = 2^-1060 < 2^-1022 ✗ UNDERFLOW!
```

**limb[20] underflows to zero or denormal**, breaking two-sum. ✗

### Example 3: Value Near DBL_MAX with 19 Limbs (WORKS)

```cpp
ereal<19> x(1e308);  // Near DBL_MAX ≈ 2^1023

limb[0]:  2^1023
limb[1]:  2^970
limb[2]:  2^917
...
limb[18]: 2^69    ≈ 5.9e20
limb[19]: 2^16    ≈ 65536

DBL_MIN = 2^-1022 ≈ 2.23e-308

limb[19] = 2^16 >> 2^-1022 ✓ SAFE (huge margin!)
```

**Even for maximum values, 19 limbs works perfectly.** ✓

### Example 4: Why Range-Based Thinking Fails

**Incorrect reasoning**:
> "If I start with 2^1023, I can go down to 2^-1022, that's 38 limbs worth of range!"

**Why it fails**:
```cpp
ereal<38> x(1e308);  // Starts near 2^1023
ereal<38> y(1.0);    // Starts near 2^0

// These are the SAME TYPE!
// But 'y' would have limbs below DBL_MIN
// Type must work for BOTH values

ereal<38> z = x * 0.0001;  // Now z is small
// z's small limbs would underflow
```

**The type must handle its entire value range, not just large values.**

---

## Implementation Constraints {#implementation-constraints}

### Static Assertion in Code

From `ereal_impl.hpp:78-82`:

```cpp
static_assert(maxlimbs <= 19,
    "ereal<maxlimbs>: maxlimbs must be <= 19 to maintain algorithmic correctness. "
    "Larger values cause the last limb to underflow below DBL_MIN, violating the "
    "non-overlapping property required by Shewchuk's expansion arithmetic. "
    "This results in incorrect two_sum/two_product operations and silent arithmetic errors.");
```

### Precision vs Limb Count

| Limbs | Mantissa Bits | Decimal Digits | Status |
|-------|---------------|----------------|--------|
| 4     | 212           | ~64            | ✓ Safe |
| 8     | 424           | ~127           | ✓ Safe |
| 12    | 636           | ~191           | ✓ Safe |
| 16    | 848           | ~255           | ✓ Safe |
| 19    | 1007          | ~303           | ✓ Safe (maximum) |
| 20    | 1060          | ~319           | ✗ **FAILS for values near 1.0** |
| 38    | 2014          | ~606           | ✗ **FAILS for values < 2^969** |

### Error Manifestations with maxlimbs > 19

If the limit were violated (without the static assertion):

1. **Silent Arithmetic Errors**:
   - Two-sum returns incorrect error terms (lost to underflow)
   - Non-overlapping property violated
   - Results appear valid but are mathematically wrong

2. **Precision Loss**:
   - Small limbs become denormalized (54-bit precision → fewer bits)
   - Gradual degradation rather than catastrophic failure
   - Hard to detect without rigorous testing

3. **Inconsistent Behavior**:
   - Works correctly for large values (near DBL_MAX)
   - Fails silently for small values (near 1.0)
   - Breaks the principle of uniform type behavior

---

## Why This Matters {#why-this-matters}

### Type Safety

```cpp
// This should work uniformly for all values:
ereal<n> x = ...;  // any value
ereal<n> y = ...;  // any value
ereal<n> z = x + y;  // must be correct regardless of magnitudes
```

**The type cannot have "safe regions" and "unsafe regions"** - it must work correctly for its entire value range.

### Mathematical Correctness

Shewchuk's algorithms provide **exact arithmetic** (within the limits of floating-point representation). This property must be preserved:

```cpp
// These are GUARANTEED to be exact (error-free):
a + b = (sum, error)  // two_sum
a * b = (prod, error) // two_product
```

If limbs underflow, these guarantees are **violated**, leading to:
- Incorrect geometric predicates (orient2d, orient3d)
- Wrong results in high-precision calculations
- Unreliable numerical algorithms

### Performance Considerations

While 19 limbs might seem limiting, it provides:
- **~303 decimal digits** of precision (more than quadruple-double's ~64 digits)
- **Algorithmic correctness** for all operations
- **Predictable behavior** across the entire value range

For applications needing more precision, consider:
- Arbitrary-precision libraries (MPFR, GMP)
- Symbolic computation (exact rational arithmetic)
- Extended exponent range formats (quadruple precision, custom formats)

---

## References {#references}

### Primary Sources

1. **Shewchuk, J. R.** (1997). *Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates*. Discrete & Computational Geometry, 18(3), 305-363.
   - Defines expansion arithmetic and two-sum/two-product algorithms
   - Establishes correctness requirements for multi-component arithmetic
   - Available: https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf

2. **Dekker, T. J.** (1971). *A Floating-Point Technique for Extending the Available Precision*. Numerische Mathematik, 18(3), 224-242.
   - Original two-sum algorithm (Fast-Two-Sum)
   - Foundation for expansion arithmetic

3. **Priest, D. M.** (1991). *Algorithms for Arbitrary Precision Floating Point Arithmetic*. Proceedings of the 10th Symposium on Computer Arithmetic.
   - Early work on multi-component arithmetic
   - Discusses precision limits and error propagation

### IEEE-754 Standard

4. **IEEE Standard 754-2008** for Floating-Point Arithmetic
   - Defines normal vs subnormal numbers
   - DBL_MIN = 2^-1022 (smallest normal double)
   - Rounding modes and error behavior

### Related Documentation

5. **Universal Numbers Library** - `ereal` implementation
   - `include/sw/universal/number/ereal/ereal_impl.hpp` (lines 34-61)
   - Static assertion enforcing maxlimbs ≤ 19
   - Implementation of Shewchuk's algorithms

---

## Appendix: Quick Reference {#quick-reference}

### Key Formulas

**Maximum limbs for starting magnitude M = 2^E:**
```
M × 2^(-53n) >= 2^-1022
n <= (1022 + E) / 53
```

**For E = 0 (magnitude ~1.0):**
```
n <= 1022 / 53 ≈ 19.28  →  n_max = 19
```

**For E = 1023 (magnitude ~DBL_MAX):**
```
n <= 2045 / 53 ≈ 38.6  →  n_max = 38 (but type must work for E=0!)
```

### Decision Tree

```
Q: How many limbs can I use?
├─ Will values ever be < 2^969?
│  ├─ Yes → Use maxlimbs ≤ 19
│  └─ No  → Could use up to ~38 (but type must be uniform!)
└─ Need uniform type behavior?
   └─ Yes → Use maxlimbs ≤ 19 (always)
```

### Common Misconceptions

❌ **Wrong**: "The range is 2^2045, so I can use 38 limbs"
✓ **Right**: "Each limb is 53 bits smaller, and the smallest must be ≥ DBL_MIN"

❌ **Wrong**: "I only work with large values, so I can use more limbs"
✓ **Right**: "The type must work for all values, including results of operations"

❌ **Wrong**: "I can check the magnitude and use variable limb counts"
✓ **Right**: "maxlimbs is a compile-time constant; the type is fixed"

---

## Conclusion

The 19-limb limit for `ereal` arises from a fundamental requirement of Shewchuk's expansion arithmetic: **all components and error terms must remain representable as normal IEEE-754 doubles**.

While naive analysis of the double-precision range suggests ~38 limbs might work, the actual constraint is more subtle: the expansion must work correctly for **any starting magnitude**, not just the maximum representable value.

For values near 1.0 (which are common in many applications), the 20th limb would have magnitude ~2^-1060, which is **below DBL_MIN** and therefore cannot be represented as a normal double. This violates the fundamental assumptions of the two-sum algorithm, leading to incorrect arithmetic.

The limit of 19 limbs provides:
- ✓ **~303 decimal digits** of precision (mantissa basis)
- ✓ **Algorithmic correctness** for all value magnitudes
- ✓ **Predictable behavior** across the type's entire range
- ✓ **Error-free arithmetic** properties of expansion arithmetic

This is not a limitation of the implementation, but rather a **fundamental mathematical constraint** of multi-component floating-point arithmetic using IEEE-754 doubles.

---

**Document Status**: Ready for review
**Last Updated**: 2025-01-04
**Location**: `docs/ereal_limb_limit_derivation.md`
