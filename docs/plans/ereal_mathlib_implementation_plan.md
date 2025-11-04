# ereal Mathlib Implementation Plan

**Date:** 2025-11-03
**Objective:** Implement mathlib infrastructure for ereal (Shewchuk adaptive precision number system)
**Status:** Planning Phase
**Branch:** v3.89

---

## Executive Summary

This plan outlines the implementation of a complete mathematical function library (mathlib) for the `ereal` number system. The design follows the established Universal library architecture, using `qd_cascade` as the reference template. The implementation will be done in phases, starting with simple stub functions that delegate to `std::` functions (via double conversion), then progressively refining each function to deliver results at requested precision levels.

---

## Background

### ereal Number System
- **Type:** Adaptive-precision multi-component floating-point arithmetic
- **Implementation:** Based on Shewchuk's expansion arithmetic algorithms
- **Key Characteristic:** Precision grows dynamically as needed (elastic)
- **Template Parameter:** `maxlimbs` (default 1024) - maximum number of components
- **Storage:** Vector of double-precision limbs
- **Location:** `include/sw/universal/number/ereal/`

### Universal Precision Architecture

**Fixed-Precision Types** (e.g., qd_cascade, cfloat, posit):
- Fixed size at compile time
- All arithmetic operates at maximum precision for that size
- Results rounded back to fixed size
- No implicit conversions (all conversions explicit)

**Elastic-Precision Types** (e.g., ereal, efloat, einteger):
- Adaptive size at runtime
- Precision grows/shrinks during computation
- **Future capability:** Request specific precision for operations
- No implicit conversions (all conversions explicit)

### Reference Architecture: qd_cascade

The `qd_cascade` number system provides the quintessential structure:
```
include/sw/universal/number/qd_cascade/
├── qd_cascade.hpp                    # Main header (includes mathlib.hpp)
├── qd_cascade_impl.hpp               # Implementation
├── qd_cascade_fwd.hpp                # Forward declarations
├── attributes.hpp                     # Attributes and classification
├── exceptions.hpp                     # Exception types
├── manipulators.hpp                   # I/O manipulators
├── numeric_limits.hpp                 # std::numeric_limits specialization
├── mathlib.hpp                        # ← Math library root (includes all math)
└── math/
    ├── constants/
    │   └── qd_cascade_constants.hpp   # Mathematical constants
    └── functions/
        ├── cbrt.hpp                   # Cube root
        ├── classify.hpp               # Classification functions
        ├── error_and_gamma.hpp        # erf, erfc, tgamma, lgamma
        ├── exponent.hpp               # exp, exp2, exp10, expm1
        ├── fractional.hpp             # fmod, remainder, modf
        ├── hyperbolic.hpp             # sinh, cosh, tanh, asinh, acosh, atanh
        ├── hypot.hpp                  # hypot, hypot3
        ├── logarithm.hpp              # log, log2, log10, log1p
        ├── minmax.hpp                 # min, max, fmin, fmax
        ├── next.hpp                   # nextafter, nexttoward
        ├── numerics.hpp               # frexp, ldexp, copysign, etc.
        ├── pow.hpp                    # pow, pown
        ├── sqrt.hpp                   # sqrt
        ├── trigonometry.hpp           # sin, cos, tan, asin, acos, atan, atan2
        └── truncate.hpp               # floor, ceil, trunc, round
```

---

## Current State Analysis

### qd_cascade Structure (Reference)
- ✅ Complete mathlib infrastructure
- ✅ mathlib.hpp includes all function headers
- ✅ Organized into constants/ and functions/ subdirectories
- ✅ All standard math functions categorized by type
- ✅ Well-documented with comments
- ✅ Phase-based implementation (Phase 1-3 complete)

