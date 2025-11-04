# ereal Mathlib Phase 1: Simple Functions Implementation

**Date:** 2025-11-03
**Objective:** Implement simple math functions with full adaptive precision
**Status:** Planning
**Branch:** v3.89
**Prerequisites:** Phase 0 complete (stub infrastructure in place)

---

## Executive Summary

Phase 1 refines the simplest mathlib functions from double-precision stubs to full adaptive-precision implementations. These functions can be implemented directly using ereal's existing comparison operators and component-wise operations without requiring complex transcendental algorithms.

**Target Functions:**
- **minmax** (min, max) - use comparison operators
- **truncate** (floor, ceil) - component-wise operations
- **classify** (fpclassify, isnan, isinf, etc.) - use ereal's native methods
- **numerics** (copysign, signbit, abs) - simple sign/component manipulation

**Out of Scope for Phase 1:**
- trunc, round (require summing all components)
- fractional (fmod, remainder - require division)
- hypot (requires sqrt)
- error_and_gamma (complex special functions)
- transcendentals (exp, log, pow, trig, hyperbolic)

---

## Background

### What Phase 0 Provided

All mathlib functions exist as stubs:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> sqrt(const ereal<maxlimbs>& x) {
    return ereal<maxlimbs>(std::sqrt(double(x)));
}
```

This provides ~15-17 decimal digits of precision (double precision).

### What Phase 1 Delivers

Selected functions will be implemented using ereal's native adaptive-precision capabilities:
- **Comparison-based functions**: Use ereal's `operator<`, `operator>`, etc.
- **Component-wise functions**: Operate on expansion components directly
- **Classification functions**: Use ereal's `isnan()`, `isinf()`, `iszero()`, etc.

**Precision improvement**: From ~15 digits (double) to full adaptive precision (limited only by maxlimbs)

---

## Available Infrastructure

### ereal Capabilities

**Expansion Arithmetic** (from `expansion_ops.hpp`):
- `linear_expansion_sum()` - addition
- `expansion_product()` - multiplication
- `expansion_quotient()` - division
- `expansion_reciprocal()` - 1/x
- `scale_expansion()` - scalar multiplication
- `compare_adaptive()` - adaptive comparison
- `renormalize_expansion()` - cleanup
- `compress_expansion()` - reduce components

**Comparison Operators** (ereal_impl.hpp:290-313):
```cpp
bool operator==(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs)
bool operator<(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs)
bool operator>(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs)
// ... etc
```
All use `compare_adaptive()` for full-precision comparison.

**Classification Methods** (ereal_impl.hpp:149-156):
```cpp
bool iszero() const noexcept
bool isone() const noexcept
bool ispos() const noexcept
bool isneg() const noexcept
bool isinf() const noexcept
bool isnan() const noexcept
```

**Component Access** (ereal_impl.hpp:163):
```cpp
const std::vector<double>& limbs() const noexcept
```

**Sign Access** (ereal_impl.hpp:160):
```cpp
int sign() const noexcept  // Returns -1 or 1
```

---

## Phase 1 Function Details

### 1. minmax.hpp

**Current Implementation (stub):**
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> min(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
    return ereal<maxlimbs>(std::min(double(x), double(y)));
}

template<unsigned maxlimbs>
inline ereal<maxlimbs> max(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
    return ereal<maxlimbs>(std::max(double(x), double(y)));
}
```

**Phase 1 Implementation:**
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> min(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
    return (x < y) ? x : y;
}

template<unsigned maxlimbs>
inline ereal<maxlimbs> max(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
    return (x > y) ? x : y;
}
```

**Benefits:**
- Uses adaptive-precision comparison
- Returns exact result with full precision
- Simple, elegant, correct

**Test Strategy:**
- Test with values that differ only in low-order components
- Example: `x = 1.0 + 1e-30`, `y = 1.0 + 2e-30`
- Verify min/max correctly identifies difference beyond double precision

---

### 2. truncate.hpp

**Target Functions:** floor(), ceil()
**Deferred:** trunc(), round() (require summing components)

**Current Implementation (stub):**
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> floor(const ereal<maxlimbs>& x) {
    return ereal<maxlimbs>(std::floor(double(x)));
}
```

