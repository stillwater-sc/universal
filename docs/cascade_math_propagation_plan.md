# Mathematical Function Propagation Plan
## From dd → td_cascade & qd_cascade

**Date:** 2025-11-01  
**Goal:** Propagate the complete dd math library API to td_cascade and qd_cascade  
**Pattern:** Follow dd_cascade implementation as the reference template

---

## Executive Summary

**Current State:**
- **dd**: Complete math library (16 function files, ~1,458 LOC)
- **dd_cascade**: Complete math library (all 16 files ported)
- **td_cascade**: Only numerics.hpp (1 file)
- **qd_cascade**: Only numerics.hpp (1 file)

**Work Required:**
- Copy and adapt 15 function files × 2 cascade types = **30 files**
- Define missing constants for td_cascade and qd_cascade
- Update mathlib.hpp includes for both types
- Create test infrastructure

---

## File Inventory & Complexity

| File | Lines | Complexity | Functions | Dependencies |
|------|-------|------------|-----------|--------------|
| hypot.hpp | 15 | Low | 1 | sqrt |
| minmax.hpp | 19 | Low | 2 | - |
| error_and_gamma.hpp | 21 | Low | 2 | - |
| fractional.hpp | 21 | Low | 2 | - |
| truncate.hpp | 24 | Low | 4 | - |
| numerics.hpp | 35 | Low | 3 | ✅ EXISTS |
| hyperbolic.hpp | 41 | Medium | 6 | exp, log |
| cbrt.hpp | 43 | Medium | 1 | frexp, ldexp, sqr, constants |
| classify.hpp | 63 | Low | 8 | - |
| next.hpp | 64 | Medium | 4 | - |
| pow.hpp | 68 | Medium | 4 | exp, log, sqr |
| exponent.hpp | 105 | Medium | 4 | log, floor, pow |
| sqrt.hpp | 120 | High | 4 | sqr, reciprocal, constants |
| logarithm.hpp | 122 | High | 4 | frexp, ldexp, sqr, constants |
| trigonometry.hpp | 439 | High | 18 | Many interdependencies |
| old_logarithm.hpp | 258 | N/A | - | **SKIP** (legacy) |

---

## Transformation Pattern

### Type Replacements

| Source (dd) | dd_cascade | td_cascade | qd_cascade |
|-------------|------------|------------|------------|
| `dd` | `dd_cascade` | `td_cascade` | `qd_cascade` |
| `dd_third` | `ddc_third` | `tdc_third` | `qdc_third` |
| `dd_eps` | `ddc_eps` | `tdc_eps` | `qdc_eps` |

### Macro Replacements

| Source (dd) | dd_cascade | td_cascade | qd_cascade |
|-------------|------------|------------|------------|
| `DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION` | `DD_CASCADE_THROW_ARITHMETIC_EXCEPTION` | `TD_CASCADE_THROW_ARITHMETIC_EXCEPTION` | `QD_CASCADE_THROW_ARITHMETIC_EXCEPTION` |
| `DOUBLEDOUBLE_NATIVE_SQRT` | `DD_CASCADE_NATIVE_SQRT` | `TD_CASCADE_NATIVE_SQRT` | `QD_CASCADE_NATIVE_SQRT` |
| `dd_negative_sqrt_arg` | `dd_negative_sqrt_arg` | `td_negative_sqrt_arg` | `qd_negative_sqrt_arg` |
| `dd_negative_nroot_arg` | `dd_negative_nroot_arg` | `td_negative_nroot_arg` | `qd_negative_nroot_arg` |
| `dd_invalid_argument` | `dd_invalid_argument` | `td_invalid_argument` | `qd_invalid_argument` |

### Header Path Replacements

```
dd/math/functions/*.hpp          → td_cascade/math/functions/*.hpp
dd_cascade/math/functions/*.hpp  → qd_cascade/math/functions/*.hpp
```

---

## Prerequisites

### 1. Define Missing Constants

**Location:** `td_cascade_impl.hpp` and `qd_cascade_impl.hpp`

**td_cascade constants to add:**
```cpp
// simple constants (add near line ~430 in td_cascade_impl.hpp)
constexpr td_cascade tdc_third(0.33333333333333331, 1.8503717077085941e-17, 0.0);
constexpr double     tdc_eps = 4.93038065763132e-32;  // 2^-104 (inherited from dd precision)
```

**qd_cascade constants to add:**
```cpp
// simple constants (add near line ~445 in qd_cascade_impl.hpp)
constexpr qd_cascade qdc_third(0.33333333333333331, 1.8503717077085941e-17, 0.0, 0.0);
constexpr double     qdc_eps = 4.93038065763132e-32;  // 2^-104 (inherited from dd precision)
```

**Note:** These are approximations. More precise constants may be computed for triple and quad precision later.

### 2. Exception Class Definitions