**Function Categories in qd_cascade:**
1. **Numerics:** frexp, ldexp, copysign, signbit
2. **Classification:** fpclassify, isnan, isinf, isfinite, isnormal
3. **Truncation:** floor, ceil, trunc, round
4. **Min/Max:** min, max, fmin, fmax
5. **Fractional:** fmod, remainder
6. **Hypot:** hypot
7. **Error/Gamma:** erf, erfc, tgamma, lgamma
8. **Square/Cube Roots:** sqrt, cbrt
9. **Exponential:** exp, exp2, exp10, expm1
10. **Logarithmic:** log, log2, log10, log1p
11. **Power:** pow, pown
12. **Hyperbolic:** sinh, cosh, tanh, asinh, acosh, atanh
13. **Trigonometric:** sin, cos, tan, asin, acos, atan, atan2

### ereal Current State
- ✅ Basic implementation complete (ereal_impl.hpp)
- ✅ Arithmetic operators implemented
- ✅ Conversion operators implemented
- ✅ Attributes.hpp exists (basic functionality)
- ✅ Manipulators.hpp exists
- ❌ **NO mathlib infrastructure**
- ❌ **NO math/ directory**
- ❌ **NO mathlib.hpp** (commented out in ereal.hpp line 53)
- ❌ **NO constants defined**
- ❌ **NO math functions implemented**

**Current ereal structure:**
```
include/sw/universal/number/ereal/
├── ereal.hpp                          # Main header (mathlib.hpp commented out)
├── ereal_impl.hpp                     # Implementation
├── ereal_fwd.hpp                      # Forward declarations
├── attributes.hpp                     # Basic attributes
├── exceptions.hpp                     # Exception types
├── manipulators.hpp                   # I/O manipulators
└── numeric_limits.hpp                 # (Commented out in ereal.hpp)

# Missing:
# ├── mathlib.hpp                      # ← TO BE CREATED
# └── math/                            # ← TO BE CREATED
#     ├── constants/                   # ← TO BE CREATED
#     │   └── ereal_constants.hpp      # ← TO BE CREATED
#     └── functions/                   # ← TO BE CREATED
#         └── [all function files]     # ← TO BE CREATED
```

---

## Implementation Strategy

### Phase 0: Infrastructure Setup (This Plan)
**Goal:** Create directory structure and stub files
**Duration:** ~2-3 hours
**Outcome:** Functional (but low-precision) mathlib that compiles and passes basic tests

### Phase 1: Low-Complexity Functions (Future)
**Goal:** Refine simple functions to full precision
**Functions:** truncate, minmax, fractional, hypot, error_and_gamma
**Duration:** ~1-2 weeks

### Phase 2: Medium-Complexity Functions (Future)
**Goal:** Refine transcendental functions
**Functions:** cbrt, hyperbolic, exponent, logarithm, pow
**Duration:** ~2-3 weeks

### Phase 3: High-Complexity Functions (Future)
**Goal:** Refine advanced functions
**Functions:** sqrt, trigonometry
**Duration:** ~2-3 weeks

### Phase 4: Precision Control (Future)
**Goal:** Add precision specification API
**Features:** Request specific precision for operations
**Duration:** ~1-2 weeks

---

## Phase 0 Implementation Plan: Infrastructure Setup

### Objectives
1. Create complete directory structure matching qd_cascade
2. Create mathlib.hpp that includes all function headers
3. Create all function header files with stub implementations
4. Create constants header with basic constants (double precision initially)
5. Uncomment mathlib.hpp include in ereal.hpp
6. Ensure everything compiles
7. Create basic test to verify functionality

### Stub Implementation Strategy

All stub functions will:
1. Convert ereal to double: `double(x)`
2. Call std:: function: `std::function(double(x))`
3. Convert back to ereal: `ereal(result)`

**Example stub pattern:**
```cpp
inline ereal sqrt(const ereal& x) {
    return ereal(std::sqrt(double(x)));
}
```

**Rationale:**
- Provides immediate functionality (low precision but correct)
- Allows testing of infrastructure and API
- Enables progressive refinement without breaking existing code
- Matches Universal's incremental development philosophy

---

## Detailed File Creation List

