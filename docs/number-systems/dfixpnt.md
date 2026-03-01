# Decimal Fixed-Point (dfixpnt): Exact Base-10 Fractional Arithmetic

## Why

Binary fixed-point (`fixpnt`) gives you deterministic arithmetic, but it cannot represent common decimal fractions exactly. The value 0.1 has an infinite binary expansion (0.00011001100...), so a binary fixed-point type must round it. For financial ledgers, tax computation, COBOL migration, point-of-sale systems, and any domain governed by decimal rounding rules, this is a correctness bug, not a performance tradeoff.

Decimal floating-point (`dfloat`) solves the representation problem but introduces a variable exponent that makes the type more complex than many applications need. Most business arithmetic operates on values with a *fixed* number of decimal places: 2 for currency, 4 for unit prices, 6 for interest rates. The number of fractional digits never changes, and the range is bounded and known at compile time.

`dfixpnt` combines the strengths of both: it represents values in base-10 (so 0.1 is exact) with a fixed radix point (so the type is simple, bounded, and deterministic). Every digit occupies a known position, there is no exponent to manage, and every operation reduces to decimal digit arithmetic with predictable, bounded precision.

## What

`dfixpnt<ndigits, radix, encoding, arithmetic, bt>` is a signed decimal fixed-point number:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `ndigits` | `unsigned` | -- | Total number of decimal digits |
| `radix` | `unsigned` | -- | Number of fractional digits (digits after the decimal point) |
| `encoding` | `DecimalEncoding` | `BCD` | Internal digit encoding: `BCD`, `BID`, or `DPD` |
| `arithmetic` | `bool` | `Modulo` | `Modulo` (wrap on overflow) or `Saturate` (clamp to range) |
| `bt` | typename | `uint8_t` | Storage block type for underlying `blockbinary` |

### Value Representation

A `dfixpnt<ndigits, radix>` stores a sign bit and an unsigned decimal magnitude. The represented value is:

```text
value = (-1)^sign * magnitude * 10^(-radix)
```

For example, `dfixpnt<8, 3>` uses 5 integer digits and 3 fractional digits:
- Value `456.789` is stored as sign=0, magnitude=456789
- Resolution: 10^-3 = 0.001
- Range: [-99999.999, +99999.999]

### Key Properties

- **Base-10 representation**: 0.1, 0.01, 19.99 are all exact, no binary rounding
- **Fixed precision**: the number of fractional digits is set at compile time
- **Deterministic**: decimal digit arithmetic, bit-exact on every platform
- **Trivially copyable**: no heap allocation, suitable for embedded targets and hardware co-design
- **Two overflow modes**: `Modulo` wraps silently; `Saturate` clamps to maxpos/maxneg
- **Three encodings**: BCD (4 bits/digit), BID (binary integer), DPD (10 bits/3 digits)
- **No NaN, no infinity, no denormals**: every state is a valid number
- **`is_exact = true`**: `std::numeric_limits` reports exact arithmetic (radix 10)

### Digit Encodings

The `encoding` template parameter controls how decimal digits are packed into binary storage:

| Encoding | Bits per Digit | Storage for 8 Digits | Tradeoff |
|----------|---------------|---------------------|----------|
| `BCD` | 4.0 | 32 bits | Simple nibble access, standard BCD hardware support |
| `BID` | ~3.32 | 27 bits | Most compact, fast binary arithmetic, slower digit access |
| `DPD` | ~3.33 | 30 bits | IEEE 754-2008 compatible, efficient 3-digit packing |

All three encodings produce identical arithmetic results. BCD is the default and recommended encoding for most use cases.

### Supported Operations

- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Conversions: to/from `int`, `long long`, `float`, `double`
- String I/O: `operator<<`, `operator>>`, `assign("123.456")`, `to_string()`
- Digit access: `digit(i)`, `setdigit(i, d)` (index 0 = least significant)
- Range queries: `minpos()`, `maxpos()`, `minneg()`, `maxneg()`, `zero()`
- Inspection: `type_tag()`, `to_binary()`, `color_print()`

## How It Works

Internally, `dfixpnt` stores its value as a sign-magnitude pair:

```cpp
bool _sign;                                    // true = negative
blockdecimal<ndigits, encoding, bt> _block;    // unsigned magnitude
```

The `blockdecimal` class packs decimal digits into a `blockbinary` using the chosen encoding. All arithmetic is performed as decimal digit operations:

- **Addition/subtraction**: digit-by-digit with decimal carry/borrow, sign resolved by comparing magnitudes
- **Multiplication**: schoolbook digit-by-digit into a double-width `blockdecimal<2*ndigits>`, then shifted right by `radix` positions (dividing by 10^radix to restore the fixed-point scale)
- **Division**: dividend is shifted left by `radix` positions (multiplying by 10^radix) in a wider `blockdecimal`, then divided by the divisor, producing a result at the correct scale

The `Saturate` mode detects overflow after each addition and clamps the result to the maximum representable magnitude.

### Comparison with Binary Fixed-Point

