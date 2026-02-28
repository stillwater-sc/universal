# Hexadecimal Floating-Point (hfloat): IBM System/360 Radix-16 Arithmetic

## Why

In 1964, IBM introduced its System/360 mainframe with a hexadecimal (radix-16) floating-point format that became one of the most widely deployed floating-point representations in computing history. For over two decades, virtually every mainframe financial, scientific, and engineering application used this format. Billions of dollars of legacy code, datasets, and algorithms were designed around its behavior.

When IEEE 754 (radix-2) became the standard in 1985, it did not replace IBM HFP overnight. Legacy systems, archived data, and regulatory-certified code still depend on hexadecimal floating-point semantics. Converting these systems to IEEE 754 changes numerical results -- sometimes subtly, sometimes catastrophically -- because the two formats have fundamentally different rounding, normalization, and precision characteristics.

The `hfloat` type provides a faithful, portable C++ implementation of IBM hexadecimal floating-point, enabling:

- **Legacy validation**: verify that modern reimplementations match original mainframe results
- **Data archaeology**: read and interpret hexadecimal floating-point values from archived datasets
- **Numerical forensics**: understand why legacy code produces specific results that differ from IEEE 754
- **Education**: study how radix choice affects precision, rounding, and numerical stability

## What

`hfloat<ndigits, es, bt>` is a hexadecimal floating-point number:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `ndigits` | `unsigned` | -- | Number of hexadecimal fraction digits |
| `es` | `unsigned` | -- | Exponent field width in bits |
| `bt` | typename | `uint32_t` | Storage block type |

### Key Properties