**Phase 1 Implementation (based on qd_cascade pattern):**
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> floor(const ereal<maxlimbs>& x) {
    using namespace expansion_ops;

    // Get components
    const auto& limbs = x.limbs();
    if (limbs.empty()) return ereal<maxlimbs>(0.0);

    // Create result expansion
    std::vector<double> result_limbs(limbs.size(), 0.0);

    // Floor first component
    result_limbs[0] = std::floor(limbs[0]);

    // If first component unchanged, it's already integer, check next
    if (result_limbs[0] == limbs[0]) {
        for (size_t i = 1; i < limbs.size(); ++i) {
            result_limbs[i] = std::floor(limbs[i]);
            if (result_limbs[i] != limbs[i]) {
                // This component had fractional part, zero remaining
                break;
            }
        }
    }

    // Construct result from limbs
    ereal<maxlimbs> result;
    result = result_limbs[0];
    for (size_t i = 1; i < result_limbs.size(); ++i) {
        if (result_limbs[i] != 0.0) {
            result += ereal<maxlimbs>(result_limbs[i]);
        }
    }

    return result;
}
```

**Similar implementation for ceil()** (replace `std::floor` with `std::ceil`)

**Algorithm:**
1. Apply floor/ceil to first (most significant) component
2. If first component is already integer, check next component
3. Continue until finding fractional part or exhausting components
4. Zero all remaining components after fractional part found

**Benefits:**
- Preserves full precision of integer part
- Correctly handles values that are integers in expansion representation
- Matches qd_cascade behavior

**Test Strategy:**
- Test with pure integers (all components should be preserved)
- Test with small fractional parts beyond double precision
- Test edge cases near integer boundaries

---

### 3. classify.hpp

**Functions:** fpclassify, isnan, isinf, isfinite, isnormal, signbit

**Current Implementation (stub):**
```cpp
template<unsigned maxlimbs>
inline bool isnan(const ereal<maxlimbs>& x) {
    return std::isnan(double(x));
}
```

**Phase 1 Implementation:**
```cpp
template<unsigned maxlimbs>
inline bool isnan(const ereal<maxlimbs>& x) {
    return x.isnan();
}

template<unsigned maxlimbs>
inline bool isinf(const ereal<maxlimbs>& x) {
    return x.isinf();
}

template<unsigned maxlimbs>
inline bool isfinite(const ereal<maxlimbs>& x) {
    return !x.isinf() && !x.isnan();
}

template<unsigned maxlimbs>
inline bool isnormal(const ereal<maxlimbs>& x) {
    // A number is normal if it's finite, non-zero, and not subnormal
    // For ereal with expansion arithmetic, any non-zero finite value is "normal"
    return !x.iszero() && !x.isinf() && !x.isnan();
}

template<unsigned maxlimbs>
inline bool signbit(const ereal<maxlimbs>& x) {
    return x.isneg();
}

template<unsigned maxlimbs>
inline int fpclassify(const ereal<maxlimbs>& x) {
    if (x.isnan()) return FP_NAN;
    if (x.isinf()) return FP_INFINITE;
    if (x.iszero()) return FP_ZERO;
    // For ereal, everything else is normal (no subnormal representation)
    return FP_NORMAL;
}
```

**Benefits:**
- Uses ereal's native classification methods
- Correct for expansion arithmetic (no subnormals in expansion representation)
- Fast and simple

**Note on Subnormals:**
ereal doesn't have subnormal values in the IEEE-754 sense. Each component is a normal double, and the expansion can represent arbitrarily small values by adding more components. Therefore, `isnormal()` returns true for any non-zero finite value.

---

### 4. numerics.hpp

**Target Functions:** copysign, signbit
**Already Correct:** abs (implemented in ereal_impl.hpp:228)
**Deferred:** frexp, ldexp (require exponent manipulation)

**Current Implementation (stub):**
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> copysign(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
    return ereal<maxlimbs>(std::copysign(double(x), double(y)));
}
```

**Phase 1 Implementation:**
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> copysign(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
    if (x.sign() == y.sign()) {
        return x;
    } else {
        return -x;  // Use ereal's unary minus operator
    }
}

template<unsigned maxlimbs>
inline bool signbit(const ereal<maxlimbs>& x) {
    return x.isneg();
}
```

**Benefits:**
- Uses ereal's native sign() method and unary minus operator
- Preserves full precision (no conversion to/from double)
- Simple and elegant

**Note on abs():**
Already correctly implemented in ereal_impl.hpp:228:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> abs(const ereal<maxlimbs>& x) {
    return (x.isneg()) ? -x : x;
}
```
No changes needed - already full precision!

---

## Implementation Order

### Step 1: Update minmax.hpp
- Replace stubs with comparison-based implementation
- Update regression test with precision validation
- Build and verify

### Step 2: Update classify.hpp
- Replace stubs with ereal method-based implementation
- Update regression test
- Build and verify

### Step 3: Update numerics.hpp (partial)
- Implement copysign using sign() method
- Verify abs() is already correct
- Note: signbit is in classify.hpp
- Update regression test
- Build and verify

### Step 4: Update truncate.hpp (partial)
- Implement floor() component-wise
- Implement ceil() component-wise
- Leave trunc() and round() as stubs (mark as TODO for Phase 2)
- Update regression test
- Build and verify

### Step 5: Documentation
- Update CHANGELOG
- Create session log
- Update implementation plan status

---

## Testing Strategy

### Precision Validation Pattern

Each test will verify that the Phase 1 implementation achieves precision beyond double:

