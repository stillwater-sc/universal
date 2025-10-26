# Mathematical Constants for Cascade Types

## Overview

Successfully created high-precision mathematical constants for all three cascade types:
- **dd_cascade_constants.hpp** - 2 components, 106 bits, ~32 decimal digits
- **td_cascade_constants.hpp** - 3 components, 159 bits, ~48 decimal digits
- **qd_cascade_constants.hpp** - 4 components, 212 bits, ~64 decimal digits

## Implementation Date

2025-10-18

## The Problem: td_constants Were Actually dd_constants

The existing `td_constants.hpp` file only had 2 components per constant:

```cpp
// WRONG - only 2 components!
constexpr td td_pi (3.141592653589793116e+00, 1.224646799147353207e-16);
```

These were actually `dd` (double-double) constants, not true triple-double constants. For proper triple-double precision, we need **3 components**.

## The Solution: Use qd as Oracle

Rather than trying to compute the third component ourselves (which would lose precision), we used the existing **qd (quad-double) constants as the oracle**:

1. **qd constants** were already carefully computed by Jack Poulson (Scibuilders) and Theodore Omtzigt (Stillwater Supercomputing)
2. Each qd constant has **4 high-precision components**
3. We extracted components to create cascade constants:
   - **dd_cascade**: Extract `[0, 1]` from qd
   - **td_cascade**: Extract `[0, 1, 2]` from qd
   - **qd_cascade**: Use all `[0, 1, 2, 3]` from qd

This ensures **maximum precision** and **consistency across the cascade family**.

## Constants Provided

Each cascade type now has the following constants:

### Pi Family
- `pi_4`, `pi_2`, `pi`, `3pi_4`, `2pi` - Pi multiples and fractions
- `1_pi`, `2_pi` - Reciprocals of pi

### Euler's Number
- `e` - Euler's constant
- `1_e` - Reciprocal of e

### Logarithms
- `ln2`, `ln10` - Natural logarithms (base e)
- `lge`, `lg10` - Binary logarithms (base 2)
- `log2`, `loge` - Common logarithms (base 10)

### Square Roots
- `sqrt2`, `sqrt3`, `sqrt5` - Square roots
- `1_sqrt2`, `2_sqrtpi` - Reciprocals involving square roots

### Special Constants
- `phi` - Golden ratio
- `1_phi` - Reciprocal of golden ratio

## Why These Constants Matter

### 1. **Precision Preservation**

Computing these constants at runtime would lose precision:

```cpp
// BAD - loses precision!
td_cascade bad_pi = td_cascade(3.14159265358979323846);
// Only 1 component gets meaningful precision

// GOOD - precomputed with full precision
td_cascade good_pi = td_cascade_pi;
// All 3 components have carefully computed values
```

### 2. **Performance**

Precomputed constants are **instant** vs. runtime computation which would require:
- High-precision arithmetic libraries
- Expensive iterative algorithms
- Multiple operations to achieve desired precision

### 3. **Correctness**

The qd oracle constants were computed using rigorous methods and validated. Extracting components ensures we inherit this correctness.

## Example Component Breakdown: Pi

Here's how pi's precision increases through the cascade hierarchy:

```
dd_cascade_pi:
  [0] = 3.1415926535897931      (53 bits)
  [1] = 1.2246467991473532e-16  (53 bits more)
  Total: ~106 bits, ~32 decimal digits

td_cascade_pi:
  [0] = 3.1415926535897931      (53 bits)
  [1] = 1.2246467991473532e-16  (53 bits more)
  [2] = -2.9947698097183397e-33 (53 bits more)
  Total: ~159 bits, ~48 decimal digits

qd_cascade_pi:
  [0] = 3.1415926535897931      (53 bits)
  [1] = 1.2246467991473532e-16  (53 bits more)
  [2] = -2.9947698097183397e-33 (53 bits more)
  [3] = 1.1124542208633657e-49  (53 bits more)
  Total: ~212 bits, ~64 decimal digits
```

