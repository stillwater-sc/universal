# Valid ereal Configuration Reference

## Algorithmic Constraint

The `ereal<maxlimbs>` type is based on Shewchuk's expansion arithmetic (two_sum/two_product algorithms), which requires all components and error terms to be representable as **normal IEEE-754 double-precision values**.

**Maximum valid configuration: `maxlimbs = 19`**

Larger values cause the last limb to underflow below `DBL_MIN` (2^-1022), violating the non-overlapping property and resulting in **incorrect arithmetic operations**.

Reference: Shewchuk, "Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates", 1997

---

## Standard Configurations

| Configuration | Decimal Digits | Threshold    | Typical Use Case |
|---------------|----------------|--------------|------------------|
| `ereal<1>`    | 15             | 10^-13       | Single double precision |
| `ereal<2>`    | 31             | 10^-29       | Double-double (dd) |
| `ereal<4>`    | 63             | 10^-61       | Quad-double (qd) |
| `ereal<8>`    | 127            | 10^-125      | **Default** - balanced precision |
| `ereal<12>`   | 191            | 10^-189      | High precision intermediate |
| `ereal<16>`   | 255            | 10^-253      | Very high precision |
| `ereal<19>`   | 303            | 10^-301      | **Maximum valid** configuration |

**Note**: Threshold values shown are for adaptive precision testing (margin = 2 decimal digits).

---

## Recommended Configurations by Use Case

### General Purpose Scientific Computing
**`ereal<8>` (default)** - 127 decimal digits
- Good balance of precision and performance
- Suitable for most scientific computations
- Threshold: 10^-125

### High-Precision Mathematical Functions
**`ereal<12>`** - 191 decimal digits
- Extended precision for mathematical libraries
- Suitable for iterative refinement algorithms
- Threshold: 10^-189

### Maximum Precision Applications
**`ereal<19>`** - 303 decimal digits
- Maximum algorithmically valid precision
- For computations requiring extreme accuracy
- Threshold: 10^-301 (clamped to representable minimum)

### Legacy Compatibility
**`ereal<2>`** - 31 decimal digits (double-double)
**`ereal<4>`** - 63 decimal digits (quad-double)
- Compatible with existing dd/qd libraries
- Well-studied precision characteristics

---

## Technical Details

### Precision Calculation
```
bits_precision = maxlimbs × 53  (mantissa bits per double)
decimal_digits = bits_precision × log₁₀(2)
               ≈ bits_precision × 0.30103
```

### Adaptive Testing Threshold
```
threshold = 10^-(decimal_digits - margin)
          where margin = 2 (default safety margin)
```

For `maxlimbs >= 20`, threshold underflows in double (< 10^-308).

### Why maxlimbs ≤ 19?

Each limb adds ~53 bits of precision. After n limbs:
```
smallest_correction ≈ 2^(-53n)

For algorithmic correctness:
2^(-53n) ≥ DBL_MIN = 2^(-1022)
-53n ≥ -1022
n ≤ 19.28
```

Therefore **maxlimbs = 19** is the maximum.

---

## Compile-Time Enforcement

The library enforces this constraint at compile time:

```cpp
template<unsigned maxlimbs = 8>
class ereal {
    static_assert(maxlimbs <= 19,
        "ereal<maxlimbs>: maxlimbs must be <= 19 to maintain algorithmic correctness...");
    // ...
};
```

Attempting to instantiate `ereal<20>` or larger will result in a compilation error with a clear explanation.

---

## Testing and Validation

All configurations `ereal<1>` through `ereal<19>` can be validated against IEEE-754 double-precision reference values using adaptive threshold testing.

The Universal library provides:
- `get_adaptive_threshold<Real>()` - Automatically scales threshold based on type precision
- `check_relative_error()` - Validates results within adaptive bounds
- `check_exact_value()` - For mathematically exact comparisons

See: `include/sw/universal/verification/test_suite_mathlib_adaptive.hpp`

---

## C'est la vie

It would be nice if IEEE-754 double precision could validate 20+ components (640+ decimal digits), but the constraint at `DBL_MIN` is a fundamental limitation of the underlying representation.

For precision beyond `ereal<19>`, external validation against arbitrary-precision libraries (MPFR, Boost.Multiprecision) would be required.