| Feature | `fixpnt<nbits, rbits>` | `dfixpnt<ndigits, radix>` |
|---------|------------------------|---------------------------|
| Base | 2 | 10 |
| 0.1 exact? | No | Yes |
| Resolution | 2^(-rbits) | 10^(-radix) |
| Storage | nbits binary bits | BCD/BID/DPD encoded digits |
| Arithmetic | Binary add/subtract | Decimal digit add/subtract |
| Use case | DSP, control systems | Finance, COBOL, business logic |

## How to Use It

### Include

```cpp
#include <universal/number/dfixpnt/dfixpnt.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
// 8 total digits, 2 fractional: range +/-999999.99, resolution 0.01
dfixpnt<8, 2> price(19.99);
dfixpnt<8, 2> tax_rate(0.08);   // 8% tax, exact in decimal
dfixpnt<8, 2> tax = price * tax_rate;
dfixpnt<8, 2> total = price + tax;

std::cout << "Price: " << price << '\n';
std::cout << "Tax:   " << tax   << '\n';
std::cout << "Total: " << total << '\n';
```

### String-Based Construction

```cpp
// Parse exact decimal values from strings
dfixpnt<8, 3> a;
a.assign("456.789");
std::cout << a << '\n';              // 456.789
std::cout << to_binary(a) << '\n';   // BCD nibble representation
```

### Digit Access

```cpp
dfixpnt<8, 3> a;
a.assign("456.789");

// digit(0) is the least significant digit
// For 456.789: digit(0)=9, digit(1)=8, digit(2)=7, digit(3)=6, digit(4)=5, digit(5)=4
std::cout << "cents digit: " << a.digit(0) << '\n';  // 9
std::cout << "ones digit:  " << a.digit(3) << '\n';  // 6
```

### Saturating Arithmetic

```cpp
// Saturate mode: clamp instead of wrapping on overflow
dfixpnt<4, 2, DecimalEncoding::BCD, Saturate> balance(50.00);
dfixpnt<4, 2, DecimalEncoding::BCD, Saturate> deposit(99.00);

balance += deposit;
// With Modulo: would wrap (4 digits = max 99.99)
// With Saturate: clamps to 99.99
std::cout << balance << '\n';
```

### Encoding Selection

```cpp
// All three encodings produce identical arithmetic results
using BCD8 = dfixpnt<8, 3, DecimalEncoding::BCD>;
using BID8 = dfixpnt<8, 3, DecimalEncoding::BID>;
using DPD8 = dfixpnt<8, 3, DecimalEncoding::DPD>;

BCD8 bcd(123);
BID8 bid(123);
DPD8 dpd(123);

std::cout << "BCD: " << to_binary(bcd) << " : " << bcd << '\n';
std::cout << "BID: " << to_binary(bid) << " : " << bid << '\n';
std::cout << "DPD: " << to_binary(dpd) << " : " << dpd << '\n';
// All print 123.000, but with different internal bit patterns
```

### Plug-in Replacement

```cpp
template<typename Decimal>
Decimal compute_total(Decimal price, Decimal quantity, Decimal discount) {
    Decimal subtotal = price * quantity;
    return subtotal - subtotal * discount;
}

// Use with double during prototyping
auto result_d = compute_total(19.99, 3.0, 0.10);

// Switch to exact decimal for production
using Money = dfixpnt<10, 2>;
auto result_m = compute_total(Money(19.99), Money(3), Money(0.10));
// No binary rounding: 19.99 and 0.10 are exact
```

### Inspecting Representations

```cpp
dfixpnt<8, 3> x(42);

std::cout << "value    : " << x << '\n';
std::cout << "string   : " << x.to_string() << '\n';
std::cout << "binary   : " << to_binary(x) << '\n';
std::cout << "type     : " << type_tag(x) << '\n';
std::cout << "fields   : " << type_field(x) << '\n';
// Output:
//   value    : 42.000
//   string   : 42.000
//   binary   : 0.00000100'0010.000000000000
//   type     : dfixpnt<  8,   3, BCD,     Modulo, h>
//   fields   : fields(i:5|f:3)
```

## Problems It Solves

| Problem | How dfixpnt Solves It |
|---------|-----------------------|
| Binary fixed-point cannot represent 0.1, 0.01 exactly | Base-10 digits: every decimal fraction is exact |
| Floating-point `0.1 + 0.2 != 0.3` | Fixed decimal arithmetic: 0.1 + 0.2 == 0.3 always |
| Financial ledgers accumulate penny rounding errors | Exact dollar-and-cent arithmetic with configurable decimal places |
| COBOL PICTURE clauses need exact decimal fixed-point semantics | `dfixpnt<ndigits, radix>` maps directly to COBOL `PIC 9(n)V9(r)` |
| Tax computation must follow regulatory decimal rounding rules | Base-10 arithmetic matches pencil-and-paper decimal rules |
| Database NUMERIC(p,s) types lose precision in C++ computation | `dfixpnt<p, s>` preserves the exact precision and scale |
| Overflow in financial totals could produce garbage | `Saturate` mode clamps to safe bounds instead of wrapping |
| Need IEEE 754-2008 DPD-compatible decimal storage | `DecimalEncoding::DPD` uses the standard declet encoding |