**Check if these exist:**
```bash
grep -r "td_negative_sqrt_arg\|qd_negative_sqrt_arg" include/sw/universal/number/{td_cascade,qd_cascade}/
```

**If not, define in:** `td_cascade_exceptions.hpp`, `qd_cascade_exceptions.hpp` (or add to impl.hpp)

---

## Implementation Plan

### Phase 1: Low Complexity Files (5 files, ~100 LOC each)
**Effort:** 2-3 hours

1. **hypot.hpp** (15 lines)
   - Simple: `hypot(x, y) = sqrt(x*x + y*y)`
   - Dependency: sqrt (already exists in dd_cascade)

2. **minmax.hpp** (19 lines)
   - Simple: fmin, fmax wrappers
   - No dependencies

3. **error_and_gamma.hpp** (21 lines)
   - Stubs: erf, erfc (return error for now)
   - No dependencies (just placeholders)

4. **fractional.hpp** (21 lines)
   - Simple: modf, fmod wrappers
   - No dependencies

5. **truncate.hpp** (24 lines)
   - Simple: floor, ceil, trunc, round
   - No dependencies

**Action:**
- Copy from dd_cascade → td_cascade (5 files)
- Global replace: `dd_cascade` → `td_cascade`
- Copy from dd_cascade → qd_cascade (5 files)
- Global replace: `dd_cascade` → `qd_cascade`

### Phase 2: Medium Complexity Files (6 files, ~50-70 LOC each)
**Effort:** 4-5 hours

6. **classify.hpp** (63 lines)
   - Functions: fpclassify, isinf, isnan, isfinite, isnormal, isdenorm, iszero, signbit
   - No dependencies (uses standard library on .high() component)

7. **next.hpp** (64 lines)
   - Functions: nextafter, nexttoward, ulp, etc.
   - Medium complexity bit manipulation

8. **cbrt.hpp** (43 lines)
   - Cube root using Newton iteration
   - Dependencies: frexp, ldexp, sqr, reciprocal, constants (\_third)
   - **Requires:** tdc_third, qdc_third constants

9. **hyperbolic.hpp** (41 lines)
   - Functions: sinh, cosh, tanh, asinh, acosh, atanh
   - Dependencies: exp, log (currently stubs in td/qd mathlib.hpp)
   - **Note:** Will work once exp/log are implemented

10. **pow.hpp** (68 lines)
    - Functions: pow(dd,dd), pow(dd,double), pow(dd,int), npwr
    - Dependencies: exp, log, sqr
    - **Note:** npwr exists in floatcascade already

11. **exponent.hpp** (105 lines)
    - Functions: exp, expm1, exp2, exp10
    - Complex Taylor series implementations
    - Dependencies: floor, log, pow

**Action:**
- Copy & adapt each file (2 types × 6 files = 12 files)
- Replace type names and constants
- Verify dependency chain

### Phase 3: High Complexity Files (3 files, 120-439 LOC)
**Effort:** 8-10 hours

12. **sqrt.hpp** (120 lines)
    - Functions: sqrt, rsqrt, nroot
    - Karp's trick for sqrt, Newton iteration for nroot
    - **Already exists in dd_cascade** - verify td/qd versions work

13. **logarithm.hpp** (122 lines)
    - Functions: log, log2, log10, log1p
    - Taylor series and range reduction
    - Dependencies: frexp, ldexp, sqr, constants
    - **Critical:** Used by many other functions

14. **trigonometry.hpp** (439 lines)
    - Functions: sin, cos, tan, asin, acos, atan, atan2, sincos, etc.
    - Complex range reduction and Taylor series
    - Many interdependencies
    - **Most complex file**

**Action:**
- **Careful review** of dd_cascade versions
- Copy to td_cascade, test incrementally
- Copy to qd_cascade, test incrementally
- Create validation tests for each function

### Phase 4: Skip File
15. **old_logarithm.hpp** (258 lines)
    - **ACTION: SKIP** - This is legacy code
    - Not included in dd_cascade mathlib.hpp

---

## Testing Strategy

### Unit Tests (Per Function File)

Create test files in:
- `static/td_cascade/math/<function_category>.cpp`
- `static/qd_cascade/math/<function_category>.cpp`

**Test Pattern:**
```cpp
// Example: static/td_cascade/math/sqrt.cpp
#include <universal/number/td_cascade/td_cascade.hpp>

int test_sqrt() {
    td_cascade x(2.0);
    td_cascade result = sqrt(x);
    td_cascade expected(1.4142135623730950488, ...);
    // Validate components
}

int main() {
    int nrOfFailures = 0;
    nrOfFailures += test_sqrt();
    nrOfFailures += test_rsqrt();
    nrOfFailures += test_nroot();
    return nrOfFailures;
}
```

### Integration Tests

Use existing arithmetic tests to validate that math functions work correctly:
- `tdc_arith_arithmetic` (already passing)
- `qdc_arith_arithmetic` (already passing)

