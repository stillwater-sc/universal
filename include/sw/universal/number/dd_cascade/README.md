# dd_cascade - Double-Double using floatcascade<2>

## Overview

`dd_cascade` is a modernized implementation of double-double arithmetic using the `floatcascade<2>` framework. It provides ~106 bits of precision (~32 decimal digits) through the combination of two IEEE-754 double-precision values.

## Purpose

This implementation exists to:

1. **Unify the codebase** - All multi-component types (dd, td, qd) will eventually use the same floatcascade infrastructure
2. **Leverage fortified operations** - Benefits from volatile-hardened error-free transformations in floatcascade
3. **Maintain compatibility** - Provides the same API as classic `dd` (high(), low() accessors)
4. **Enable future features** - Foundation for Priest's algorithms and ELREALO (Exact Lazy Real Objects)

## Current Status

### ✅ Implemented
- Core arithmetic operations (+, -, *, /)
- Comparison operators
- Component access (high(), low(), operator[])
- Array-style indexing compatible with cascade framework
- Type conversions to/from native types
- Special value handling (inf, nan, zero)
- Basic attributes (iszero, isnan, isinf, etc.)

### 🚧 To Be Ported from Classic dd

The following features need to be ported from the mature `dd` implementation:

#### High Priority
- [ ] **Sophisticated string formatting** - Full `to_string()` with precision, width, scientific notation
- [ ] **Decimal string parsing** - Robust `parse()` function for reading decimal strings
- [ ] **Stream I/O operators** - Full iostream integration with formatting flags

#### Mathematical Functions
- [ ] `sqrt()` - High-precision square root
- [ ] `exp()`, `log()`, `log10()` - Exponential and logarithmic functions
- [ ] `sin()`, `cos()`, `tan()` - Trigonometric functions
- [ ] `asin()`, `acos()`, `atan()`, `atan2()` - Inverse trigonometric functions
- [ ] `sinh()`, `cosh()`, `tanh()` - Hyperbolic functions
- [ ] `asinh()`, `acosh()`, `atanh()` - Inverse hyperbolic functions
- [ ] `pow()`, `pown()` - Power functions
- [ ] `frexp()`, `ldexp()` - Mantissa/exponent manipulation
- [ ] Additional functions (modf, fmod, copysign, etc.)

#### Testing & Validation
- [ ] Cross-validation tests against classic `dd`
- [ ] Performance benchmarks
- [ ] Edge case testing
- [ ] Regression test suite

## API Compatibility

`dd_cascade` maintains API compatibility with classic `dd`:

```cpp
// Classic dd style
dd_cascade x(1.0, 1e-17);
double hi = x.high();  // 1.0
double lo = x.low();   // 1e-17

// Also supports cascade style
double comp0 = x[0];   // 1.0
double comp1 = x[1];   // 1e-17
```

## Usage Example

```cpp
#include <universal/number/dd_cascade/dd_cascade.hpp>

using namespace sw::universal;

int main() {
    dd_cascade a(1.0, 1e-17);
    dd_cascade b(2.0, 2e-17);

    dd_cascade sum = a + b;      // High-precision addition
    dd_cascade product = a * b;   // High-precision multiplication

    std::cout << "Sum: " << sum << std::endl;

    return 0;
}
```

## Design Principles

1. **Defense in depth** - Uses both `/fp:precise` compiler flags and volatile modifiers
2. **Zero overhead** - When not needed, costs nothing (header-only)
3. **Composability** - Works seamlessly with other Universal types
4. **Testability** - Can be validated against classic dd implementation

## Migration Path

The long-term plan is:

1. **Phase 1** (Current) - Parallel implementation alongside classic `dd`
2. **Phase 2** - Port features from classic `dd` to reach feature parity
3. **Phase 3** - Extensive cross-validation and testing
4. **Phase 4** - Make `dd` an alias to `dd_cascade` (with deprecation warnings)
5. **Phase 5** - Remove classic implementation after transition period

## Technical Details

### Precision
- **Total bits**: 128 (2 × 64-bit doubles)
- **Fraction bits**: 106 (non-overlapping mantissa bits)
- **Decimal digits**: ~32 (31.95...)
- **Epsilon**: 2^-106 ≈ 4.93e-32

### Implementation
- Built on `floatcascade<2>` template
- Uses fortified error-free transformations (two_sum, two_prod)
- All critical operations protected with volatile modifiers
- Compatible with `/fp:precise` compiler mode

### Dependencies
- `universal/internal/floatcascade/floatcascade.hpp` - Core cascade operations
- Standard C++ headers (no external dependencies)

## Contributing

When porting features from classic `dd`:

1. Maintain the same function signatures for compatibility
2. Add tests that cross-validate against classic implementation
3. Document any differences in behavior or precision
4. Mark TODOs when using placeholder implementations

## References

- Classic dd implementation: `include/sw/universal/number/dd/`
- floatcascade framework: `include/sw/universal/internal/floatcascade/`
- Related types: td (triple-double), qd (quad-double)

## Questions?

See the main Universal documentation or raise an issue on GitHub.
