# Unified Decimal Conversion API for Universal Floating-Point Types

## Overview

This document describes the unified decimal conversion facility for the Universal library, providing a central mechanism to convert arbitrary-precision floating-point values into human-readable decimal strings.

## Problem Statement

Universal contains many floating-point formats (fixpnt, lns, dbns, posit, takum, dd, qd, priest) with arbitrary precision configurations that cannot reliably convert to decimal strings by marshalling through native C++ types (float/double/long double) as this loses precision.

## Solution Architecture

The solution implements the **Dragon algorithm** (Steele & White, 1990) adapted for Universal's internal triple representations:

1. **`value<fbits>`**: (sign, scale, fraction_without_hidden_bit) - fraction managed as bitblock
2. **`blocktriple<fbits, op, bt>`**: (sign, scale, significand) - significand managed as multi-limb blocks

### Key Components

```
include/sw/universal/number/support/
├── decimal.hpp             (existing: arbitrary-precision decimal arithmetic)
├── dragon.hpp              (new: Dragon algorithm core)
└── decimal_converter.hpp   (new: unified conversion API)

conversion/
└── decimal_converter_test.cpp  (new: comprehensive test suite)
```

## API Reference

### Core Conversion Functions

#### `to_decimal_string` for `value<fbits>`

```cpp
template<unsigned fbits>
std::string to_decimal_string(
    const internal::value<fbits>& v,
    std::ios_base::fmtflags flags = std::ios_base::dec,
    std::streamsize precision = 6
);
```

**Parameters:**
- `v`: value<> to convert
- `flags`: ioflags controlling format (scientific, fixed, showpos, uppercase)
- `precision`: number of digits after decimal point

**Returns:** Formatted decimal string

**Example:**
```cpp
value<52> v(3.14159265358979);
std::string s1 = to_decimal_string(v, std::ios_base::scientific, 15);
// Returns: "3.141592653589790e+00"

std::string s2 = to_decimal_string(v, std::ios_base::fixed, 8);
// Returns: "3.14159265"
```

#### `to_decimal_string` for `blocktriple<fbits>`

```cpp
template<unsigned fbits, BlockTripleOperator op, typename bt>
std::string to_decimal_string(
    const blocktriple<fbits, op, bt>& triple,
    std::ios_base::fmtflags flags = std::ios_base::dec,
    std::streamsize precision = 6
);
```

**Parameters:** Same as value<> version

**Example:**
```cpp
blocktriple<52, BlockTripleOperator::REP, uint32_t> bt(1.0e-100);
std::string s = to_decimal_string(bt, std::ios_base::scientific, 20);
// Returns high-precision decimal representation
```

### Stream Insertion Operators

The API provides stream insertion operators that respect std::ostream formatting:

```cpp
template<unsigned fbits>
std::ostream& operator<<(std::ostream& ostr, const internal::value<fbits>& v);

template<unsigned fbits, BlockTripleOperator op, typename bt>
std::ostream& operator<<(std::ostream& ostr, const blocktriple<fbits, op, bt>& triple);
```

**Example:**
```cpp
value<112> v(1.234567890123456789);
std::cout << std::scientific << std::setprecision(18) << v << '\n';
// Output: 1.234567890123456789e+00

std::cout << std::fixed << std::setprecision(6) << v << '\n';
// Output: 1.234568
```

## Supported ioflags

The conversion facility respects standard iostream formatting flags:

| Flag | Effect |
|------|--------|
| `std::ios_base::scientific` | Use scientific notation (d.ddde±ee) |
| `std::ios_base::fixed` | Use fixed-point notation (ddd.ddd) |
| `std::ios_base::showpos` | Show '+' for positive numbers |
| `std::ios_base::uppercase` | Use 'E' instead of 'e' for exponent |
| `std::ios_base::left` | Left-align output (with width) |
| `std::ios_base::right` | Right-align output (with width, default) |

### Additional Stream Manipulators

- `std::setprecision(n)`: Set number of digits after decimal point
- `std::setw(n)`: Set field width
- `std::setfill(c)`: Set fill character for padding

## Usage Examples

### Example 1: Basic Conversion

```cpp
#include <universal/number/support/decimal_converter.hpp>
#include <universal/internal/value/value.hpp>

using namespace sw::universal;

internal::value<52> v(123.456);
std::string s = to_decimal_string(v, std::ios_base::fixed, 2);
std::cout << s << '\n';  // Output: 123.46
```

### Example 2: High-Precision Scientific Notation

```cpp
internal::value<112> v(1.0e-200);  // Very small number
std::string s = to_decimal_string(v, std::ios_base::scientific, 50);
std::cout << s << '\n';  // Output: 1.00000...e-200 (with 50 digits precision)
```

### Example 3: Using with Posit Types

```cpp
#include <universal/number/posit/posit.hpp>
#include <universal/number/support/decimal_converter.hpp>

posit<256, 5> p(3.141592653589793238462643383279502884197);

// Posit internally uses value<> or blocktriple<>
// The posit's operator<< can use to_decimal_string internally
std::cout << std::setprecision(35) << std::scientific << p << '\n';
```