- **Radix-16 representation**: each fraction digit encodes 4 bits
- **No hidden bit**: the leading hex digit is stored explicitly (unlike IEEE 754's implicit 1.x)
- **No NaN or infinity**: overflow saturates to maxpos/maxneg, NaN requests map to zero
- **No subnormals**: underflow goes directly to zero
- **Truncation rounding only**: IBM HFP never rounds up, it always truncates
- **Wobbling precision**: effective precision varies by 0-3 bits depending on the value

### Standard Formats

IBM defined three standard formats, all sharing a 7-bit exponent:

| Alias | Template | Bits | Hex Digits | Binary Digits | Approx Decimal Digits |
|-------|----------|------|------------|---------------|----------------------|
| `hfloat_short` | `hfloat<6, 7>` | 32 | 6 | 21-24 | 6-7 |
| `hfloat_long` | `hfloat<14, 7>` | 64 | 14 | 53-56 | 15-17 |
| `hfloat_extended` | `hfloat<28, 7>` | 128 | 28 | 109-112 | 32-34 |

The bit budget for each format:

```
1 (sign) + 7 (exponent) + ndigits*4 (fraction) = total bits

hfloat_short:     1 + 7 + 24  =  32
hfloat_long:      1 + 7 + 56  =  64
hfloat_extended:  1 + 7 + 112 = 128
```

## How It Works

### Value Encoding

The value of an hfloat is:

```
value = (-1)^sign * 16^(exponent - bias) * 0.f1f2...fn
```

where `f1` through `fn` are hexadecimal digits (0-F), each occupying 4 bits. The fraction is interpreted as a hexadecimal number between 0.0 and just below 1.0. The exponent bias is 64 for the standard 7-bit exponent.

**Example: encoding 1.0**

```
1.0 = 0.1₁₆ * 16¹
    sign = 0, exponent = 1 + 64 = 65, fraction = 0x100000
    binary: 0.1000001.000100000000000000000000
```

**Example: encoding 8.0**

```
8.0 = 0.8₁₆ * 16¹
    sign = 0, exponent = 1 + 64 = 65, fraction = 0x800000
    binary: 0.1000001.100000000000000000000000
```

### Normalization by Hex Digit

The critical difference from IEEE 754: normalization shifts by 4 bits (one hex digit) at a time, not 1 bit. The fraction is normalized so that the leading hex digit is non-zero (1-F), but within that digit, leading zero bits are allowed.

This means 1.0 and 8.0 share the same exponent even though their magnitudes differ by a factor of 8. The leading hex digit of 1.0 is `1` (binary `0001`) while for 8.0 it is `8` (binary `1000`).

### Wobbling Precision

Because normalization operates on hex-digit boundaries, the effective number of significant bits varies:

| Leading Hex Digit | Binary | Leading Zero Bits | Effective Precision (short) |
|-------------------|--------|-------------------|-----------------------------|
| 1 | `0001` | 3 | 21 bits |
| 2-3 | `001x` | 2 | 22 bits |
| 4-7 | `01xx` | 1 | 23 bits |
| 8-F | `1xxx` | 0 | 24 bits |

This "wobbling precision" means a 32-bit hfloat has between 21 and 24 significant bits, compared to IEEE 754 single precision which always has 24 bits (23 stored + 1 hidden). In the worst case, IBM HFP loses 3 bits of precision compared to a same-width IEEE format.

### Truncation Rounding

IBM HFP uses truncation (round toward zero) exclusively. When a result cannot be represented exactly, the trailing bits are simply discarded. This differs from IEEE 754's default round-to-nearest-even:

```
IEEE 754:  1.0 + 2^-24 rounds to 1.0 + 2^-23 (rounds up)
IBM HFP:   1.0 + 2^-24 truncates to 1.0 (rounds down)
```

Truncation is simpler to implement in hardware and makes results predictable, but it introduces a systematic negative bias in accumulated errors.

### No NaN, No Infinity, No Subnormals

IBM HFP predates the IEEE 754 concepts of NaN, infinity, and subnormal numbers:

- **Overflow**: saturates to maxpos (positive) or maxneg (negative) instead of producing infinity
- **Invalid operations**: produce zero instead of NaN
- **Underflow**: produces zero instead of a subnormal number (gradual underflow does not exist)

This means `isinf()` and `isnan()` always return `false` for hfloat values.

## How to Use It

### Include

```cpp
#include <universal/number/hfloat/hfloat.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
// Standard IBM formats
hfloat_short  a(42.0);       // 32-bit IBM HFP (6 hex digits)
hfloat_long   b(3.14159);    // 64-bit IBM HFP (14 hex digits)

// Arithmetic
auto sum  = a + b;
auto prod = a * b;
auto quot = a / b;

std::cout << "sum     : " << sum  << '\n';
std::cout << "product : " << prod << '\n';
std::cout << "quotient: " << quot << '\n';
```

### Inspecting Representations

```cpp
hfloat_short x(1.0);

std::cout << "value     : " << x << '\n';
std::cout << "binary    : " << to_binary(x) << '\n';
std::cout << "hex       : " << to_hex(x) << '\n';
std::cout << "components: " << components(x) << '\n';
std::cout << "type      : " << type_tag(x) << '\n';
// Output:
//   value     : 1
//   binary    : 0.1000001.000100000000000000000000
//   hex       : +0x0.100000 * 16^1
//   components: (+0x100000 * 16^1)
//   type      : hfloat_short (IBM HFP 32-bit)
```

### Special Values and Saturation

```cpp
hfloat_short maxval(SpecificValue::maxpos);
hfloat_short minval(SpecificValue::minpos);

// Overflow saturates instead of producing infinity
hfloat_short inf_request(SpecificValue::infpos);
std::cout << inf_request.isinf() << '\n';  // 0 (false, it's maxpos)

// NaN requests produce zero
hfloat_short nan_request(SpecificValue::qnan);
std::cout << nan_request.isnan() << '\n';  // 0 (false, it's zero)
```

### Dynamic Range

```cpp
hfloat_short s;
std::cout << dynamic_range(s) << '\n';

hfloat_long l;
std::cout << dynamic_range(l) << '\n';
```

### Custom Configurations

```cpp
// Non-standard configurations for research
hfloat<4, 5> narrow;         // 4 hex digits, 5-bit exponent (21 bits total)
hfloat<10, 7> mid;           // 10 hex digits, 7-bit exponent (48 bits total)
```

## Problems It Solves

| Problem | How hfloat Solves It |
|---------|-----------------------|
| Legacy mainframe code produces different results under IEEE 754 | Faithfully reproduces IBM HFP arithmetic with truncation rounding |
| Archived datasets contain hexadecimal floating-point values | Reads and interprets hex-encoded values with correct semantics |
| Regulatory-certified code must not change numerical behavior | Same arithmetic rules as the original System/360 hardware |
| Students need to understand how radix affects floating-point | Demonstrates wobbling precision, truncation bias, and hex normalization |
| Numerical forensics: "why does the mainframe get a different answer?" | Bit-exact comparison between HFP and IEEE 754 computation paths |

## Comparison with IEEE 754

| Feature | IEEE 754 (binary) | IBM HFP (hexadecimal) |
|---------|--------------------|-----------------------|
| Radix | 2 | 16 |
| Hidden bit | Yes (implicit leading 1) | No (explicit fraction) |
| Normalization granularity | 1 bit | 4 bits (1 hex digit) |
| Precision stability | Fixed | Wobbles by 0-3 bits |
| Rounding | Round-to-nearest-even (default) | Truncation only |
| NaN | Yes | No (maps to zero) |
| Infinity | Yes | No (saturates to maxpos/maxneg) |
| Subnormals | Yes (gradual underflow) | No (hard underflow to zero) |
| Exponent range (32-bit) | 2^-126 to 2^127 | 16^-64 to 16^63 |
| Precision (32-bit) | 24 bits (always) | 21-24 bits (wobbles) |