---

## mathlib.hpp Updates

### td_cascade/mathlib.hpp

**Add includes (uncomment):**
```cpp
#include <universal/number/td_cascade/math/functions/classify.hpp>
#include <universal/number/td_cascade/math/functions/error_and_gamma.hpp>
#include <universal/number/td_cascade/math/functions/exponent.hpp>
#include <universal/number/td_cascade/math/functions/fractional.hpp>
#include <universal/number/td_cascade/math/functions/hyperbolic.hpp>
#include <universal/number/td_cascade/math/functions/hypot.hpp>
#include <universal/number/td_cascade/math/functions/logarithm.hpp>
#include <universal/number/td_cascade/math/functions/minmax.hpp>
#include <universal/number/td_cascade/math/functions/next.hpp>
#include <universal/number/td_cascade/math/functions/pow.hpp>
#include <universal/number/td_cascade/math/functions/sqrt.hpp>
#include <universal/number/td_cascade/math/functions/trigonometry.hpp>
#include <universal/number/td_cascade/math/functions/truncate.hpp>
#include <universal/number/td_cascade/math/functions/cbrt.hpp>
```

**Remove placeholder implementations** (lines 64-168):
- exp, log, log10, log2, sin, cos, tan, asin, acos, atan, atan2
- sinh, cosh, tanh, asinh, acosh, atanh

**Keep:**
- pown() wrapper (delegates to floatcascade)
- floor() and ceil() implementations (currently in mathlib.hpp)

### qd_cascade/mathlib.hpp

**Same pattern as td_cascade**

---

## Dependency Order (Recommended Implementation Sequence)

```
1. Constants (tdc_third, qdc_third, tdc_eps, qdc_eps)
   ↓
2. Low-level primitives (already exist)
   - sqr, reciprocal, frexp, ldexp
   ↓
3. Phase 1: Independent functions
   - classify, minmax, truncate, fractional, error_and_gamma
   ↓
4. Phase 2a: sqrt & logarithm (needed by others)
   - sqrt, logarithm
   ↓
5. Phase 2b: exp & pow
   - exponent (exp functions)
   - pow
   ↓
6. Phase 2c: Dependent functions
   - hyperbolic (needs exp, log)
   - cbrt (needs sqrt, constants)
   - hypot (needs sqrt)
   - next
   ↓
7. Phase 3: Trigonometry (most complex)
   - trigonometry (needs many dependencies)
```

---

## Risk Assessment

### Low Risk
- **Phase 1 files**: Simple wrappers, minimal logic
- **classify.hpp**: Just uses std::fpclassify on .high()

### Medium Risk
- **sqrt.hpp, logarithm.hpp**: Complex algorithms, but well-tested in dd_cascade
- **pow.hpp, exponent.hpp**: Moderate complexity
- **Constants**: Need verification for triple/quad precision

### High Risk
- **trigonometry.hpp**: 439 lines, complex range reduction
- **Interdependencies**: Many functions call each other
- **Precision**: Need to verify algorithms work correctly for N=3, N=4

---

## Verification Checklist

For each file copied:
- [ ] Type names replaced (dd → td_cascade/qd_cascade)
- [ ] Constants replaced (dd_third → tdc_third/qdc_third)
- [ ] Macros replaced (DOUBLEDOUBLE → TD_CASCADE/QD_CASCADE)
- [ ] Header paths updated
- [ ] Compiles without errors
- [ ] Added to mathlib.hpp includes
- [ ] Unit test created (if applicable)
- [ ] Integration test passes

---

## Estimated Effort

| Phase | Files | Complexity | Hours | Notes |
|-------|-------|------------|-------|-------|
| Prerequisites | 2 | Low | 1 | Add constants |
| Phase 1 | 10 | Low | 3 | Simple copies |
| Phase 2 | 12 | Medium | 5 | Moderate complexity |
| Phase 3 | 6 | High | 10 | Complex algorithms |
| Testing | - | - | 4 | Unit + integration |
| Documentation | - | - | 1 | Update comments |
| **Total** | **30** | - | **24** | ~3 days |

---

## Success Criteria

1. ✅ All 15 function files exist in td_cascade/math/functions/
2. ✅ All 15 function files exist in qd_cascade/math/functions/
3. ✅ mathlib.hpp includes all function headers
4. ✅ All files compile without warnings
5. ✅ Arithmetic tests pass (already done)
6. ✅ Math function unit tests pass
7. ✅ No duplicate function definitions
8. ✅ Consistent naming conventions

---

## Notes

- **dd_cascade** serves as the reference implementation
- **Follow the existing patterns** in dd_cascade exactly
- **Test incrementally** - don't copy all files at once
- **Priority order**: sqrt/log → exp → everything else
- **Constants**: May need refinement for true triple/quad precision
- **Trigonometry**: Save for last due to complexity

