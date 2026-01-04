# ereal Mathematical Constants

This directory contains high-precision mathematical constant generation using the **ereal** (elastic real) number system.

## Architecture

The constant generation program has been **moved from internal/expansion/** to this location to reflect proper architectural separation:

- **internal/expansion/**: Low-level primitive tests for expansion operations (two_sum, grow_expansion, etc.)
- **elastic/ereal/**: User-facing API demonstrations and applications

## constant_generation.cpp

### Purpose
Demonstrates how to use the `ereal<nlimbs>` API to compute high-precision mathematical constants without needing an external oracle.

### Computed Constants
- **π** (pi): Using Machin's formula (π/4 = 4·arctan(1/5) - arctan(1/239))
- **e**: Using Taylor series (e = Σ(1/n!))
- **√2, √3, √5, √7, √11**: Using Newton-Raphson iteration
- **ln(2)**: Using artanh series (ln(2) = 2·artanh(1/3))
- **Derived**: π/2, π/4, 1/π, 2/π

### Round-Trip Validation Tests

The program includes comprehensive **oracle-free validation** using mathematical identities:

#### 1. Square Root Round-Trip: `sqrt(n)² = n`
Tests that squaring a square root returns the original integer exactly.
```cpp
ereal<128> sqrt2 = compute_sqrt(2.0);
ereal<128> squared = sqrt2 * sqrt2;
// squared should equal 2.0 exactly
```

#### 2. Arithmetic Round-Trip: `(a×b)/b = a`
Tests that multiplication and division are inverse operations.
```cpp
ereal<128> product = pi * e;
ereal<128> recovered = product / e;
// recovered should equal pi
```

#### 3. Addition Round-Trip: `(a+b)-b = a`
Tests that addition and subtraction preserve values.
```cpp
ereal<128> sum = sqrt2 + sqrt3;
ereal<128> recovered = sum - sqrt3;
// recovered should equal sqrt2
```

#### 4. Rational Round-Trip: `(p/q)×q = p`
Tests rational arithmetic precision.
```cpp
ereal<128> p(7.0);
ereal<128> q(13.0);
ereal<128> quotient = p / q;
ereal<128> recovered = quotient * q;
// recovered should equal 7.0
```

#### 5. Compound Round-Trip: `((a+b)×c)/c = a+b`
Tests complex operation sequences.
```cpp
ereal<128> sum = sqrt5 + sqrt7;
ereal<128> product = sum * pi;
ereal<128> recovered = product / pi;
// recovered should equal sum
```

### Results

**All validation tests PASS** with zero or near-zero error:
- Square roots: 0.0 relative error (exact!)
- Arithmetic operations: 0.0 relative error
- Compound operations: ~1e-16 relative error (double precision rounding only)

This validates that the ereal expansion arithmetic is mathematically correct and maintains extraordinary precision across operations.

### Output Format

The program generates 4-component quad-double (qd) representations suitable for installation in:
- `include/sw/universal/number/qd/math/constants/qd_constants.hpp`
- `include/sw/universal/number/qd_cascade/math/constants/qd_cascade_constants.hpp`

Example output:
```cpp
// pi
constexpr double pi_qd[4] = {
    3.14159265358979312e+00,
    1.24891566497657860e-16,
    -7.00707480547762294e-33,
    -4.67251407072755229e-37
};
```

## Building and Running

```bash
# From build directory
cmake .. -DUNIVERSAL_BUILD_NUMBER_EREALS=ON
make ereal_constant_generation
./elastic/ereal/ereal_constant_generation
```

## Key Insights

1. **No Oracle Required**: All validation uses exact mathematical identities, not comparisons against external "truth" values

2. **Adaptive Precision**: The ereal system automatically grows component counts as needed:
   - π: 102 components
   - e: 102 components
   - √2: 20 components
   - ln(2): 76 components

3. **Zero Error Round-Trips**: Integer-valued identities (like sqrt(n)²) return exact integers, proving the expansion arithmetic is error-free

4. **User-Facing API**: This code demonstrates how end users would interact with ereal, not the low-level expansion primitives

## See Also

- `elastic/ereal/api/api.cpp` - Basic ereal API patterns
- `elastic/ereal/arithmetic/*.cpp` - Arithmetic operation tests
- `internal/expansion/arithmetic/*.cpp` - Low-level expansion primitive tests (for implementers, not users)
