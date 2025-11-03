# ereal Mathlib Phase 3: High-Complexity Functions (Roots)

**Date:** 2025-11-03
**Objective:** Implement high-complexity root functions using Newton-Raphson iteration
**Status:** Planning
**Branch:** v3.89
**Prerequisites:** Phase 1 & 2 complete (18 functions at full adaptive precision, frexp/ldexp available)

---

## Executive Summary

Phase 3 implements the most critical high-complexity functions: square root, cube root, and hypotenuse. These functions require iterative algorithms (Newton-Raphson) and are essential building blocks for numerical computing and future transcendental functions.

**Target Functions:**
- **sqrt** - Square root via Newton-Raphson (highest priority)
- **cbrt** - Cube root using frexp/ldexp + Newton-Raphson
- **hypot** - Hypotenuse using sqrt

**Deferred to Future:**
- **Transcendentals** (exp, log, pow) - Taylor series, very complex
- **Trigonometry** (sin, cos, tan, etc.) - Argument reduction + CORDIC
- **Hyperbolic** (sinh, cosh, tanh, etc.) - Series expansion

**Estimated Duration:** 6-8 hours

---

## Background

### What Phases 1 & 2 Provided

**Phase 1** (12 functions):
- minmax, classify, copysign, floor, ceil

**Phase 2** (6 functions):
- trunc, round, frexp, ldexp, fmod, remainder

**Cumulative**: 18 of 50+ functions at full adaptive precision (36%)

### Critical Enablers for Phase 3

**From Phase 2**:
- ✅ `frexp()` - Extract exponent (essential for cbrt)
- ✅ `ldexp()` - Power-of-2 scaling (essential for cbrt)
- ✅ Division operator - Uses `expansion_quotient` (essential for Newton-Raphson)

**From Phase 1**:
- ✅ Comparison operators - For zero/negative checks

### Why These Functions Are Critical

1. **sqrt** - Most fundamental:
   - Used in: hypot, norms, distances, standard deviation
   - Building block for: many numerical algorithms
   - Essential for: geometry, statistics, optimization

2. **cbrt** - Cube root:
   - Used in: volume calculations, cubic equations
   - Complex algorithm: requires frexp/ldexp for range reduction

3. **hypot** - Hypotenuse without overflow:
   - Used in: vector norms, complex arithmetic
   - Critical property: `hypot(x, y) = sqrt(x² + y²)` without intermediate overflow

---

## Available Infrastructure

### Newton-Raphson Method

**General form**: Find x such that f(x) = 0

**For sqrt**: f(x) = x² - a = 0
- Iteration: x' = (x + a/x) / 2
- Convergence: Quadratic (doubles correct digits each iteration)

**For cbrt**: f(x) = x³ - a = 0
- Iteration: x' = (2x + a/x²) / 3
- Or: x' = x + x(1 - ax³)/3 (using reciprocal form)
- Convergence: Quadratic

### Precision Requirements

**ereal adaptive precision**:
- 1 component: ~53 bits
- N components: ~N × 53 bits
- maxlimbs: up to maxlimbs × 53 bits

**Newton-Raphson iterations needed**:
- Initial approximation: ~53 bits (from high component)
- Each iteration doubles precision
- For N components: need ⌈log₂(N)⌉ iterations after initial guess

**Safe iteration count**:
```cpp
// For ereal<maxlimbs>: approximately maxlimbs components
int iterations = 3 + static_cast<int>(std::log2(maxlimbs + 1));
// Example: maxlimbs=1024 → 3 + 10 = 13 iterations
// This provides 2^13 ≈ 8192x initial precision = plenty of margin
```

---

## Phase 3 Function Details

### 1. sqrt.hpp - Square Root

**Current Status**: Stub using `std::sqrt(double(x))`

**Algorithm**: Newton-Raphson iteration

```
For sqrt(a):
  f(x) = x² - a
  f'(x) = 2x
  x_{n+1} = x_n - f(x_n)/f'(x_n) = x_n - (x_n² - a)/(2x_n)
         = (2x_n² - x_n² + a)/(2x_n)
         = (x_n² + a)/(2x_n)
         = (x_n + a/x_n) / 2
```

**Implementation**:

```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> sqrt(const ereal<maxlimbs>& a) {
    // Handle special cases
    if (a.iszero()) return ereal<maxlimbs>(0.0);
    if (a.isneg()) {
        // TODO: Return NaN or throw exception when ereal supports it
        return a;  // For now, return input
    }

    // Initial approximation from high component
    const auto& limbs = a.limbs();
    ereal<maxlimbs> x = std::sqrt(limbs[0]);

    // Determine number of iterations based on desired precision
    // Each iteration doubles correct digits
    // For adaptive precision with maxlimbs, need log2(maxlimbs) + margin
    int iterations = 3 + static_cast<int>(std::log2(maxlimbs + 1));

    // Newton-Raphson: x = (x + a/x) / 2
    for (int i = 0; i < iterations; ++i) {
        x = (x + a / x) * 0.5;
    }

    return x;
}
```