Each component adds ~53 bits of precision (one IEEE-754 double's mantissa).

## Validation

The test `cascade_constants_test.cpp` validates:

✅ All constants compile and load correctly
✅ Component values match qd oracle extraction
✅ dd_cascade components match first 2 of td_cascade
✅ td_cascade components match first 3 of qd_cascade
✅ Arithmetic operations with constants work correctly

```bash
g++ -std=c++20 -I include/sw cascade_constants_test.cpp -o cascade_constants_test
./cascade_constants_test
```

Output shows:
```
Consistency Check (Oracle Extraction Validation):
dd_cascade_pi[0:1] matches td_cascade_pi[0:1]: ✓ PASS
td_cascade_pi[0:2] matches qd_cascade_pi[0:2]: ✓ PASS
```

## Files Created

```
include/sw/universal/number/dd_cascade/math/constants/
└── dd_cascade_constants.hpp

include/sw/universal/number/td_cascade/math/constants/
└── td_cascade_constants.hpp

include/sw/universal/number/qd_cascade/math/constants/
└── qd_cascade_constants.hpp
```

Each main cascade header now includes its constants:
- `dd_cascade.hpp` → includes dd_cascade_constants.hpp
- `td_cascade.hpp` → includes td_cascade_constants.hpp
- `qd_cascade.hpp` → includes qd_cascade_constants.hpp

## Usage Example

```cpp
#include <universal/number/qd_cascade/qd_cascade.hpp>

using namespace sw::universal;

int main() {
    // Use precomputed high-precision constants
    auto circumference = qd_cascade(2.0) * qd_cascade_pi;
    auto area = qd_cascade_pi * radius * radius;

    // Much better than:
    auto bad_pi = qd_cascade(3.14159265358979323846);  // Only ~17 digits!

    return 0;
}
```

## Comparison with Classic dd/td/qd Constants

| Type | File | Components | Status |
|------|------|-----------|--------|
| dd | `dd/math/constants/dd_constants.hpp` | 2 | ✓ Correct |
| td | `td/math/constants/td_constants.hpp` | 2 | ✗ **Wrong!** Should be 3 |
| qd | `qd/math/constants/qd_constants.hpp` | 4 | ✓ Correct (Oracle) |
| dd_cascade | `dd_cascade/math/constants/` | 2 | ✓ Correct (from qd) |
| td_cascade | `td_cascade/math/constants/` | 3 | ✓ **Fixed!** |
| qd_cascade | `qd_cascade/math/constants/` | 4 | ✓ Correct (from qd) |

## Key Insight: Oracle-Based Constant Generation

**Question**: Can we compute the third component ourselves, or do we need a higher-precision oracle?

**Answer**: We must use a higher-precision oracle.

Why?
1. Computing constants with insufficient precision loses information
2. The qd constants were computed using specialized high-precision methods
3. Extracting components from qd → td → dd preserves maximum precision
4. This ensures consistency: all cascade types share the same fundamental values

This is analogous to the **Priest algorithms** underlying floatcascade<>: we need exact error-free transformations. Similarly, we need exact high-precision constants, which only an oracle can provide.

## Credits

Mathematical constants computed by:
- **Scibuilders** (Jack Poulson)
- **Stillwater Supercomputing** (Theodore Omtzigt)

Cascade constant extraction and implementation:
- **2025-10-18** - Created dd_cascade, td_cascade, qd_cascade constants by extracting from qd oracle

## Future Work

### Missing Constants from qd

Some qd constants only have 2 components (incomplete):
```cpp
constexpr qd qd_pi_3 (1.570796326794896558, 6.123233995736766036e-17); // Only 2!
```

These need to be computed with full 4-component precision to complete the cascade constant families.

### Additional Constants to Add

Consider adding:
- Catalan's constant
- Apéry's constant (ζ(3))
- Euler-Mascheroni constant (γ)
- More transcendental constants as needed

## References

- Original qd constants: `include/universal/number/qd/math/constants/qd_constants.hpp`
- Original dd constants: `include/universal/number/dd/math/constants/dd_constants.hpp`
- Original td constants: `include/universal/number/td/math/constants/td_constants.hpp` (incomplete!)
- Priest's algorithms: `include/universal/internal/error_free_ops/`
- floatcascade framework: `include/universal/internal/floatcascade/`

## Conclusion

The cascade mathematical constants are now complete and correct:
- ✅ **dd_cascade**: 2 components extracted from qd oracle
- ✅ **td_cascade**: 3 components extracted from qd oracle (**fixed from 2!**)
- ✅ **qd_cascade**: 4 components from qd oracle

This provides a solid foundation for high-precision numerical work with guaranteed precision and consistency across the cascade family. The oracle-based approach ensures we don't lose precision through recomputation.
