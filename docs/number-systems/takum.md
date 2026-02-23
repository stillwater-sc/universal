# Takum: Bounded-Range Tapered Floating-Point

## Why

Posits provide tapered precision with a run-length encoded regime that creates enormous dynamic range -- a 32-bit posit can represent values from 2^-120 to 2^120. But this range is far more than most applications need. Numbers like 10^-80 or 10^80 arise almost exclusively in overflow/underflow scenarios, not in meaningful computation. Every bit spent encoding extreme exponents is a bit *not* spent on precision where it matters.

Takum (Icelandic: *takmarkao umfang* = "limited range") addresses this by replacing the posit's unbounded run-length regime with a bounded 3-bit regime that caps the dynamic range at practical limits. The bits saved from exponent encoding go directly to fraction bits, giving takum more precision than a same-size posit for values in the normal computational range.

## What

`takum<nbits, bt>` is a bounded-range tapered floating-point:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Total bits |
| `bt` | typename | `uint8_t` | Storage block type |

Note: unlike posit, takum has **no `es` parameter** -- the exponent structure is fixed by the 3-bit regime.

### Encoding

```
[sign : 1] [regime : 3 bits] [direction : 1 bit] [exponent : regime+1 bits] [fraction : remaining]
```

- **Regime (3 bits)**: encodes how many exponent bits follow (0-7)
- **Direction bit**: determines exponent sign/direction
- **Exponent**: variable-length (regime value + 1 bits)
- **Fraction**: remaining bits (with implicit leading 1 for most regime values)

### Key Differences from Posit

| Property | Posit<32,2> | Takum<32> |
|----------|-------------|-----------|
| Dynamic range | ~2^240 | ~2^64 |
| Max precision (near 1.0) | ~27 fraction bits | ~25 fraction bits |
| Range growth with nbits | Exponential (2^(nbits × 2^es)) | Logarithmic (bounded) |
| Regime encoding | Run-length (variable) | Fixed 3-bit |
| Exponent parameter | es (configurable) | Fixed by regime |

### Design Goals

1. **More effective dynamic range**: don't waste bits on 10^-80
2. **Bounded range**: dynamic range grows logarithmically, not exponentially
3. **Symmetric exponents**: largest and smallest representable exponents are equal in magnitude
4. **Predictable precision**: 3-bit regime has bounded overhead

## How It Works

The 3-bit regime field directly encodes the number of exponent bits minus one:
- Regime = 0: 1 exponent bit follows
- Regime = 1: 2 exponent bits follow
- Regime = 7: 8 exponent bits follow

The direction bit determines whether the exponent is positive (large numbers) or negative (small numbers). This creates a bounded dynamic range that grows logarithmically with nbits, in contrast to posit's exponential growth.

For values near 1.0, the regime is small and few exponent bits are needed, leaving most bits for the fraction (high precision). For extreme values, more exponent bits are used but the range is bounded -- you never waste more than 8 exponent bits.

## How to Use It

### Include

```cpp
#include <universal/number/takum/takum.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
takum<32> a(3.14159);
takum<32> b(2.71828);
takum<32> c = a * b;

std::cout << "pi * e = " << c << std::endl;
std::cout << "encoding: " << to_binary(c) << std::endl;
```

### Comparing with Posit

```cpp
// Show that takum trades extreme range for more precision
posit<32, 2> p_max;
p_max.maxpos();
takum<32> t_max;
t_max.maxpos();

std::cout << "posit<32,2> maxpos: " << p_max << std::endl;  // ~10^36
std::cout << "takum<32>   maxpos: " << t_max << std::endl;  // ~10^19

// But takum has more fraction bits for normal-range values
posit<32, 2> p(1.23456789012345);
takum<32>    t(1.23456789012345);
std::cout << "posit precision: " << p << std::endl;
std::cout << "takum precision: " << t << std::endl;
// takum preserves more digits for values in normal range
```

### General-Purpose Computing

```cpp
template<typename Real>
Real compute_statistics(const std::vector<Real>& data) {
    Real sum(0), sum_sq(0);
    Real n(data.size());
    for (const auto& x : data) {
        sum += x;
        sum_sq += x * x;
    }
    Real mean = sum / n;
    Real variance = sum_sq / n - mean * mean;
    return variance;
}

// Takum: enough range for normal computation, more precision than posit
std::vector<takum<32>> data = { takum<32>(1.5), takum<32>(2.3), takum<32>(1.8) };
auto var = compute_statistics(data);
```

## Problems It Solves

| Problem | How takum Solves It |
|---------|-----------------------|
| Posit wastes bits encoding 10^-80 to 10^80 | Bounded 3-bit regime caps range at practical limits |
| Need more precision at same bit width | Saved exponent bits become fraction bits |
| Hardware design needs predictable accumulator width | Bounded range = bounded accumulator, unlike posit |
| Dynamic range grows faster than useful | Logarithmic growth with nbits instead of exponential |
| Exponent encoding overhead varies unpredictably | Fixed 3-bit regime = predictable overhead |
| General-purpose computing doesn't need extreme range | Bounded range matches real-world value distributions |