### Directory Structure
```
include/sw/universal/number/ereal/
├── mathlib.hpp                                    # NEW - Root mathlib header
└── math/                                          # NEW - Directory
    ├── constants/                                 # NEW - Directory
    │   └── ereal_constants.hpp                    # NEW - Mathematical constants
    └── functions/                                 # NEW - Directory
        ├── cbrt.hpp                               # NEW - Cube root
        ├── classify.hpp                           # NEW - Classification (may reuse attributes.hpp)
        ├── error_and_gamma.hpp                    # NEW - erf, erfc, tgamma, lgamma
        ├── exponent.hpp                           # NEW - exp, exp2, exp10, expm1
        ├── fractional.hpp                         # NEW - fmod, remainder, modf
        ├── hyperbolic.hpp                         # NEW - sinh, cosh, tanh, etc.
        ├── hypot.hpp                              # NEW - hypot
        ├── logarithm.hpp                          # NEW - log, log2, log10, log1p
        ├── minmax.hpp                             # NEW - min, max
        ├── next.hpp                               # NEW - nextafter, nexttoward
        ├── numerics.hpp                           # NEW - frexp, ldexp, copysign
        ├── pow.hpp                                # NEW - pow, pown
        ├── sqrt.hpp                               # NEW - sqrt
        ├── trigonometry.hpp                       # NEW - sin, cos, tan, etc.
        └── truncate.hpp                           # NEW - floor, ceil, trunc, round
```

### File Templates

Each file will follow this structure:

**1. mathlib.hpp** (Root header)
```cpp
#pragma once
// mathlib.hpp: definition of mathematical functions for ereal
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#include <universal/number/ereal/math/functions/numerics.hpp>

// Phase 0: Stub implementations (all functions delegate to std:: via double)
#include <universal/number/ereal/math/functions/classify.hpp>
#include <universal/number/ereal/math/functions/error_and_gamma.hpp>
#include <universal/number/ereal/math/functions/fractional.hpp>
#include <universal/number/ereal/math/functions/hypot.hpp>
#include <universal/number/ereal/math/functions/minmax.hpp>
#include <universal/number/ereal/math/functions/truncate.hpp>

#include <universal/number/ereal/math/functions/cbrt.hpp>
#include <universal/number/ereal/math/functions/hyperbolic.hpp>
#include <universal/number/ereal/math/functions/exponent.hpp>
#include <universal/number/ereal/math/functions/logarithm.hpp>
#include <universal/number/ereal/math/functions/pow.hpp>

#include <universal/number/ereal/math/functions/sqrt.hpp>
#include <universal/number/ereal/math/functions/trigonometry.hpp>

namespace sw { namespace universal {

    // pown returns x raised to the integer power n
    // TODO Phase 1: Implement using adaptive-precision algorithm
    template<unsigned maxlimbs>
    inline ereal<maxlimbs> pown(const ereal<maxlimbs>& x, int n) {
        return ereal<maxlimbs>(std::pow(double(x), n));
    }

}} // namespace sw::universal
```

**2. Function header template** (e.g., sqrt.hpp)
```cpp
#pragma once
// sqrt.hpp: sqrt function for ereal
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

namespace sw { namespace universal {

    // Square root function
    // TODO Phase 2: Implement using adaptive-precision Newton-Raphson
    template<unsigned maxlimbs>
    inline ereal<maxlimbs> sqrt(const ereal<maxlimbs>& x) {
        return ereal<maxlimbs>(std::sqrt(double(x)));
    }

}} // namespace sw::universal
```

**3. Constants header** (ereal_constants.hpp)
```cpp
#pragma once
// ereal_constants.hpp: mathematical constants for ereal
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

namespace sw { namespace universal {

    // TODO Phase 1: Implement high-precision constants using expansion arithmetic
    // Currently using double-precision as placeholders

    // Pi and related
    template<unsigned maxlimbs = 1024>
    inline ereal<maxlimbs> ereal_pi() {
        return ereal<maxlimbs>(3.141592653589793238);
    }

    template<unsigned maxlimbs = 1024>
    inline ereal<maxlimbs> ereal_2pi() {
        return ereal<maxlimbs>(6.283185307179586477);
    }

    // Euler's number
    template<unsigned maxlimbs = 1024>
    inline ereal<maxlimbs> ereal_e() {
        return ereal<maxlimbs>(2.718281828459045235);
    }

    // Natural logarithms
    template<unsigned maxlimbs = 1024>
    inline ereal<maxlimbs> ereal_ln2() {
        return ereal<maxlimbs>(0.693147180559945309);
    }

    // ... etc
}} // namespace sw::universal
```

