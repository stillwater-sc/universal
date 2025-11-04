# IEEE Remainder Implementation Fix

## Summary

Fixed the IEEE remainder implementation in `fractional.hpp` to use proper round-to-nearest-even semantics and correct exception handling for division by zero.

## Changes Made

### File: `/include/sw/universal/number/ereal/math/functions/fractional.hpp`

#### 1. Fixed `fmod()` - Division by Zero Handling (lines 17-20)

**Before:**
```cpp
if (y.iszero()) {
    // fmod(x, 0) is undefined - return x for now
    // TODO: proper NaN handling when ereal supports it
    return x;
}
```

**After:**
```cpp
if (y.iszero()) {
    // fmod(x, 0) is undefined - raise domain error
    throw ereal_divide_by_zero();
}
```

#### 2. Fixed `remainder()` - IEEE Round-to-Nearest-Even (lines 30-88)

**Before (INCORRECT):**
```cpp
// remainder(x, y) = x - n*y where n = round(x/y)
// Uses round() which ties away from zero (NOT IEEE compliant!)
ereal<maxlimbs> quotient = x / y;
ereal<maxlimbs> n = round(quotient);  // WRONG!
return x - (n * y);
```

**After (CORRECT):**
```cpp
// IEEE round-to-nearest-even implementation:
Real quotient = x / y;

// Get floor and ceiling of quotient
Real floor_q = floor(quotient);
Real ceil_q = ceil(quotient);

// Compute fractional part
Real frac = quotient - floor_q;

// Choose n based on IEEE round-to-nearest-even rule:
Real n;
Real half(0.5);

if (frac < half) {
    n = floor_q;              // Round down
}
else if (frac > half) {
    n = ceil_q;               // Round up
}
else {
    // Tie-breaking: frac == 0.5
    // Round to even integer
    Real two(2.0);
    Real floor_q_div_2 = floor_q / two;
    Real floor_q_div_2_floor = floor(floor_q_div_2);
    Real twice_floor = floor_q_div_2_floor * two;

    if (twice_floor == floor_q) {
        n = floor_q;          // floor_q is even
    }
    else {
        n = ceil_q;           // floor_q is odd, ceil_q is even
    }
}

return x - (n * y);
```

Also changed division by zero handling to throw exception instead of silently returning x.

## Test Results

All test cases pass correctly:

### Round-to-Nearest-Even Tests
- `remainder(5.0, 2.0) = 1.0` ✓ (5/2 = 2.5 → rounds to 2 (even) → 5 - 2×2 = 1)
- `remainder(7.0, 2.0) = -1.0` ✓ (7/2 = 3.5 → rounds to 4 (even) → 7 - 4×2 = -1)
- `remainder(7.0, 3.0) = 1.0` ✓ (7/3 = 2.33... → rounds to 2 → 7 - 2×3 = 1)

### Exception Handling Tests
- `remainder(x, 0.0)` → throws `ereal_divide_by_zero` ✓
- `fmod(x, 0.0)` → throws `ereal_divide_by_zero` ✓

### Other Tests
- Negative values: `remainder(-7.0, 3.0) = -1.0` ✓
- Exact division: `remainder(9.0, 3.0) = 0.0` ✓
- Range verification: results in [-|y|/2, |y|/2] ✓

## IEEE 754 Compliance

The implementation now correctly implements IEEE 754 remainder semantics:

1. **Round-to-nearest-even**: On exact halfway cases (fractional part = 0.5), always rounds to the even integer
2. **Exception handling**: Division by zero raises `ereal_divide_by_zero` exception
3. **Result range**: Remainder is in range [-|y|/2, |y|/2]

## References

- IEEE 754-2008 Section 5.3.1: remainder(x,y) operation
- C11 Standard 7.12.10.2: The remainder functions