```cpp
// Test min with values differing beyond double precision
ereal<> x, y, result;

// Set up values that differ only in low-order components
// (requires expansion arithmetic construction)
x = ereal<>(1.0) + ereal<>(1e-100);
y = ereal<>(1.0) + ereal<>(2e-100);

result = min(x, y);

// Verify result equals x (not y)
// This would fail if using double-precision comparison
if (result == x && result != y) {
    std::cout << "Precision validation PASSED\n";
} else {
    std::cout << "Precision validation FAILED\n";
}
```

### Test Categories

For each function:
1. **Basic functionality** - correct result for simple inputs
2. **Precision validation** - correct result beyond double precision
3. **Edge cases** - zero, near-zero, large values
4. **Special values** - if applicable (NaN, inf)

### Regression Test Updates

Replace `MANUAL_TESTING` stubs with actual test code:
```cpp
#if MANUAL_TESTING
    // Phase 1: Precision validation tests
    bool success = true;

    // Test 1: Basic functionality
    success &= test_basic_minmax();

    // Test 2: Precision beyond double
    success &= test_precision_minmax();

    // Test 3: Edge cases
    success &= test_edge_cases_minmax();

    if (success) {
        std::cout << "Phase 1: Full precision validation - PASS\n";
    } else {
        std::cout << "Phase 1: Full precision validation - FAIL\n";
        nrOfFailedTestCases++;
    }

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else
    // Automated regression tests (future)
#endif
```

---

## Success Criteria

### Phase 1 Complete When:

1. ✅ minmax.hpp: min/max implemented using comparison operators
2. ✅ classify.hpp: All classification functions use ereal's native methods
3. ✅ numerics.hpp: copysign implemented using sign() method
4. ✅ truncate.hpp: floor/ceil implemented component-wise
5. ✅ All updated functions compile without errors
6. ✅ All regression tests pass with precision validation
7. ✅ Tests demonstrate precision beyond double (show difference at ~30+ digits)
8. ✅ Documentation updated (CHANGELOG, session log)
9. ✅ No regressions in existing ereal functionality

---

## Deferred to Future Phases

### Phase 2 Candidates (Medium Complexity)
- **truncate**: trunc(), round() - require summing expansion components
- **fractional**: fmod(), remainder() - require expansion_quotient
- **hypot**: requires sqrt implementation
- **cbrt**: cube root algorithm
- **exponent**: exp, exp2, exp10, expm1 - Taylor series
- **logarithm**: log, log2, log10, log1p - argument reduction + series
- **pow**: general power function

### Phase 3 Candidates (High Complexity)
- **sqrt**: Newton-Raphson with expansion arithmetic
- **trigonometry**: sin, cos, tan, asin, acos, atan - argument reduction + CORDIC/series
- **hyperbolic**: sinh, cosh, tanh, etc. - series expansion

### Phase 4
- **Precision control API**: Request specific precision for operations

---

## Risk Assessment

### Low Risk ✅
- minmax: Trivial, uses existing operators
- classify: Uses existing ereal methods
- copysign: Simple sign manipulation

### Medium Risk ⚠️
- truncate (floor/ceil): Component-wise logic needs careful testing
- May need helper function to construct ereal from limb vector

### Mitigation
- Follow qd_cascade implementation closely
- Test incrementally (one function at a time)
- Extensive precision validation tests

---

## Implementation Notes

### Building ereal from Limb Vector

May need helper to construct ereal from `std::vector<double>`:

```cpp
// Option 1: Use += operator repeatedly
ereal<maxlimbs> result;
result = limbs[0];
for (size_t i = 1; i < limbs.size(); ++i) {
    result += ereal<maxlimbs>(limbs[i]);
}

// Option 2: Direct limb assignment (if accessor available)
ereal<maxlimbs> result;
result.set_limbs(limbs);  // May need to add this method
```

Will investigate ereal API to determine best approach.

### Avoiding Double Conversion

Key principle: **Never convert to/from double in Phase 1 implementations**

- ❌ Bad: `return ereal<maxlimbs>(std::function(double(x)));`
- ✅ Good: `return (x < y) ? x : y;`

---

## Timeline Estimate

**Total: 4-6 hours**

- minmax.hpp: 30 minutes (implementation + test)
- classify.hpp: 45 minutes (implementation + test)
- numerics.hpp: 30 minutes (copysign + test)
- truncate.hpp: 2 hours (floor/ceil implementation + testing)
- Documentation: 1 hour
- Buffer: 1 hour

---

## References

1. **Shewchuk (1997)**: "Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates"
2. **qd_cascade implementation**: Reference for component-wise floor/ceil
3. **expansion_ops.hpp**: Comprehensive expansion arithmetic operations
4. **ereal_impl.hpp**: Existing ereal capabilities and methods

---

## Next Steps

After approval:
1. Begin with minmax.hpp (simplest)
2. Continue with classify.hpp
3. Implement numerics.hpp (copysign)
4. Tackle truncate.hpp (floor/ceil)
5. Update all regression tests
6. Comprehensive build and test
7. Documentation

---

**Plan Status:** READY FOR REVIEW
**Created:** 2025-11-03
**Author:** Claude Code (with Theodore Omtzigt)