---

## Implementation Steps (Ordered)

### Step 1: Create Directory Structure
```bash
cd include/sw/universal/number/ereal
mkdir -p math/constants
mkdir -p math/functions
```

### Step 2: Create Constants File
- Create `math/constants/ereal_constants.hpp`
- Define basic constants as template functions
- Use double precision as placeholders

### Step 3: Create Function Headers (in order of complexity)

**Group A: Simple functions** (direct std:: delegation)
1. `math/functions/minmax.hpp` - min, max
2. `math/functions/truncate.hpp` - floor, ceil, trunc, round
3. `math/functions/hypot.hpp` - hypot
4. `math/functions/fractional.hpp` - fmod, remainder
5. `math/functions/error_and_gamma.hpp` - erf, erfc, tgamma, lgamma

**Group B: Numeric functions** (may need special handling)
6. `math/functions/numerics.hpp` - frexp, ldexp, copysign, signbit

**Group C: Classification functions** (may reuse attributes.hpp)
7. `math/functions/classify.hpp` - fpclassify, isnan, isinf, etc.

**Group D: Transcendental functions**
8. `math/functions/sqrt.hpp` - sqrt
9. `math/functions/cbrt.hpp` - cbrt
10. `math/functions/exponent.hpp` - exp, exp2, exp10, expm1
11. `math/functions/logarithm.hpp` - log, log2, log10, log1p
12. `math/functions/pow.hpp` - pow, npwr
13. `math/functions/hyperbolic.hpp` - sinh, cosh, tanh, etc.
14. `math/functions/trigonometry.hpp` - sin, cos, tan, etc.

**Group E: Advanced numeric functions**
15. `math/functions/next.hpp` - nextafter, nexttoward

### Step 4: Create mathlib.hpp
- Include all function headers
- Include constants header
- Add namespace wrapper
- Add pown function (matches qd_cascade pattern)

### Step 5: Modify ereal.hpp
- Uncomment line 53: `#include <universal/number/ereal/mathlib.hpp>`
- Verify it's placed after attributes.hpp and manipulators.hpp

### Step 6: Create Test File
- Create `static/ereal/math/ereal_math_stub_test.cpp`
- Test each function category
- Verify compilation and basic functionality

### Step 7: Build and Verify
```bash
cd build
cmake .. -DUNIVERSAL_BUILD_NUMBER_ELASTIC=ON
make ereal_math_stub_test
./static/ereal/math/ereal_math_stub_test
```

---

## Function Inventory

### Complete Function List (from qd_cascade)

**Classification** (from attributes.hpp or classify.hpp):
- `fpclassify(x)` - classify as normal, subnormal, zero, infinite, NaN
- `isnan(x)` - test for NaN
- `isinf(x)` - test for infinity
- `isfinite(x)` - test for finite value
- `isnormal(x)` - test for normal value
- `signbit(x)` - test sign bit

**Numeric Operations** (numerics.hpp):
- `frexp(x, exp)` - break into fraction and exponent
- `ldexp(x, exp)` - multiply by power of 2
- `copysign(x, y)` - copy sign from y to x
- `abs(x)` - absolute value

**Truncation** (truncate.hpp):
- `floor(x)` - round down to integer
- `ceil(x)` - round up to integer
- `trunc(x)` - round toward zero
- `round(x)` - round to nearest integer

**Min/Max** (minmax.hpp):
- `min(x, y)` - minimum of two values
- `max(x, y)` - maximum of two values
- `fmin(x, y)` - minimum (NaN-aware)
- `fmax(x, y)` - maximum (NaN-aware)

