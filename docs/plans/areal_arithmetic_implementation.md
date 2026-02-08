# Plan: Complete Areal Arithmetic Implementation

## Overview

Implement the stubbed arithmetic operators for the `areal` (faithful floating-point) type. The areal type has an uncertainty bit (ubit) that tracks whether values are exact or represent intervals.

## Key Concepts

**Areal Encoding**: `[sign | exponent (es bits) | fraction (fbits) | ubit (1 bit)]`
- `ubit=0`: Value is exact
- `ubit=1`: True value lies in open interval (v, next(v))

**Ubit Propagation Rule**: `result.ubit = lhs.ubit || rhs.ubit || rounding_occurred`

## Architecture Decision

Use `blocktriple` intermediate representation (same as cfloat) for:
- Proven arithmetic infrastructure
- Consistency with library patterns
- Reusable add/mul/div operations
- Clean separation of rounding logic

## Files to Modify

### Primary: `include/sw/universal/number/areal/areal_impl.hpp`

Add these methods/functions:

1. **Selector method** (~line 832):
```cpp
inline constexpr bool ubit() const noexcept {
    return (_block[0] & LSB_BIT_MASK) != 0;
}
```

2. **Normalization methods** (add after line 1053):
```cpp
template<size_t tgtbits>
constexpr void normalizeAddition(blocktriple<tgtbits, BlockTripleOperator::ADD, bt>& tgt) const;

template<size_t tgtbits>
constexpr void normalizeMultiplication(blocktriple<tgtbits, BlockTripleOperator::MUL, bt>& tgt) const;

template<size_t tgtbits>
constexpr void normalizeDivision(blocktriple<tgtbits, BlockTripleOperator::DIV, bt>& tgt) const;
```

3. **Convert function** (free function after class):
```cpp
template<size_t srcbits, BlockTripleOperator op, size_t nbits, size_t es, typename bt>
void convert(const blocktriple<srcbits, op, bt>& src, areal<nbits, es, bt>& tgt, bool inputUncertain);
```

4. **Implement arithmetic operators** (lines 561-587):
   - `operator+=` - addition with ubit propagation
   - `operator-=` - subtraction (delegates to negation + addition)
   - `operator*=` - multiplication with ubit propagation
   - `operator/=` - division with ubit propagation

### Secondary: `include/sw/universal/number/areal/areal.hpp`

Add include for blocktriple:
```cpp
#include <universal/internal/blocktriple/blocktriple.hpp>
```

## Algorithm: Addition (operator+=)

```cpp
areal& operator+=(const areal& rhs) {
    // 1. Special cases
    if (isnan() || rhs.isnan()) { setnan(); return *this; }
    if (isinf()) {
        if (rhs.isinf() && sign() != rhs.sign()) setnan();
        return *this;
    }
    if (rhs.isinf()) { *this = rhs; return *this; }
    if (iszero()) { *this = rhs; return *this; }
    if (rhs.iszero()) return *this;

    // 2. Record input uncertainty
    bool inputUncertain = ubit() || rhs.ubit();

    // 3. Normalize to blocktriple
    blocktriple<abits, BlockTripleOperator::ADD, bt> a, b, sum;
    normalizeAddition(a);
    rhs.normalizeAddition(b);

    // 4. Perform addition
    sum.add(a, b);

    // 5. Convert back with ubit propagation
    convert(sum, *this, inputUncertain);

    return *this;
}
```

## Algorithm: Multiplication (operator*=)

```cpp
areal& operator*=(const areal& rhs) {
    // 1. Special cases
    if (isnan() || rhs.isnan()) { setnan(); return *this; }
    bool resultSign = sign() != rhs.sign();

    if (isinf()) {
        if (rhs.iszero()) setnan();
        else setsign(resultSign);
        return *this;
    }
    if (rhs.isinf()) {
        if (iszero()) setnan();
        else { setinf(resultSign); }
        return *this;
    }
    if (iszero() || rhs.iszero()) {
        setzero(); setsign(resultSign);
        return *this;
    }

    // 2. Record input uncertainty
    bool inputUncertain = ubit() || rhs.ubit();

    // 3. Normalize and multiply
    blocktriple<mbits, BlockTripleOperator::MUL, bt> a, b, product;
    normalizeMultiplication(a);
    rhs.normalizeMultiplication(b);
    product.mul(a, b);

    // 4. Convert back
    convert(product, *this, inputUncertain);

    return *this;
}
```

## Algorithm: Division (operator/=)

Similar pattern with division-specific special cases:
- `0/0` → NaN
- `x/0` → ±inf
- `inf/inf` → NaN
- `x/inf` → 0 with ubit=1

## Ubit Propagation in convert()

```cpp
template<...>
void convert(const blocktriple<...>& src, areal<...>& tgt, bool inputUncertain) {
    // ... standard conversion logic ...

    // Check if rounding occurred (sticky bits)
    bool roundingOccurred = false;
    if (rightShift > 0) {
        uint64_t stickyMask = (1ull << rightShift) - 1;
        roundingOccurred = (src.significand_ull() & stickyMask) != 0;
    }

    // Set ubit
    tgt.set(0, inputUncertain || roundingOccurred);
}
```

## Overflow/Underflow Behavior

| Condition | Result | ubit |
|-----------|--------|------|
| Overflow (> maxpos) | maxpos | 1 |
| Underflow (< minpos) | 0 | 1 |

This indicates:
- Overflow: true value in (maxpos, inf)
- Underflow: true value in (0, minpos)

## Special Cases Summary

| Operation | lhs | rhs | Result |
|-----------|-----|-----|--------|
| any | NaN | any | NaN |
| + | +inf | -inf | NaN |
| + | inf | finite | inf |
| * | inf | 0 | NaN |
| * | inf | finite | ±inf |
| / | any | 0 | ±inf or NaN |
| / | inf | inf | NaN |

## Implementation Order

1. Add `ubit()` selector method
2. Add blocktriple include to areal.hpp
3. Implement `normalizeAddition()` (follow cfloat pattern)
4. Implement `convert()` with ubit propagation
5. Implement `operator+=`
6. Test addition
7. Implement `operator-=` (negation + addition)
8. Implement `normalizeMultiplication()`
9. Implement `operator*=`
10. Test multiplication
11. Implement `normalizeDivision()`
12. Implement `operator/=`
13. Test division
14. Full regression testing

## Verification

### Build
```bash
cd build
cmake -DUNIVERSAL_BUILD_NUMBER_AREALS=ON ..
make -j$(nproc)
```

### Run existing tests
```bash
./static/areal/arithmetic/areal_addition
./education/number/areal/edu_areal_basic_operators
```

### Test patterns
1. Special values: NaN, ±inf, ±0
2. Ubit propagation: exact + exact, exact + uncertain, uncertain + uncertain
3. Rounding: operations that require precision loss
4. Overflow/underflow: near maxpos/minpos
5. Exhaustive for areal<8,2>: 256 total values

## Reference Files

- `include/sw/universal/number/cfloat/cfloat_impl.hpp` - Pattern for normalization and convert
- `include/sw/universal/internal/blocktriple/blocktriple.hpp` - Intermediate arithmetic
- `static/areal/arithmetic/addition.cpp` - Existing test framework