**Key Points**:
- Initial guess: `sqrt(high_component)`
- Iteration: `x = (x + a/x) / 2`
- Uses ereal division (expansion_quotient)
- Adaptive iteration count based on maxlimbs

**Test Cases**:
- sqrt(4.0) = 2.0 (exact)
- sqrt(2.0) ≈ 1.414213562373095...
- sqrt(0.0) = 0.0
- sqrt(negative) → error handling
- sqrt(very large) - no overflow
- sqrt(very small) - maintains precision

---

### 2. cbrt.hpp - Cube Root

**Current Status**: Stub using `std::cbrt(double(x))`

**Algorithm**: Range reduction + Newton-Raphson

**Strategy** (from qd_cascade):
1. Handle special cases (zero, infinity, NaN)
2. Extract sign (cbrt preserves sign)
3. Use frexp to extract exponent: a = r × 2^e where 0.5 ≤ r < 1
4. Adjust exponent to be divisible by 3
5. Compute cbrt(r) using Newton-Raphson
6. Scale result: result = cbrt(r) × 2^(e/3)
7. Restore sign

**Newton-Raphson for cbrt**:
```
For cbrt(a) = a^(1/3):
  Method 1: Direct
    x_{n+1} = (2x_n + a/x_n²) / 3

  Method 2: Reciprocal (from qd_cascade)
    Find y = a^(-1/3), then return 1/y
    Iteration: y_{n+1} = y_n + y_n(1 - a·y_n³)/3
```

**Implementation**:

```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> cbrt(const ereal<maxlimbs>& a) {
    // Handle special cases
    if (a.iszero()) return ereal<maxlimbs>(0.0);
    if (a.isnan() || a.isinf()) return a;

    // Extract and save sign (cbrt preserves sign)
    bool negative = a.isneg();
    ereal<maxlimbs> abs_a = negative ? -a : a;

    // Use frexp to get: abs_a = r × 2^e where 0.5 ≤ r < 1
    int e;
    ereal<maxlimbs> r = frexp(abs_a, &e);

    // Adjust exponent to be divisible by 3
    // This keeps r in range [0.125, 1.0)
    while (e % 3 != 0) {
        ++e;
        r = ldexp(r, -1);
    }

    // At this point: 0.125 ≤ r < 1.0
    // Initial approximation for cbrt(r)
    const auto& r_limbs = r.limbs();
    ereal<maxlimbs> x = std::cbrt(r_limbs[0]);

    // Determine iterations
    int iterations = 3 + static_cast<int>(std::log2(maxlimbs + 1));

    // Newton-Raphson: x = (2x + r/x²) / 3
    for (int i = 0; i < iterations; ++i) {
        ereal<maxlimbs> x_squared = x * x;
        x = (ereal<maxlimbs>(2.0) * x + r / x_squared) / ereal<maxlimbs>(3.0);
    }

    // Scale by 2^(e/3)
    x = ldexp(x, e / 3);

    // Restore sign
    if (negative) x = -x;

    return x;
}
```

**Key Points**:
- Uses frexp/ldexp for range reduction
- Newton-Raphson on reduced range [0.125, 1.0)
- Preserves sign (unlike sqrt)
- Scales result appropriately

**Test Cases**:
- cbrt(8.0) = 2.0 (exact)
- cbrt(-8.0) = -2.0 (negative preserved)
- cbrt(27.0) = 3.0
- cbrt(2.0) ≈ 1.259921049894873...
- cbrt(0.0) = 0.0
- cbrt(very large) - no overflow

---

### 3. hypot.hpp - Hypotenuse

**Current Status**: Stub using `std::hypot(double(x), double(y))`

**Purpose**: Compute sqrt(x² + y²) without intermediate overflow

**Algorithm**:

```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> hypot(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
    // Use sqrt from Phase 3
    ereal<maxlimbs> x2 = x * x;
    ereal<maxlimbs> y2 = y * y;
    return sqrt(x2 + y2);
}

// 3-argument version
template<unsigned maxlimbs>
inline ereal<maxlimbs> hypot(const ereal<maxlimbs>& x,
                              const ereal<maxlimbs>& y,
                              const ereal<maxlimbs>& z) {
    ereal<maxlimbs> x2 = x * x;
    ereal<maxlimbs> y2 = y * y;
    ereal<maxlimbs> z2 = z * z;
    return sqrt(x2 + y2 + z2);
}
```

**Key Points**:
- Simple implementation using sqrt
- ereal's expansion arithmetic prevents overflow naturally
- No need for complex scaling (unlike fixed-precision types)

**Test Cases**:
- hypot(3.0, 4.0) = 5.0 (3-4-5 triangle)
- hypot(5.0, 12.0) = 13.0 (5-12-13 triangle)
- hypot(0.0, 0.0) = 0.0
- hypot(large, large) - no overflow
- hypot(x, y, z) - 3D version

---

## Implementation Order

