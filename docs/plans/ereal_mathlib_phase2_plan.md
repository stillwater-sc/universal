# ereal Mathlib Phase 2: Medium-Complexity Functions

**Date:** 2025-11-03
**Objective:** Implement medium-complexity math functions requiring expansion operations
**Status:** Planning
**Branch:** v3.89
**Prerequisites:** Phase 1 complete (12 simple functions at full adaptive precision)

---

## Executive Summary

Phase 2 implements medium-complexity functions that require expansion arithmetic operations beyond simple comparisons and component manipulation. These functions use available expansion operations (quotient, reciprocal, product) and introduce exponent manipulation and component summation patterns.

**Target Functions:**
- **truncate** (trunc, round) - component summation
- **numerics** (frexp, ldexp) - exponent manipulation
- **fractional** (fmod, remainder) - expansion quotient

**Explicitly Deferred to Phase 3:**
- **sqrt, hypot** - Newton-Raphson iterations (high complexity)
- **cbrt** - complex algorithm requiring frexp/ldexp (implement after Phase 2 numerics)
- **All transcendentals** - exp, log, pow, trig, hyperbolic

**Estimated Duration:** 6-8 hours

---

## Background

### What Phase 1 Provided

12 functions now use full adaptive precision:
- ✅ minmax (min, max) - comparison operators
- ✅ classify (6 functions) - native methods
- ✅ numerics (copysign) - sign manipulation
- ✅ truncate (floor, ceil) - component-wise operations

**Key Pattern Established**: Never convert to/from double - use native operations

### What Phase 2 Delivers

6 additional functions with full adaptive precision:
- **trunc(), round()** - Complete truncate.hpp
- **frexp(), ldexp()** - Essential for exponent manipulation
- **fmod(), remainder()** - Modulo operations using division

**Precision improvement**: From ~15 digits (Phase 0 stubs) to unlimited (adaptive precision)

---

## Available Infrastructure

### Expansion Operations (from expansion_ops.hpp)

**Already Available**:
- `expansion_product(e, f)` - Multiply two expansions
- `expansion_quotient(e, f)` - Divide two expansions (uses reciprocal)
- `expansion_reciprocal(e)` - Compute 1/e via Newton iteration
- `scale_expansion(e, b)` - Multiply expansion by scalar
- `linear_expansion_sum(e, f)` - Add two expansions
- `compare_adaptive(e, f)` - Compare with early termination

**Not Available** (Phase 3):
- `expansion_sqrt()` - Will need to implement for Phase 3

### ereal Capabilities

- ✅ Arithmetic operators (+, -, *, /) - use expansion operations
- ✅ Component access via `limbs()`
- ✅ Comparison operators
- ✅ Sign manipulation

---

## Phase 2 Function Details

### 1. truncate.hpp - Complete Implementation

**Current Status**:
- ✅ floor(), ceil() implemented in Phase 1
- ❌ trunc(), round() still stubs

#### trunc() - Truncate Toward Zero

**Algorithm**:
```
if x >= 0: trunc(x) = floor(x)
if x < 0:  trunc(x) = ceil(x)
```

**Implementation**:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> trunc(const ereal<maxlimbs>& x) {
    return (x >= ereal<maxlimbs>(0.0)) ? floor(x) : ceil(x);
}
```

**Rationale**:
- Leverages Phase 1 floor/ceil implementations
- Simple sign-based dispatch
- No new algorithms needed

**Test Cases**:
- trunc(2.7) = 2.0
- trunc(-2.7) = -2.0
- trunc(5.0) = 5.0
- trunc(0.0) = 0.0

---

#### round() - Round to Nearest Integer

**Algorithm**:
```
round(x) = floor(x + 0.5)  for x >= 0
round(x) = ceil(x - 0.5)   for x < 0
```

**Implementation**:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> round(const ereal<maxlimbs>& x) {
    if (x >= ereal<maxlimbs>(0.0)) {
        return floor(x + ereal<maxlimbs>(0.5));
    } else {
        return ceil(x - ereal<maxlimbs>(0.5));
    }
}
```

**Rationale**:
- Standard rounding: add 0.5 then floor (for positive)
- Symmetric handling for negative values
- Uses Phase 1 floor/ceil + ereal arithmetic