### Example 4: Stream Formatting

```cpp
value<52> v1(1234.5);
value<52> v2(-0.000123);

std::cout << std::setw(15) << std::setfill('*') << std::left << v1 << '\n';
// Output: 1234.5*********

std::cout << std::setw(15) << std::right << std::scientific << v2 << '\n';
// Output: **-1.230000e-04
```

### Example 5: Integrating with Existing Types

To integrate decimal conversion into an existing Universal number type:

```cpp
// In your type's header file:
#include <universal/number/support/decimal_converter.hpp>

template<size_t nbits, size_t es>
class posit {
    // ... existing implementation ...

public:
    friend std::ostream& operator<<(std::ostream& ostr, const posit& p) {
        // Convert posit to internal value<> or blocktriple<>
        internal::value<fbits> v = p.to_value();

        // Use unified decimal converter
        std::string s = to_decimal_string(v, ostr.flags(), ostr.precision());

        // Handle width/fill/alignment
        return decimal_format_inserter(ostr, s);
    }
};
```

## Algorithm Details

### Dragon Algorithm Overview

The Dragon algorithm converts floating-point numbers to decimal without using floating-point arithmetic:

1. **Input**: (sign, binary_exponent, binary_mantissa)
2. **Representation**: Value = ±mantissa × 2^exponent
3. **Goal**: Generate decimal digits d₀.d₁d₂... × 10^k

**Key Insight**: 10 = 2 × 5, so 10^k = 2^k × 5^k

The algorithm uses only integer arithmetic (via the `decimal` class) to:
- Scale the mantissa to the appropriate decimal range
- Extract digits iteratively
- Handle rounding correctly

### Advantages

- **Exact**: No intermediate floating-point rounding errors
- **Arbitrary Precision**: Works for any Universal floating-point type
- **Correct Rounding**: Produces correctly rounded decimal output
- **Deterministic**: Same input always produces same output

### Performance Considerations

For very high precision (> 100 decimal digits), the algorithm may be slower than native conversions due to arbitrary-precision arithmetic. For typical use cases (≤ 50 digits), performance is excellent.

## Special Cases

The API correctly handles all IEEE-754 special cases:

| Input | Output |
|-------|--------|
| +0.0 | "0" or "0.000000" (depending on precision) |
| -0.0 | "-0" or "-0.000000" |
| +∞ | "inf" or "+inf" (with showpos) |
| -∞ | "-inf" |
| qNaN | "qnan" (quiet NaN) |
| sNaN | "snan" (signaling NaN) |

## Migration Guide

### From Native Type Marshalling

**Before:**
```cpp
// Loses precision for large fbits!
value<112> v(...);
std::cout << double(v) << '\n';  // Truncates to ~15 digits
```

**After:**
```cpp
value<112> v(...);
std::cout << std::setprecision(30) << v << '\n';  // Full precision
```

### From Manual String Construction

**Before:**
```cpp
std::string my_custom_to_string(const MyType& x) {
    // Custom, error-prone conversion logic
    // ...
}
```

**After:**
```cpp
#include <universal/number/support/decimal_converter.hpp>

std::string my_custom_to_string(const MyType& x) {
    internal::value<fbits> v = x.to_value();
    return to_decimal_string(v, std::ios_base::scientific, desired_precision);
}
```

## Testing

Run the test suite:

```bash
cd build
make decimal_converter_test
./conversion/decimal_converter_test
```

The test suite covers:
- Basic Dragon algorithm functions
- value<> conversions (various precisions)
- blocktriple<> conversions (various operators)
- ioflags variations
- Stream insertion operators
- Special cases (zero, infinity, NaN)
- Edge cases (very large/small numbers)

## References

1. Steele, Guy L., Jr., and Jon L. White. "How to print floating-point numbers accurately." *ACM SIGPLAN Notices* 25.6 (1990): 112-126.

2. Burger, Robert G., and R. Kent Dybvig. "Printing floating-point numbers quickly and accurately." *ACM SIGPLAN Notices* 31.5 (1996): 108-116.

3. Loitsch, Florian. "Printing floating-point numbers quickly and accurately with integers." *ACM SIGPLAN Notices* 45.6 (2010): 233-243. (Grisu algorithm)

## Future Enhancements

Potential future improvements:

1. **Grisu Algorithm**: Implement faster Grisu2/Grisu3 for common cases
2. **Caching**: Cache powers of 10 for repeated conversions
3. **Parallel Digit Extraction**: For very high precision
4. **Custom Allocators**: For large decimal arithmetic
5. **Formatting Options**: Thousands separators, localization

## Support

For questions or issues:
- GitHub Issues: https://github.com/stillwater-sc/universal/issues
- Documentation: https://github.com/stillwater-sc/universal/wiki

---

**Copyright © 2017-2025 Stillwater Supercomputing, Inc.**
**License**: MIT Open Source License
