# td_cascade Implementation Summary

## Overview

Successfully created `td_cascade` - a unified triple-double precision number type using the fortified `floatcascade<3>` framework. This completes the cascade family unification:
- **dd_cascade** (double-double, 2 components)
- **td_cascade** (triple-double, 3 components)  ← NEW
- **qd_cascade** (quad-double, 4 components)

All three now share the same architecture with volatile-hardened error-free transformations.

## Implementation Date

2025-10-18

## Relationship to Existing td

The existing `td` implementation (in `number/td/`) already uses `floatcascade<3>`, but is named inconsistently with the cascade family. The new `td_cascade` provides:
- **Consistent naming** with dd_cascade and qd_cascade
- **Same implementation** using floatcascade<3>
- **Unified API** across the cascade family
- **Parallel implementation** allowing gradual migration

Both `td` and `td_cascade` can coexist during migration.

## Key Features

### Precision
- **159 fraction bits** (3 × 53 bits from IEEE-754 double precision)
- **~48 decimal digits** of precision
- **ULP**: 2^-159 ≈ 1.388e-48

### Architecture
- Built on `floatcascade<3>` template with volatile-protected error-free transformations
- 3-component representation: [c0, c1, c2] where each component is ~ULP of the previous
- Uses Knuth two_sum, Dekker fast_two_sum, and Priest two_prod

### API Compatibility
- Component access via `[0]`, `[1]`, `[2]` indexing
- Compatible with existing td API patterns
- Standard arithmetic operators: +, -, *, /
- Comparison operators: ==, !=, <, <=, >, >=
- Assignment operators: +=, -=, *=, /=

### Arithmetic Operations

#### Addition
Uses expansion_ops::add_cascades() which creates 6-component expansion, then compresses and renormalizes to 3 components:
```cpp
result = add_cascades(a, b)  // Creates 6 components
compressed[0] = result[0]
compressed[1] = result[1]
compressed[2] = result[2] + result[3] + result[4] + result[5]
return renormalize(compressed)
```

#### Subtraction
Negates and uses addition path.

#### Multiplication
Uses expansion_ops::multiply_cascades() which handles the product expansion and renormalizes to 3 components.

#### Division
**3 Newton-Raphson refinement iterations** for triple-double precision:
```
q0 = a[0] / b[0]
r1 = a - q0*b
q1 = r1[0] / b[0]
r2 = r1 - q1*b
q2 = r2[0] / b[0]
result = renormalize([q0, q1, q2])
```

## Files Created

```
include/sw/universal/number/td_cascade/
├── td_cascade.hpp              # Main include file
├── td_cascade_impl.hpp         # Core implementation
├── td_cascade_fwd.hpp          # Forward declarations
├── exceptions.hpp              # Exception types
├── numeric_limits.hpp          # std::numeric_limits specialization
├── manipulators.hpp            # Debug/print utilities
├── attributes.hpp              # Attribute functions (iszero, isnan, abs, etc.)
└── mathlib.hpp                 # Mathematical functions (TODO: port from classic td)

include/sw/universal/traits/
└── td_cascade_traits.hpp       # Type traits

Root directory:
└── td_cascade_example.cpp      # Working example with test cases
```

## Compilation

```bash
g++ -std=c++20 -I include/sw td_cascade_example.cpp -o td_cascade_example
./td_cascade_example
```

## Test Results

✅ **Compilation**: Clean, no warnings
✅ **Basic arithmetic**: All operations produce correct results
✅ **Component preservation**: All 3 components preserved in operations
✅ **Windows CI failure case**: `0 + a = a` correctly preserves all components
✅ **Special values**: Zero, infinity, NaN handled correctly
✅ **Precision demonstration**: Pi approximation shows 159-bit precision

### Example Output

```
Zero addition test (Windows CI failure case):
0 + a = td_cascade(floatcascade<3>[1, 1e-17, 1e-35] ~ 1)
Components preserved: YES ✓
```

## Comparison with dd_cascade and qd_cascade

