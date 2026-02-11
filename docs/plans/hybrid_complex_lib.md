# Universal Complex Type: Implementation Plan

## Problem Statement

Apple Clang strictly enforces ISO C++ 26.2/2, which states `std::complex<T>` is only valid for `float`, `double`, and `long double`. This breaks Universal's ability to use complex arithmetic with custom types (posit, cfloat, fixpnt, lns, etc.) on macOS. Currently, complex tests are disabled in CI as a workaround.

## Recommended Solution: Standalone `sw::universal::complex<T>`

Implement a complete standalone complex type that:
- Is a drop-in replacement for `std::complex<T>`
- Works with all Universal number systems
- Provides consistent behavior across all compilers

## File Organization

```
include/sw/universal/
    math/
        complex.hpp                    # Main aggregation header (NEW)
        complex/
            complex_impl.hpp           # Core complex<T> class template (NEW)
            complex_traits.hpp         # C++20 concepts and traits (NEW)
            complex_operators.hpp      # Arithmetic operators (NEW)
            complex_functions.hpp      # Transcendental functions - default (NEW)
            complex_functions_dd.hpp   # Native dd transcendentals (NEW)
            complex_functions_qd.hpp   # Native qd transcendentals (NEW)
            complex_literals.hpp       # User-defined literals (NEW)
    traits/
        complex_traits.hpp             # is_complex trait (NEW)

static/complex/                        # Test directory (NEW)
    api/api.cpp
    arithmetic/{add,sub,mul,div}.cpp
    math/{transcendentals,trigonometry,hyperbolic}.cpp
    interop/std_complex.cpp
```

## Implementation Steps

### Step 1: Create Core Infrastructure

**File: `include/sw/universal/math/complex/complex_traits.hpp`**
- Define `Arithmetic` concept (basic arithmetic operations)
- Define `ComplexCompatible` concept (convertible to/from double)
- Define `is_universal_number<T>` trait with specializations for each number system

### Step 2: Implement Core Complex Class

**File: `include/sw/universal/math/complex/complex_impl.hpp`**
- Template class `complex<T>` where `T` satisfies `ComplexCompatible`
- Data members: `T _re, _im`
- Constructors: default, from real, from (real, imag), copy, move, converting
- Interop constructor from `std::complex<double>`
- Accessors: `real()`, `imag()`, setters
- Compound assignment operators: `+=`, `-=`, `*=`, `/=`
- Conversion operator to `std::complex<double>`

### Step 3: Implement Operators

**File: `include/sw/universal/math/complex/complex_operators.hpp`**
- Unary: `+`, `-`
- Binary: `+`, `-`, `*`, `/` (complex-complex and complex-scalar)
- Comparison: `==`, `!=`
- Free functions: `real()`, `imag()`, `conj()`, `norm()`, `abs()`, `arg()`, `polar()`
- Classification: `isnan()`, `isinf()`, `isfinite()`
- Stream I/O: `operator<<`, `operator>>`

### Step 4: Implement Transcendental Functions (Hybrid Approach)

**File: `include/sw/universal/math/complex/complex_functions.hpp`**

Strategy: **Hybrid approach** - delegate to `std::complex<double>` by default, with native implementations for high-precision types (dd, qd).

**Default implementation (for most types):**
```cpp
template<ComplexCompatible T>
complex<T> exp(const complex<T>& c) {
    std::complex<double> dc(double(c.real()), double(c.imag()));
    std::complex<double> result = std::exp(dc);
    return complex<T>(T(result.real()), T(result.imag()));
}
```

**Native implementation for dd/qd (preserves precision):**
```cpp
// exp(a+bi) = exp(a) * (cos(b) + i*sin(b))
template<>
complex<dd> exp(const complex<dd>& c) {
    dd exp_re = sw::universal::exp(c.real());
    dd cos_im = sw::universal::cos(c.imag());
    dd sin_im = sw::universal::sin(c.imag());
    return complex<dd>(exp_re * cos_im, exp_re * sin_im);
}
```

**File: `include/sw/universal/math/complex/complex_functions_dd.hpp`** (NEW)
- Native dd implementations for: `exp`, `log`, `sqrt`, `sin`, `cos`, `tan`

**File: `include/sw/universal/math/complex/complex_functions_qd.hpp`** (NEW)
- Native qd implementations for: `exp`, `log`, `sqrt`, `sin`, `cos`, `tan`

