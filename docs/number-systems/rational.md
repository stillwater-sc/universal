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

Each component (numerator and denominator) is a signed `nbits`-bit integer
with range `-(2^(nbits-1))` to `2^(nbits-1) - 1`. The smallest positive
rational is `1 / (2^(nbits-1) - 1)` and the largest is `(2^(nbits-1) - 1) / 1`.

For example, `rb128 = rational<128>` has 128-bit signed components:
- Min positive = 1 / (2^127 - 1) ~= 5.88e-39
- Max positive = (2^127 - 1) / 1 ~= 1.70e38

| Type | Min Positive | Max Positive |
|------|-------------|-------------|
| `rb8` | 1/127 ~= 7.87e-3 | 127 |
| `rb16` | 1/32767 ~= 3.05e-5 | 32,767 |
| `rb32` | 1/(2^31-1) ~= 4.66e-10 | ~2.15e9 |
| `rb64` | 1/(2^63-1) ~= 1.08e-19 | ~9.22e18 |
| `rb128` | 1/(2^127-1) ~= 5.88e-39 | ~1.70e38 |

## How It Works

Each rational stores two `blockbinary<nbits, BlockType, Signed>` values: the numerator `n` and denominator `d`. The value is `n/d`.

Every arithmetic operation computes its exact result in a wide integer type that holds every product and sum of two operands, then reduces it by the GCD:
- Addition: `a/b + c/d = (a*d + c*b) / (b*d)`
- Multiplication: `a/b * c/d = (a*c) / (b*d)`
- Division: `a/b / c/d = (a*d) / (b*c)`

The reduced result is then fitted back into the type (#2132):
- **It fits:** stored exactly. A binary rational stores a magnitude of `2^(nbits-1)` with the sign in the other field: in a `rational<4>`, 8/7 is -8/-7.
- **Past the range:** **saturates** at `maxpos` or `maxneg`. Fixed-width rational arithmetic never wraps: in `rational<8>`, `100 + 100` is `maxpos` (127), not -56.
- **Inside the range, but the reduced numerator or denominator does not fit:** the **closest** rational the type holds, the same as a conversion from a native float.
- **A zero divisor:** NaN (0/0), in every base. With `RATIONAL_THROW_ARITHMETIC_EXCEPTION`, it throws `rational_divide_by_zero` instead.

Comparisons are exact: `a/b` against `c/d` cross-multiplies in the same wide type, so `2^53 + 1` and `2^53` are different values. A NaN is unordered: `==` and `<` are false, `!=` is true.

`SpecificValue` constructors give the named values in every base. There is no infinity: `infpos`/`infneg` saturate at `maxpos`/`maxneg`, and `numeric_limits::infinity()` is `max()`.

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