**Fractional** (fractional.hpp):
- `fmod(x, y)` - floating-point remainder
- `remainder(x, y)` - IEEE remainder
- `modf(x, *iptr)` - split into integer and fractional parts

**Hypot** (hypot.hpp):
- `hypot(x, y)` - sqrt(x² + y²) without overflow
- `hypot(x, y, z)` - sqrt(x² + y² + z²) without overflow

**Error and Gamma** (error_and_gamma.hpp):
- `erf(x)` - error function
- `erfc(x)` - complementary error function
- `tgamma(x)` - gamma function
- `lgamma(x)` - log gamma function

**Roots** (sqrt.hpp, cbrt.hpp):
- `sqrt(x)` - square root
- `cbrt(x)` - cube root

**Exponential** (exponent.hpp):
- `exp(x)` - e^x
- `exp2(x)` - 2^x
- `exp10(x)` - 10^x
- `expm1(x)` - e^x - 1

**Logarithmic** (logarithm.hpp):
- `log(x)` - natural logarithm
- `log2(x)` - binary logarithm
- `log10(x)` - common logarithm
- `log1p(x)` - log(1 + x)

**Power** (pow.hpp):
- `pow(x, y)` - x^y
- `pown(x, n)` - x^n (integer n)

**Hyperbolic** (hyperbolic.hpp):
- `sinh(x)` - hyperbolic sine
- `cosh(x)` - hyperbolic cosine
- `tanh(x)` - hyperbolic tangent
- `asinh(x)` - inverse hyperbolic sine
- `acosh(x)` - inverse hyperbolic cosine
- `atanh(x)` - inverse hyperbolic tangent

**Trigonometric** (trigonometry.hpp):
- `sin(x)` - sine
- `cos(x)` - cosine
- `tan(x)` - tangent
- `asin(x)` - arcsine
- `acos(x)` - arccosine
- `atan(x)` - arctangent
- `atan2(y, x)` - arctangent of y/x

**Next** (next.hpp):
- `nextafter(x, y)` - next representable value toward y
- `nexttoward(x, y)` - next representable value toward y (long double)

---

## Template Considerations

### ereal is a Template
```cpp
template<unsigned maxlimbs = 1024>
class ereal { ... };
```

All functions must be template functions:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs> sqrt(const ereal<maxlimbs>& x) {
    return ereal<maxlimbs>(std::sqrt(double(x)));
}
```

### Automatic Template Deduction
Most uses will benefit from template argument deduction:
```cpp
ereal<2048> x = 2.0;
ereal<2048> y = sqrt(x);  // Template parameter deduced
```

---

## Testing Strategy

### Phase 0 Tests (Stub Validation)
**Goal:** Verify infrastructure works, not precision

1. **Compilation Test:** Ensure all headers compile
2. **Basic Functionality:** Each function returns reasonable result
3. **Type Safety:** Verify return types are correct
4. **No Regression:** Existing ereal tests still pass

**Test File:** `static/ereal/math/ereal_math_stub_test.cpp`

```cpp
#include <universal/number/ereal/ereal.hpp>
#include <iostream>

