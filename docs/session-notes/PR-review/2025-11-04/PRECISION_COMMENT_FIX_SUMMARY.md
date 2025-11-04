# Ereal Precision Comment Fix

## Summary

Fixed inconsistent precision comments in two ereal math regression test files where the bit count (1216) and decimal digit count (≈303) were calculated using different conventions, causing confusion.

## Problem

**Files affected:**
1. `elastic/ereal/math/functions/hyperbolic.cpp` line 402
2. `elastic/ereal/math/functions/trigonometry.cpp` line 459

**Before the fix:**
```cpp
// Extreme precision tests at 1216 bits (≈303 decimal digits, maximum algorithmically valid)
```

**Issue:**
The comment mixed two different precision calculation methods:
- **1216 bits** = 19 limbs × 64 bits/limb (total bits including exponent)
- **≈303 digits** = 19 limbs × 53 bits/limb × log₁₀(2) (mantissa precision only)

This created an inconsistency because:
- 1216 bits × log₁₀(2) ≈ **366 decimal digits** (correct conversion)
- 19 × 53 bits × log₁₀(2) ≈ 1007 bits ≈ **303 decimal digits** (different calculation)

## Two Valid Precision Conventions

The codebase uses two different conventions for describing ereal precision:

### Convention 1: Mantissa Precision (53 bits/limb)
Used in: `ereal_numerics.md`, `progressive_precision.cpp`, `logarithm.cpp`

- `ereal<8>`: 8 × 53 = 424 bits ≈ 127 decimal digits
- `ereal<16>`: 16 × 53 = 848 bits ≈ 255 decimal digits
- `ereal<19>`: 19 × 53 = 1007 bits ≈ 303 decimal digits ✓

### Convention 2: Total Bits (64 bits/limb)
Used in: `hyperbolic.cpp`, `trigonometry.cpp`, `geometry/predicates.cpp`

- `ereal<8>`: 8 × 64 = 512 bits ≈ 154 decimal digits
- `ereal<16>`: 16 × 64 = 1024 bits ≈ 308 decimal digits
- `ereal<19>`: 19 × 64 = 1216 bits ≈ 366 decimal digits ✓

**Both conventions are valid**, but they must be used consistently within each file.

## The Bug

Within `hyperbolic.cpp` and `trigonometry.cpp`, the earlier comments used **Convention 2 (total bits)**:

- Line 369/423: "512 bits (≈154 decimal digits)" for `ereal<8>` ✓ consistent
- Line 390/447: "1024 bits (≈308 decimal digits)" for `ereal<16>` ✓ consistent
- Line 402/459: "1216 bits (≈303 decimal digits)" for `ereal<19>` ✗ **inconsistent!**

The last line incorrectly mixed:
- 1216 bits from Convention 2 (total bits)
- 303 digits from Convention 1 (mantissa precision)

## Solution

**After the fix:**
```cpp
// Extreme precision tests at 1216 bits (≈366 decimal digits, maximum algorithmically valid)
```

**Changes:**
- Changed "≈303 decimal digits" to "≈366 decimal digits" in both files
- Now consistent with Convention 2 (total bits) used throughout each file
- Matches the pattern in `geometry/predicates.cpp` line 292

**Calculation verification:**
- 1216 bits × log₁₀(2) = 1216 × 0.30103 ≈ 366.05 digits ✓

## Reference: Correct Usage in Other Files

### geometry/predicates.cpp (line 292) - Correct:
```cpp
// Test at maximum precision (1216 bits ≈ 366 digits, maxlimbs=19)
```
Uses total bits convention consistently ✓

### ereal_numerics.md (line 46) - Also Correct:
```
**`ereal<19>`** - 303 decimal digits
```
Uses mantissa precision convention (doesn't mention bits) ✓

### logarithm.cpp (line 313) - Also Correct:
```cpp
// Maximum precision tests at ereal<19> (≈303 decimal digits, maximum algorithmically valid)
```
Uses mantissa precision convention (doesn't mention bits) ✓

## Impact

### Before Fix:
- ❌ Inconsistent precision descriptions within files
- ❌ Mixed two different calculation methods
- ❌ Confusing for users trying to understand ereal precision
- ❌ Contradicted correct usage in geometry/predicates.cpp

### After Fix:
- ✅ Consistent use of total bits convention within each file
- ✅ All three precision levels follow same calculation pattern
- ✅ Matches reference implementation in geometry/predicates.cpp
- ✅ Clear and accurate for users

## Files Modified

1. **File**: `elastic/ereal/math/functions/hyperbolic.cpp`
   **Line**: 402
   **Change**: "≈303 decimal digits" → "≈366 decimal digits"

2. **File**: `elastic/ereal/math/functions/trigonometry.cpp`
   **Line**: 459
   **Change**: "≈303 decimal digits" → "≈366 decimal digits"

## Precision Calculation Details

### Total Bits Method (64 bits per limb):
Each double occupies 64 bits total (1 sign + 11 exponent + 52 fraction):
- `ereal<8>`: 8 × 64 = 512 bits → 512 × log₁₀(2) ≈ 154.15 digits
- `ereal<16>`: 16 × 64 = 1024 bits → 1024 × log₁₀(2) ≈ 308.30 digits
- `ereal<19>`: 19 × 64 = 1216 bits → 1216 × log₁₀(2) ≈ 366.05 digits

### Mantissa Precision Method (53 bits per limb):
Only the mantissa contributes to precision (53 bits including implicit bit):
- `ereal<8>`: 8 × 53 = 424 bits → 424 × log₁₀(2) ≈ 127.64 digits
- `ereal<16>`: 16 × 53 = 848 bits → 848 × log₁₀(2) ≈ 255.27 digits
- `ereal<19>`: 19 × 53 = 1007 bits → 1007 × log₁₀(2) ≈ 303.14 digits

Both methods are valid, but must be used consistently.

## Verification

Both files compile cleanly without warnings:
```bash
g++ -std=c++20 -I./include/sw -c ./elastic/ereal/math/functions/hyperbolic.cpp -Wall
g++ -std=c++20 -I./include/sw -c ./elastic/ereal/math/functions/trigonometry.cpp -Wall
# Success: no errors or warnings
```

## Context: Why Two Conventions Exist

### Total Bits (64 bits/limb):
- Describes the memory footprint
- Useful for comparing against other multi-precision formats
- Natural for describing the overall size

### Mantissa Precision (53 bits/limb):
- Describes the actual numerical precision
- More accurate for numerical analysis
- Matches the IEEE-754 mantissa size (52 fraction + 1 implicit)

**Note**: Shewchuk's expansion arithmetic uses the full double values, so both perspectives are meaningful. The key is **consistency within each file**.

## Conclusion

This fix ensures that precision descriptions are internally consistent within each file. Files using the total bits convention (1216 bits) now correctly show the corresponding decimal digits (≈366), matching the pattern established in `geometry/predicates.cpp` and maintaining consistency with earlier comments in the same files.
