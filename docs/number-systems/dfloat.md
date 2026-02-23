# Decimal Floating-Point (dfloat): Base-10 Arithmetic Without Binary Rounding

## Why

The value 0.1 cannot be represented exactly in binary floating-point. Neither can 0.2, 0.3, or any decimal fraction whose denominator is not a power of two. This means that `0.1 + 0.2 != 0.3` in IEEE-754 -- a fact that has caused countless bugs in financial software, tax computation, and any domain where humans expect base-10 arithmetic to behave like base-10 arithmetic.

The `dfloat` type represents numbers in base-10, where decimal fractions *are* exact. The value 0.1 is stored as the digit 1 with a decimal exponent of -1, with no binary rounding error. This makes dfloat the correct choice for any computation that must match pencil-and-paper decimal arithmetic.

## What

`dfloat<ndigits, es, bt>` is a decimal floating-point number:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `ndigits` | `unsigned` | -- | Number of significant decimal digits |
| `es` | `unsigned` | -- | Exponent field width (binary-encoded decimal exponent) |
| `bt` | typename | `uint8_t` | Storage block type |

### Key Properties

- **Base-10 representation**: decimal fractions are exact
- **Decimal digits**: `ndigits` significant digits of precision
- **Decimal exponent**: value = significand x 10^exponent
- **IEEE 754-2008 compatible**: targets decimal interchange format (DPD/BID)
- **No binary rounding error** for values expressible as `d x 10^e`

### Status

The dfloat type is currently a skeleton implementation with the core infrastructure defined. The encoding format and arithmetic pipeline are specified, but full arithmetic operations are still under development.

## How It Works

Unlike binary floating-point (which stores significand x 2^exponent), dfloat stores significand x 10^exponent. Each decimal digit is encoded in the significand field, and the exponent represents a power of 10. This means:

- `0.1` = 1 x 10^-1 (exact)
- `0.01` = 1 x 10^-2 (exact)
- `9.99` = 999 x 10^-2 (exact)
- `1/3` = still inexact (0.333... must be truncated)

The encoding follows the IEEE 754-2008 decimal format principles, potentially using Densely Packed Decimal (DPD) encoding for efficient digit storage.

## How to Use It

### Include

```cpp
#include <universal/number/dfloat/dfloat.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
dfloat<7, 8> price(19.99);    // 7 significant decimal digits
dfloat<7, 8> tax_rate(0.075);  // 7.5% tax, exact in decimal
dfloat<7, 8> tax = price * tax_rate;
dfloat<7, 8> total = price + tax;
// No binary rounding: 19.99 and 0.075 are represented exactly
```

## Problems It Solves

| Problem | How dfloat Solves It |
|---------|-----------------------|
| `0.1 + 0.2 != 0.3` in binary floating-point | Decimal fractions are exact in base-10 |
| Financial calculations accumulate penny rounding errors | Exact representation of dollars and cents |
| Tax computation must match regulatory decimal rules | Base-10 arithmetic matches pencil-and-paper rules |
| Database NUMERIC/DECIMAL types lose precision in C++ | dfloat preserves decimal precision natively |
| Currency conversion chains compound binary errors | Decimal intermediate values stay exact |
| Regulatory compliance requires decimal rounding rules | Native decimal rounding (not binary approximation) |