int main() {
    using namespace sw::universal;

    ereal<> x = 2.0;
    ereal<> y;

    // Test each category
    y = sqrt(x);         std::cout << "sqrt(2) = " << y << "\n";
    y = sin(x);          std::cout << "sin(2) = " << y << "\n";
    y = exp(x);          std::cout << "exp(2) = " << y << "\n";
    y = log(x);          std::cout << "log(2) = " << y << "\n";
    y = floor(x);        std::cout << "floor(2) = " << y << "\n";
    y = min(x, ereal<>(3.0));  std::cout << "min(2,3) = " << y << "\n";

    std::cout << "All stub functions compiled and executed.\n";
    return 0;
}
```

### Future Tests (Precision Validation)
Will be added as functions are refined in Phases 1-3.

---

## Risk Assessment

### Low Risk
- ✅ Architecture well-established (qd_cascade template)
- ✅ Stub pattern simple and proven
- ✅ No algorithm complexity in Phase 0

### Medium Risk
- ⚠️ Template syntax requires care (ereal is parameterized)
- ⚠️ Large number of files to create (~15 headers)
- ⚠️ Integration with existing ereal implementation

### Mitigation
- Follow qd_cascade structure exactly
- Create and test files incrementally
- Build and test after each file
- Use template parameter consistently

---

## Success Criteria

### Phase 0 Complete When:
1. ✅ All 15+ function header files created
2. ✅ mathlib.hpp created and includes all headers
3. ✅ ereal_constants.hpp created with basic constants
4. ✅ ereal.hpp includes mathlib.hpp (uncommented)
5. ✅ Everything compiles without errors
6. ✅ Basic test demonstrates all functions callable
7. ✅ Results are approximately correct (double precision)
8. ✅ No regressions in existing ereal functionality

---

## Future Enhancements

### Phase 1-3: Progressive Refinement
Each function will be refined to use:
- Shewchuk's expansion arithmetic for exact operations
- Taylor series for transcendentals
- Argument reduction for trigonometry
- Newton-Raphson for roots and reciprocals
- CORDIC algorithms where appropriate

### Phase 4: Precision Control API
Add ability to request specific precision:
```cpp
ereal<> x = 2.0;
ereal<> y = sqrt(x, 200);  // Request 200 bits precision
```

### Phase 5: Performance Optimization
- Inline critical functions
- Optimize common paths
- Add fast paths for low-precision requests
- Consider SIMD for expansion operations

---

## References

### Academic
1. **Shewchuk (1997):** "Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates"
2. **Priest (1991):** "Algorithms for Arbitrary Precision Floating Point Arithmetic"
3. **Dekker (1971):** "A Floating-Point Technique for Extending the Available Precision"

### Implementation
1. **QD Library:** Hida-Li-Bailey quad-double library
2. **MPFR:** Multiple Precision Floating-Point Reliable Library
3. **Universal Numbers:** This codebase's qd_cascade, dd_cascade, td_cascade

### Design Documents
1. `CLAUDE.md` - Universal library architecture
2. `docs/sessions/session_2025-11-02_cascade_cbrt_sqrt_fixes.md` - Recent mathlib work
3. `docs/cascade_math_propagation_plan.md` - Cascade mathlib evolution

---

## Notes

### Why Stubs First?
This approach:
- Provides immediate functionality
- Allows testing of API surface
- Enables client code to be written
- Supports incremental refinement
- Matches Universal's development philosophy
- Reduces risk (small steps)

### Template vs Non-Template
Unlike qd_cascade (which is a concrete type), ereal is a template. All math functions must be template functions to work with arbitrary `maxlimbs` parameters.

### Constants Challenge
qd_cascade uses `constexpr` values with all components specified. ereal constants will need to be:
- Template functions (not constexpr objects)
- Generated on first use
- Cached for performance
- This is a Phase 1 concern

---

## Approval Checklist

Before proceeding to implementation:
- [ ] User reviews directory structure
- [ ] User approves stub strategy
- [ ] User confirms function list completeness
- [ ] User agrees with phased approach
- [ ] User approves testing strategy
- [ ] User confirms template approach

---

## Implementation Timeline

**Phase 0 Estimate:** 2-3 hours
- Directory creation: 5 minutes
- Constants file: 30 minutes
- Function files (15): 90 minutes (6 min each)
- mathlib.hpp: 15 minutes
- ereal.hpp modification: 5 minutes
- Test file creation: 20 minutes
- Build and debug: 30 minutes

**Total:** ~3 hours for complete stub infrastructure

---

## Next Steps

After approval:
1. Create directory structure
2. Begin with constants file
3. Create function files (simplest first)
4. Create mathlib.hpp
5. Modify ereal.hpp
6. Create test
7. Build and verify
8. Document completion
9. Commit changes

---

**Plan Status:** AWAITING REVIEW
**Created:** 2025-11-03
**Author:** Claude Code (with Theodore Omtzigt)