### Step 1: Implement sqrt() (2 hours)
- Follow qd_cascade pattern
- Adaptive iteration count
- Test with known values
- Verify precision

### Step 2: Implement cbrt() (2 hours)
- Range reduction using frexp/ldexp
- Newton-Raphson iteration
- Sign handling
- Test with known values

### Step 3: Implement hypot() (30 minutes)
- Simple wrapper around sqrt
- 2-argument and 3-argument versions
- Test with Pythagorean triples

### Step 4: Regression Tests (2 hours)
- Update sqrt.cpp with comprehensive tests
- Update cbrt.cpp with comprehensive tests
- Update hypot.cpp with comprehensive tests
- Verify precision beyond double

### Step 5: Documentation (1 hour)
- Update CHANGELOG
- Create Phase 3 session log

---

## Testing Strategy

### sqrt() Tests

1. **Exact values**:
   - sqrt(0) = 0
   - sqrt(1) = 1
   - sqrt(4) = 2
   - sqrt(9) = 3
   - sqrt(16) = 4

2. **Known irrational values**:
   - sqrt(2) ≈ 1.414213562373095...
   - sqrt(3) ≈ 1.732050807568877...
   - sqrt(5) ≈ 2.236067977499790...

3. **Precision validation**:
   - Verify: (sqrt(x))² ≈ x (within epsilon)
   - Compare against high-precision reference

4. **Edge cases**:
   - sqrt(very_small) maintains precision
   - sqrt(very_large) no overflow
   - sqrt(negative) error handling

### cbrt() Tests

1. **Exact values**:
   - cbrt(0) = 0
   - cbrt(1) = 1
   - cbrt(8) = 2
   - cbrt(-8) = -2 (sign preserved)
   - cbrt(27) = 3
   - cbrt(-27) = -3

2. **Known irrational values**:
   - cbrt(2) ≈ 1.259921049894873...
   - cbrt(3) ≈ 1.442249570307408...

3. **Precision validation**:
   - Verify: (cbrt(x))³ ≈ x
   - Test frexp/ldexp roundtrip

4. **Range reduction**:
   - Test with various exponents
   - Verify scaling correctness

### hypot() Tests

1. **Pythagorean triples**:
   - hypot(3, 4) = 5
   - hypot(5, 12) = 13
   - hypot(8, 15) = 17

2. **3D version**:
   - hypot(0, 0, 0) = 0
   - hypot(2, 3, 6) = 7

3. **Precision**:
   - Verify: hypot(x, y)² = x² + y²

4. **No overflow**:
   - hypot(large, large) computes correctly

---

## Success Criteria

### Phase 3 Complete When:

1. ✅ sqrt() implemented with Newton-Raphson
2. ✅ cbrt() implemented with range reduction + Newton-Raphson
3. ✅ hypot() implemented using sqrt (2-arg and 3-arg)
4. ✅ All functions compile without errors
5. ✅ Regression tests pass with precision validation
6. ✅ Newton-Raphson convergence verified
7. ✅ frexp/ldexp integration validated (cbrt)
8. ✅ Documentation updated

---

## Risk Assessment

### Medium Risk ⚠️
- **Newton-Raphson convergence**: Need sufficient iterations for maxlimbs
- **cbrt range reduction**: Must handle exponent adjustment correctly
- **Division in iterations**: Expansion_quotient must be accurate

### Mitigation
- Follow proven qd_cascade algorithms
- Adaptive iteration count: `3 + log2(maxlimbs)`
- Extensive testing with known values
- Verify (sqrt(x))² = x and (cbrt(x))³ = x

---

## Deferred Functions

### Transcendentals (Very High Complexity)
- **exp, log, pow**: Require Taylor series, argument reduction
- **sin, cos, tan**: Require CORDIC or series + range reduction
- **asin, acos, atan**: Inverse trig functions
- **sinh, cosh, tanh**: Hyperbolic functions
- **Special functions**: erf, gamma, etc.

These warrant separate phases due to complexity.

---

## Timeline Estimate

**Total: 6-8 hours**

- Planning: 30 minutes ✅ (this document)
- sqrt implementation: 2 hours
- cbrt implementation: 2 hours
- hypot implementation: 30 minutes
- Regression tests: 2 hours
- Documentation: 1 hour

---

## References

1. **qd_cascade sqrt.hpp**: Proven Newton-Raphson implementation
2. **qd_cascade cbrt.hpp**: Range reduction + Newton-Raphson
3. **Numerical Recipes**: Newton-Raphson convergence theory
4. **Shewchuk**: Expansion arithmetic properties

---

## Next Steps

After Phase 3 approval:
1. Implement sqrt() with adaptive iterations
2. Implement cbrt() with frexp/ldexp
3. Implement hypot() using sqrt
4. Update all regression tests
5. Comprehensive verification
6. Documentation

**Then consider**: Transcendentals (exp, log, etc.) in future phases

---

**Plan Status:** READY FOR REVIEW AND IMPLEMENTATION
**Created:** 2025-11-03
**Author:** Claude Code (with Theodore Omtzigt)
