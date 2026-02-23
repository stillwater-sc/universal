# Rational: Exact Fractional Arithmetic

## Why

Floating-point numbers cannot represent 1/3, 1/7, or 1/10 exactly. Every operation on these values introduces rounding error that accumulates over long computations. For symbolic computation, exact geometric reasoning, or any algorithm where you need the mathematical property that *a/b is represented precisely as a/b*, floating-point is fundamentally unsuitable.

The Universal `rational` type stores numbers as explicit numerator/denominator pairs, automatically reduced to lowest terms. Arithmetic on rationals is closed: adding, subtracting, multiplying, or dividing two rationals always produces another rational, with no rounding error whatsoever (within the bit-width constraint).

## What

`rational<nbits, BlockType>` stores a fraction as two signed integers:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Bits per component (numerator and denominator each use `nbits` bits) |
| `BlockType` | typename | `uint8_t` | Storage limb type |

### Key Properties

- **Exact representation**: value = numerator / denominator, always in reduced form
- **Automatic normalization**: GCD reduction after every operation
- **Arithmetic closure**: +, -, *, / all produce exact rational results
- **Total storage**: 2 x `nbits` bits (e.g., `rational<32>` uses 64 bits)
- **Trivially copyable**: fixed-size, no heap allocation
- **No NaN, no infinity**: only finite rational numbers

### Standard Type Aliases

```cpp
using rb8   = rational<8, uint8_t>;     // 16 bits total
using rb16  = rational<16, uint16_t>;   // 32 bits total
using rb32  = rational<32, uint32_t>;   // 64 bits total
using rb64  = rational<64, uint64_t>;   // 128 bits total
using rb128 = rational<128, uint64_t>;  // 256 bits total
```

### Ranges

| Type | Min Positive | Max Positive |
|------|-------------|-------------|
| `rb8` | 1/128 | 127 |
| `rb16` | ~3.05e-5 | 32,767 |
| `rb32` | ~4.66e-10 | ~2.1e9 |
| `rb64` | ~1.08e-19 | ~9.2e18 |

## How It Works

Each rational stores two `blockbinary<nbits, BlockType, Signed>` values: the numerator `n` and denominator `d`. The value is `n/d`.

After every arithmetic operation, the result is normalized by dividing both components by their GCD:
- Addition: `a/b + c/d = (a*d + c*b) / (b*d)`, then normalize
- Multiplication: `a/b * c/d = (a*c) / (b*d)`, then normalize
- Division: `a/b / c/d = (a*d) / (b*c)`, then normalize

This normalization prevents numerator/denominator growth from consuming all available bits, but it does mean that intermediate results can overflow if the un-normalized product exceeds the nbits range.

## How to Use It

### Include

```cpp
#include <universal/number/rational/rational.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
// Exact representation of 1/3
rational<32> one_third;
one_third.set(1, 3);  // Exactly 1/3, no rounding

// Arithmetic is exact
rational<32> two_thirds = one_third + one_third;  // 2/3 exactly
rational<32> one = two_thirds + one_third;         // 1/1 exactly

// Compare: float(1.0f/3.0f) * 3.0f != 1.0f
// But:     rational(1,3) * rational(3,1) == rational(1,1)
```

### Exact Geometric Computation

```cpp
template<typename Number>
Number triangle_area(Number x1, Number y1, Number x2, Number y2,
                     Number x3, Number y3) {
    // Shoelface formula: exact when using rationals
    return (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / Number(2);
}

using Q = rational<64, uint64_t>;
Q area = triangle_area(Q(0), Q(0), Q(1), Q(0), Q(0), Q(1));
// area = 1/2 exactly -- no floating-point error
```

### Exact Summation

```cpp
// Harmonic series: 1 + 1/2 + 1/3 + ... + 1/n
rational<128, uint64_t> harmonic(0);
for (int k = 1; k <= 20; ++k) {
    rational<128, uint64_t> term;
    term.set(1, k);
    harmonic += term;
}
// Result is the exact rational value, not a floating-point approximation
```

## Problems It Solves

| Problem | How rational Solves It |
|---------|-----------------------|
| 1/3 cannot be represented in binary floating-point | Explicit numerator/denominator: 1/3 is exact |
| Accumulated rounding error in long computations | Every operation is exact (within bit-width) |
| Geometric predicates (point-in-polygon, orientation) fail with float | Exact integer arithmetic on rational coordinates |
| Symbolic computation needs exact fractions | Closed-form arithmetic: rationals in, rationals out |
| Financial interest calculations compound rounding errors | Exact fractional interest rates |
| Unit conversion chains accumulate error | Exact conversion factors (e.g., 5/9 for Fahrenheit to Celsius) |