**Edge Cases**:
- round(2.5) = 3.0 (round away from zero for .5)
- round(-2.5) = -3.0
- round(2.4) = 2.0
- round(2.6) = 3.0

**Alternative**: Could use round-to-even (banker's rounding) if needed later

**Test Cases**:
- round(2.3) = 2.0
- round(2.5) = 3.0
- round(2.7) = 3.0
- round(-2.3) = -2.0
- round(-2.5) = -3.0
- round(-2.7) = -3.0

---

### 2. numerics.hpp - Exponent Manipulation

**Current Status**:
- ✅ copysign() implemented in Phase 1
- ❌ frexp(), ldexp() still stubs

#### frexp() - Extract Mantissa and Exponent

**Purpose**: Decompose x = mantissa × 2^exponent where 0.5 ≤ |mantissa| < 1.0

**Algorithm**:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> frexp(const ereal<maxlimbs>& x, int* exp) {
    if (x.iszero()) {
        *exp = 0;
        return x;
    }

    // Use high component to determine exponent
    const auto& limbs = x.limbs();
    double high = limbs[0];

    // Get exponent of high component
    double mantissa_high = std::frexp(high, exp);

    // Scale entire expansion by 2^(-exponent)
    // This is equivalent to ldexp(x, -(*exp))
    ereal<maxlimbs> result = x;
    for (auto& limb : result.limbs()) {
        limb = std::ldexp(limb, -(*exp));
    }

    return result;
}
```

**Rationale**:
- High component determines overall scale
- Scale all components by same power of 2
- Preserves relative magnitudes of components
- Result is normalized expansion with 0.5 ≤ |high| < 1.0

**Challenge**: Need mutable access to limbs - may need helper method

**Alternative** (if can't mutate limbs directly):
```cpp
// Reconstruct by scaling components
ereal<maxlimbs> result;
result = std::ldexp(limbs[0], -(*exp));
for (size_t i = 1; i < limbs.size(); ++i) {
    result += ereal<maxlimbs>(std::ldexp(limbs[i], -(*exp)));
}
return result;
```

**Test Cases**:
- frexp(8.0, &e) = 0.5, e = 4  (8.0 = 0.5 × 2^4)
- frexp(1.5, &e) = 0.75, e = 1  (1.5 = 0.75 × 2^1)
- frexp(0.5, &e) = 0.5, e = 0  (0.5 = 0.5 × 2^0)
- frexp(0.0, &e) = 0.0, e = 0

---

#### ldexp() - Multiply by Power of 2

**Purpose**: Compute x × 2^exp (efficient power-of-2 scaling)

**Algorithm**:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> ldexp(const ereal<maxlimbs>& x, int exp) {
    if (x.iszero() || exp == 0) return x;

    // Scale all components by 2^exp
    const auto& limbs = x.limbs();
    ereal<maxlimbs> result;

    result = std::ldexp(limbs[0], exp);
    for (size_t i = 1; i < limbs.size(); ++i) {
        result += ereal<maxlimbs>(std::ldexp(limbs[i], exp));
    }

    return result;
}
```

**Rationale**:
- Multiply by power of 2 doesn't introduce rounding error (for reasonable exponents)
- Scale each component independently
- Preserve expansion structure
- Very efficient (no actual multiplication, just exponent adjustment)

**Benefits**:
- Essential for cbrt algorithm (Phase 3)
- Efficient scaling in numerical algorithms
- No precision loss for power-of-2 operations

**Test Cases**:
- ldexp(1.0, 3) = 8.0  (1.0 × 2^3)
- ldexp(1.5, 2) = 6.0  (1.5 × 2^2)
- ldexp(1.0, -2) = 0.25  (1.0 × 2^-2)
- ldexp(0.0, 5) = 0.0

---

### 3. fractional.hpp - Modulo Operations

**Current Status**: Both fmod() and remainder() are stubs

#### fmod() - Floating-Point Remainder

**Purpose**: Compute fmod(x, y) = x - n*y where n = trunc(x/y)

**Algorithm**:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> fmod(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
    if (y.iszero()) {
        // fmod(x, 0) is undefined, return NaN or handle error
        return x;  // TODO: proper NaN handling
    }

    // n = trunc(x / y)
    ereal<maxlimbs> quotient = x / y;  // Uses expansion_quotient
    ereal<maxlimbs> n = trunc(quotient);  // Uses Phase 2 trunc

    // result = x - n * y
    return x - (n * y);
}
```

**Rationale**:
- Uses expansion_quotient (available)
- Uses trunc() from Phase 2
- Standard fmod definition
- Preserves full precision

**Properties**:
- Result has same sign as x
- |result| < |y|
- fmod(x, y) = x - n*y where n = trunc(x/y)

**Test Cases**:
- fmod(5.3, 2.0) = 1.3  (5.3 - 2*2.0)
- fmod(-5.3, 2.0) = -1.3  (-5.3 - (-2)*2.0)
- fmod(5.3, -2.0) = 1.3
- fmod(-5.3, -2.0) = -1.3

---

#### remainder() - IEEE Remainder

**Purpose**: Compute remainder(x, y) = x - n*y where n = round(x/y)

**Algorithm**:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> remainder(const ereal<maxlimbs>& x, const ereal<maxlimbs>& y) {
    if (y.iszero()) {
        // remainder(x, 0) is undefined
        return x;  // TODO: proper NaN handling
    }

    // n = round(x / y)  - uses round-to-nearest-even
    ereal<maxlimbs> quotient = x / y;
    ereal<maxlimbs> n = round(quotient);  // Uses Phase 2 round

    // result = x - n * y
    return x - (n * y);
}
```

**Rationale**:
- Uses expansion_quotient (available)
- Uses round() from Phase 2
- Standard IEEE 754 remainder definition
- Symmetric around zero

**Properties**:
- |result| ≤ |y|/2
- Result chooses closest n (round to nearest)
- Can be positive or negative regardless of signs

**Difference from fmod**:
- fmod uses trunc (round toward zero)
- remainder uses round (round to nearest)
- remainder is symmetric: remainder(x, y) ∈ [-|y|/2, |y|/2]

**Test Cases**:
- remainder(5.3, 2.0) = 1.3  (n = 3)
- remainder(6.0, 2.0) = 0.0  (n = 3, exact division)
- remainder(7.0, 2.0) = -1.0  (n = 4, round to nearest even)

---

## Implementation Order

### Step 1: Complete truncate.hpp (1 hour)
- Implement trunc() using floor/ceil
- Implement round() using floor/ceil + arithmetic
- Update regression test truncate.cpp
- Verify compilation and testing

### Step 2: Implement numerics.hpp (2 hours)
- Implement ldexp() first (simpler, no output parameter)
- Implement frexp() (needs careful handling of exponent output)
- Update regression test numerics.cpp
- Verify compilation and testing

### Step 3: Implement fractional.hpp (2 hours)
- Implement fmod() using trunc
- Implement remainder() using round
- Update regression test fractional.cpp
- Verify compilation and testing

### Step 4: Integration Testing (1 hour)
- Comprehensive tests across all Phase 2 functions
- Edge case testing
- Precision validation

### Step 5: Documentation (1 hour)
- Update CHANGELOG
- Create Phase 2 session log
- Update plans

---

## Testing Strategy

### Function-Specific Tests

**trunc()**:
- Positive values: trunc(2.7) = 2.0
- Negative values: trunc(-2.7) = -2.0
- Integers: trunc(5.0) = 5.0
- Zero: trunc(0.0) = 0.0
- Near integers: trunc(2.999999) = 2.0

**round()**:
- Below midpoint: round(2.3) = 2.0
- At midpoint: round(2.5) = 3.0
- Above midpoint: round(2.7) = 3.0
- Negative below: round(-2.3) = -2.0
- Negative at midpoint: round(-2.5) = -3.0
- Negative above: round(-2.7) = -3.0

**frexp()**:
- Powers of 2: frexp(8.0) = (0.5, 4)
- Non-powers: frexp(6.0) = (0.75, 3)
- Fractions: frexp(0.25) = (0.5, -1)
- Zero: frexp(0.0) = (0.0, 0)

**ldexp()**:
- Positive exponent: ldexp(1.0, 3) = 8.0
- Negative exponent: ldexp(1.0, -2) = 0.25
- Zero exponent: ldexp(2.5, 0) = 2.5
- Zero value: ldexp(0.0, 10) = 0.0

**fmod()**:
- Basic: fmod(5.3, 2.0) = 1.3
- Negative dividend: fmod(-5.3, 2.0) = -1.3
- Negative divisor: fmod(5.3, -2.0) = 1.3
- Both negative: fmod(-5.3, -2.0) = -1.3

**remainder()**:
- Basic: remainder(5.3, 2.0) = 1.3
- Symmetric: remainder(-5.3, 2.0) = -1.3
- Exact division: remainder(6.0, 2.0) = 0.0
- Midpoint: remainder(7.0, 4.0) = -1.0 (round to even)

### Precision Validation

Once ereal supports multi-component construction:
- Test operations preserve full precision
- Verify no double conversion artifacts
- Confirm expansion structure maintained

---

## Success Criteria

### Phase 2 Complete When:

1. ✅ truncate.hpp: trunc(), round() implemented
2. ✅ numerics.hpp: frexp(), ldexp() implemented
3. ✅ fractional.hpp: fmod(), remainder() implemented
4. ✅ All functions compile without errors
5. ✅ All regression tests pass
6. ✅ Tests demonstrate correctness for standard cases
7. ✅ No double conversion (full adaptive precision)
8. ✅ Documentation updated (CHANGELOG, session log)

---

## Deferred to Phase 3

### High-Complexity Functions

**Roots** (Newton-Raphson):
- sqrt() - Essential, highest priority for Phase 3
- cbrt() - Depends on frexp/ldexp (late Phase 2 or early Phase 3)

**Hypot**:
- hypot(x, y) - Depends on sqrt()

**Transcendentals** (Taylor Series):
- exp, exp2, exp10, expm1
- log, log2, log10, log1p
- pow (general exponentiation)

**Trigonometry** (Argument Reduction + CORDIC):
- sin, cos, tan
- asin, acos, atan, atan2

**Hyperbolic**:
- sinh, cosh, tanh
- asinh, acosh, atanh

**Special Functions**:
- erf, erfc, tgamma, lgamma

**Next**:
- nextafter, nexttoward

---

## Risk Assessment

### Low Risk ✅
- trunc, round - Simple dispatches to floor/ceil
- ldexp - Just scaling components

### Medium Risk ⚠️
- frexp - Need to handle exponent output parameter correctly
- fmod, remainder - Depend on division working correctly

### Mitigation
- Test division operations separately first
- Verify frexp/ldexp roundtrip: x = ldexp(frexp(x, &e), e)
- Follow IEEE 754 specifications exactly
- Reference qd_cascade when uncertain

---

## Implementation Notes

### Division in ereal

ereal's operator/ uses expansion_quotient:
```cpp
ereal& operator/=(const ereal& rhs) {
    using namespace expansion_ops;
    _limb = expansion_quotient(_limb, rhs._limb);
    return *this;
}
```

expansion_quotient uses expansion_reciprocal (Newton iteration), so division is already high-precision.

### Exponent Manipulation

Key insight: ldexp and frexp operate on IEEE-754 exponent field
- For expansions, apply to each component independently
- Maintains non-overlapping property
- No precision loss for reasonable exponents

### Component Construction Pattern

From Phase 1:
```cpp
ereal<maxlimbs> result;
result = first_value;
for (...) {
    result += ereal<maxlimbs>(next_value);
}
```

Use this pattern for frexp/ldexp result construction.

---

## Timeline Estimate

**Total: 6-8 hours**

- Planning: 30 minutes ✅ (this document)
- truncate.hpp: 1 hour (trunc, round + tests)
- numerics.hpp: 2 hours (frexp, ldexp + tests)
- fractional.hpp: 2 hours (fmod, remainder + tests)
- Integration testing: 1 hour
- Documentation: 1 hour
- Buffer: 0.5-1 hour

---

## References

1. **IEEE 754**: Floating-point standard for fmod, remainder semantics
2. **expansion_ops.hpp**: Available expansion operations
3. **qd_cascade**: Reference implementations
4. **Phase 1 patterns**: floor/ceil component-wise, construction patterns

---

## Next Steps

After Phase 2 approval:
1. Implement trunc() and round()
2. Implement frexp() and ldexp()
3. Implement fmod() and remainder()
4. Update all regression tests
5. Comprehensive verification
6. Documentation

Then proceed to Phase 3: High-complexity functions (sqrt, cbrt, transcendentals)

---

**Plan Status:** READY FOR REVIEW AND IMPLEMENTATION
**Created:** 2025-11-03
**Author:** Claude Code (with Theodore Omtzigt)