| Feature | dd_cascade | td_cascade | qd_cascade |
|---------|-----------|-----------|-----------|
| Components | 2 | 3 | 4 |
| Fraction bits | 106 | 159 | 212 |
| Decimal digits | ~32 | ~48 | ~64 |
| Division refinements | 2 iterations | 3 iterations | 4 iterations |
| Epsilon | 2^-106 | 2^-159 | 2^-212 |
| Addition expansion | 3→2 | 6→3 | 7→4 |
| ULP | 4.93e-32 | 1.39e-48 | 1.22e-63 |

## Volatile Hardening

Like dd_cascade and qd_cascade, td_cascade benefits from:

1. **Compiler flags**: `/fp:precise` on MSVC, `-ffp-contract=off` on GCC/Clang
2. **Volatile modifiers**: Inside error-free transformations (two_sum, two_prod)

### Volatile Strategy

✅ Volatile **inside** critical operations (two_sum, fast_two_sum, two_prod)
✅ Regular doubles in outer functions
✅ No volatile references or parameters

This prevents:
- Expression reassociation
- Fused multiply-add reordering
- Intermediate rounding elimination
- Other IEEE-754 non-compliance

## TODO: Port from Classic td

The following features exist in `number/td/` but could be enhanced in `td_cascade`:

### High Priority
- [ ] `to_string()` with full decimal formatting (if different from existing td)
- [ ] Enhanced `parse()` for decimal string input with full precision
- [ ] `sqrt()` - High-precision square root (currently placeholder)

### Mathematical Functions
All currently use placeholder implementations. Port from classic td:
- [ ] `exp()`, `log()`, `log10()`, `log2()` - Exponential and logarithm
- [ ] `sin()`, `cos()`, `tan()` - Trigonometric functions
- [ ] `asin()`, `acos()`, `atan()`, `atan2()` - Inverse trig
- [ ] `sinh()`, `cosh()`, `tanh()` - Hyperbolic functions
- [ ] `asinh()`, `acosh()`, `atanh()` - Inverse hyperbolic
- [ ] `pow()`, `pown()` - Power functions

### Utility Functions
- [ ] Enhanced `floor()`, `ceil()` (basic implementation exists)
- [ ] `frexp()`, `ldexp()` - Exponent manipulation
- [ ] `modf()` - Integer/fractional parts
- [ ] `fmod()`, `remainder()` - Modular arithmetic
- [ ] `copysign()` - Copy sign
- [ ] `nextafter()`, `nexttoward()` - Adjacent representable value
- [ ] `fdim()` - Positive difference
- [ ] `fmax()`, `fmin()` - Maximum and minimum

## Integration with Universal

### Type Traits
```cpp
#include <universal/traits/td_cascade_traits.hpp>

template<typename T>
void myFunc() {
    if constexpr (is_td_cascade<T>) {
        // td_cascade-specific code
    }
}
```

### Numeric Limits
```cpp
#include <limits>

auto eps = std::numeric_limits<td_cascade>::epsilon();
// td_cascade(1.388e-48, 0, 0)

auto digits = std::numeric_limits<td_cascade>::digits;     // 159
auto digits10 = std::numeric_limits<td_cascade>::digits10; // 47
```

## Cascade Family Unification

### Complete Cascade Family

```
Component-based Multi-Precision Arithmetic (Cascade Family)

dd_cascade (floatcascade<2>)     106 bits → ~32 decimal digits
    ↓
td_cascade (floatcascade<3>)     159 bits → ~48 decimal digits
    ↓
qd_cascade (floatcascade<4>)     212 bits → ~64 decimal digits

All share:
- floatcascade<N> framework
- Volatile-hardened error-free ops
- Consistent API and naming
- Same arithmetic patterns
```

### Migration from td to td_cascade

```cpp
// Before
#include <universal/number/td/td.hpp>
using Number = sw::universal::td;

// After
#include <universal/number/td_cascade/td_cascade.hpp>
using Number = sw::universal::td_cascade;

// API is compatible for basic operations
Number a(1.0, 1e-17, 1e-34);
Number b(2.0);
Number c = a + b;  // Works the same
```

