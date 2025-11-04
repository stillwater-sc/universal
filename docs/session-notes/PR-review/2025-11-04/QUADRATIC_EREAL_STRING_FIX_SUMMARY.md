# Quadratic Ereal Output String Fix

## Summary

Fixed an incorrect output string in `applications/precision/numeric/quadratic_ereal.cpp` where the printed message referenced "ereal<128>" but the actual code used `ereal<19>`.

## Problem

**Before the fix (line 219):**
```cpp
std::cout << "--- Adaptive Precision (ereal<128> - Naive Formula) ---\n";

ereal<19> a(test.a), b(test.b), c(test.c);  // ← Actually uses ereal<19>
```

**Issue:**
- The output message claimed to be using `ereal<128>`
- The actual code used `ereal<19>`
- This created confusion for anyone reading the output
- `ereal<128>` would be invalid anyway (max is `ereal<19>`)

## Solution

**After the fix (line 219):**
```cpp
std::cout << "--- Adaptive Precision (ereal<19> - Naive Formula) ---\n";

ereal<19> a(test.a), b(test.b), c(test.c);  // ← Now message matches code
```

**Changes:**
- Changed `ereal<128>` to `ereal<19>` in the output string
- The message now accurately reflects the actual type being used

## Impact

### Before Fix:
- ❌ Misleading output suggesting 128-limb precision
- ❌ Inconsistent with actual code behavior
- ❌ Confusing for users trying to understand the example

### After Fix:
- ✅ Output message matches actual code
- ✅ Correctly indicates 19-limb precision (maximum for ereal)
- ✅ Clear and accurate for users

## Context

### Why `ereal<19>`?
The maximum supported value for `ereal` is 19 limbs:
- Each limb provides ~53 bits (~15.95 decimal digits)
- `ereal<19>` provides ~304 decimal digits of precision
- This is the maximum to maintain Shewchuk's expansion arithmetic correctness
- Values > 19 would cause the last limb to underflow below DBL_MIN

### What This Test Demonstrates:
The test shows that the naive quadratic formula:
```
x = (-b ± √(b² - 4ac)) / (2a)
```
works correctly with adaptive precision (`ereal<19>`), avoiding the catastrophic cancellation that occurs with fixed double precision.

## File Modified

**File**: `applications/precision/numeric/quadratic_ereal.cpp`
**Line**: 219
**Section**: Test 3 - Adaptive precision using naive formula

## Related Code

The test compares three approaches:
1. **Double (naive)** - Shows catastrophic cancellation
2. **Double (stable)** - Uses numerically stable formula
3. **ereal<19> (naive)** - Shows naive formula works with high precision

Example output (now correct):
```
--- Adaptive Precision (ereal<19> - Naive Formula) ---
  x₁ = ...
  x₂ = ...
  x₁ components: 19, x₂ components: 19
```

## Verification

The file compiles cleanly without warnings:
```bash
g++ -std=c++20 -I./include/sw -c ./applications/precision/numeric/quadratic_ereal.cpp -Wall
# Success: no errors or warnings
```

## Conclusion

This was a simple documentation fix ensuring the output message accurately reflects the code. The change eliminates potential confusion and correctly indicates that the test uses `ereal<19>`, which is the maximum precision available for the ereal type.