Functions to implement:
- Exponential: `exp`, `log`, `log10`
- Power: `pow`, `sqrt`
- Trigonometric: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`
- Hyperbolic: `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`

### Step 5: Create Aggregation Header

**File: `include/sw/universal/math/complex.hpp`**
- Include all complex component headers
- Add `to_binary()` function for complex types

### Step 6: Update Per-Number-System Headers

For each number system (posit, cfloat, fixpnt, lns, etc.), update their `math/complex.hpp`:
- Include new `<universal/math/complex.hpp>`
- Add overloads for both `sw::universal::complex<T>` and `std::complex<T>` (backward compat)
- Optionally deprecate `std::complex<T>` versions

**Files to update:**
- `include/sw/universal/number/posit/math/complex.hpp`
- `include/sw/universal/number/cfloat/math/functions/complex.hpp`
- `include/sw/universal/number/fixpnt/math/complex.hpp`
- `include/sw/universal/number/lns/math/complex.hpp`
- `include/sw/universal/number/posito/math/complex.hpp`
- `include/sw/universal/number/erational/math/complex.hpp`

### Step 7: Update Traits System

**File: `include/sw/universal/traits/complex_traits.hpp`**
- `is_sw_complex<T>` trait
- `number_traits<complex<T>>` specialization

### Step 8: Create Test Suite

**Directory: `static/complex/`**
- API tests: construction, assignment, conversion
- Arithmetic tests: exhaustive for small types, random for large
- Math tests: validate against `std::complex<double>` results
- Interop tests: conversion to/from `std::complex<double>`

### Step 9: Update CMake

**File: `CMakeLists.txt`**
- Re-enable `UNIVERSAL_BUILD_COMPLEX` option
- Add `static/complex/` subdirectory
- Remove Apple Clang special-casing for complex builds

## Key Design Decisions

1. **Complete reimplementation** (not wrapping std::complex) for:
   - Portability across all compilers
   - Full control over special value handling (NaR for posits)
   - Consistent behavior

2. **Hybrid transcendental functions**:
   - Default: delegate to `std::complex<double>` (simple, proven)
   - Native implementations for dd/qd types (preserves ~32/64 digit precision)
   - Extensible for future high-precision types

3. **C++20 concepts** for type constraints:
   - Clean API, clear error messages
   - Matches library's C++20 requirement

4. **Backward compatibility** via dual overloads:
   - Existing code using `std::complex<UniversalType>` continues to work (on permissive compilers)
   - Gradual migration path

## API Examples

```cpp
#include <universal/number/posit/posit.hpp>
#include <universal/math/complex.hpp>

using namespace sw::universal;

using Real = posit<32, 2>;
using Complex = complex<Real>;

Complex z1(Real(1.0), Real(2.0));  // 1 + 2i
Complex z2 = polar(Real(1.0), Real(3.14159/4));  // unit magnitude, 45 degrees

Complex sum = z1 + z2;
Complex product = z1 * z2;
Complex exponential = exp(z1);

Real magnitude = abs(z1);
Real phase = arg(z1);
Complex conjugate = conj(z1);

// Interop with std::complex<double>
std::complex<double> std_z = static_cast<std::complex<double>>(z1);
Complex back = Complex(std_z);
```

## Verification Plan

1. **Unit tests**: Run `ctest` after building with `-DUNIVERSAL_BUILD_COMPLEX=ON`
2. **Cross-platform CI**: Verify builds pass on macOS (Apple Clang), Linux (GCC/Clang), Windows (MSVC)
3. **Comparison tests**: Compare results against `std::complex<double>` for precision validation
4. **Existing tests**: Ensure all previously-disabled complex tests now pass

## Risk Mitigation

| Risk | Mitigation |
|------|------------|
| Precision loss in transcendentals | Document limitation; future: native dd/qd implementations |
| Breaking existing code | Dual overloads provide backward compatibility |
| Performance regression | Use constexpr; benchmark critical paths |
| Template bloat | Keep implementation simple; avoid over-engineering |

## Estimated Scope

- **New files**: 9 header files (including dd/qd native implementations), 1 CMakeLists.txt, ~12 test files
- **Modified files**: 6-8 existing complex.hpp files, root CMakeLists.txt
- **Lines of code**: ~1200-1500 new (including native dd/qd transcendentals), ~100 modified