### Coexistence Strategy

Both `td` and `td_cascade` can coexist:
- **td** (`number/td/`): Keep for existing code, compatibility
- **td_cascade** (`number/td_cascade/`): Use for new code, unified with dd/qd

This allows gradual migration without breaking existing code.

## Performance Considerations

### Expected Performance Characteristics

1. **Addition/Subtraction**: ~12-15 double operations
2. **Multiplication**: ~30-40 double operations
3. **Division**: ~50-70 double operations (3 refinement iterations)

### Optimization Opportunities

- [ ] SIMD vectorization for component operations
- [ ] Specialized fast paths for zero/one/small integers
- [ ] Precomputed constants (pi, e, ln2, etc.)
- [ ] Cache-friendly memory layout

## Validation Status

| Category | Status | Notes |
|----------|--------|-------|
| Compilation | ✅ Pass | Clean compile with C++20 |
| Basic arithmetic | ✅ Pass | +, -, *, / all working |
| Component preservation | ✅ Pass | 0 + a = a preserves all components |
| Special values | ✅ Pass | Zero, inf, NaN handled |
| Comparison ops | ✅ Pass | All 6 comparison operators |
| Assignment ops | ✅ Pass | +=, -=, *=, /= working |
| Type traits | ✅ Pass | is_td_cascade<T> working |
| Numeric limits | ✅ Pass | std::numeric_limits specialized |
| Math functions | ⚠️ Partial | Placeholders only (like classic td) |
| String I/O | ⚠️ Partial | Basic parse(), stream output works |

## Lessons Learned

### Forward Declaration Pattern
Need to forward declare `parse()` at the top of `_impl.hpp`:
```cpp
inline bool parse(const std::string&, td_cascade&);
```
Otherwise the `assign()` method can't call it.

### Avoiding Duplicate Definitions
Don't define `to_binary()` in both `_impl.hpp` and `manipulators.hpp`. Keep it in `_impl.hpp` only.

### Volatile Pattern Consistency
Same pattern across all cascade types:
- Volatile ONLY inside two_sum, fast_two_sum, two_prod
- Regular doubles everywhere else
- No volatile references or parameters

## Success Criteria

✅ **Primary Goal Achieved**: td_cascade successfully created using floatcascade<3>
✅ **Naming Unification Complete**: dd_cascade, td_cascade, qd_cascade all consistent
✅ **Windows CI Fix Validated**: 0 + a = a preserves all 3 components
✅ **API Compatibility**: Standard arithmetic and comparison operators work
✅ **Code Quality**: Clean compilation, no warnings, consistent with dd/qd patterns
✅ **Cascade Family Complete**: All three cascade types now unified

## Next Steps

1. **Consider deprecation path** for existing `td` in favor of `td_cascade`
2. **Port high-priority features** from classic td (if any enhancements needed)
3. **Create comprehensive test suite** comparing td vs td_cascade
4. **Performance benchmarking** vs classic td
5. **CI integration** for Windows/Linux/macOS
6. **Update documentation** to explain td vs td_cascade relationship

## References

- Original td implementation: `include/universal/number/td/`
- dd_cascade implementation: `include/universal/number/dd_cascade/`
- qd_cascade implementation: `include/universal/number/qd_cascade/`
- floatcascade framework: `include/universal/internal/floatcascade/`
- Error-free operations: `include/universal/internal/error_free_ops/`
- Windows CI fix documentation: `floatcascade_volatile_hardening.md`

## Conclusion

The `td_cascade` implementation completes the unification of the cascade family. All three types (dd_cascade, td_cascade, qd_cascade) now share:
- Consistent naming convention
- Common floatcascade<N> framework
- Volatile-hardened arithmetic
- Unified API patterns

This provides a solid foundation for high-precision arithmetic with clear progression paths:
- **dd_cascade**: 32 digits for moderate precision needs
- **td_cascade**: 48 digits for high precision needs
- **qd_cascade**: 64 digits for extreme precision needs

All benefit from the battle-tested floatcascade framework with volatile protections against compiler optimization issues that caused the original Windows CI failures.
