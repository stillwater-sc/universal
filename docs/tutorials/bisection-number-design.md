# Designing Custom Number Systems with the Bisection Framework

This tutorial shows how to use the bisection coding framework to design, prototype, and evaluate a custom number system in minutes. Instead of implementing a complete encoder/decoder from scratch, you write two small functions and the framework does the rest.

## Prerequisites

- C++20 compiler
- Universal library installed or available via include path
- Basic understanding of number system concepts (precision, dynamic range)

## The Core Idea

Every number system maps bit strings to real numbers. The bisection framework performs this mapping through **binary search**: starting from the interval (-inf, +inf), each bit narrows the interval by choosing one half. After `p` bits, the remaining interval defines the encoded value.

You control the mapping through two functions:

1. **Generator g(x)**: defines the coarse structure. Starting from 1, the sequence 1, g(1), g(g(1)), ... determines the "binades" -- the exponential-scale boundaries.

2. **Refinement f(a,b)**: defines the fine structure. Within each binade, f(a,b) determines how precision is distributed between a and b.

## Step 1: Choose a Generator

The generator determines your dynamic range and how it scales:

```cpp
// Linear: g(x) = x + 1 -> sequence 1, 2, 3, 4, ...
// Tiny dynamic range, maximum precision near 1
struct LinearGenerator {
    template<typename R>
    R operator()(const R& x) const { return x + R(1); }
};

// Doubling: g(x) = 2x -> sequence 1, 2, 4, 8, ...
// Each binade covers one octave (factor of 2)
struct DoublingGenerator {
    template<typename R>
    R operator()(const R& x) const { return R(2) * x; }
};

// Squaring: g(x) = x^2 -> sequence 1, 2, 4, 16, 256, ...
// Enormous dynamic range, sparse near extremes
struct SquaringGenerator {
    template<typename R>
    R operator()(const R& x) const { return x * x; }
};
```

**Rule of thumb**: faster-growing generators give wider dynamic range but sparser precision at the extremes. Slower generators pack more precision near 1 but run out of range quickly.

## Step 2: Choose a Refinement

The refinement determines how values within each binade are spaced:

```cpp
// Arithmetic mean: f(a,b) = (a+b)/2
// Uniform spacing -- like fixed-point within each binade
// Already provided: sw::universal::ArithmeticMean

// Geometric mean: f(a,b) = sqrt(a*b)
// Logarithmic spacing -- like LNS within each binade
// Already provided: sw::universal::GeometricMean

// Your own: any callable with signature R f(R, R)
struct WeightedMean {
    template<typename R>
    R operator()(const R& a, const R& b) const {
        // Bias precision toward smaller values (3:1 split)
        return (R(3) * a + b) / R(4);
    }
};
```

**Rule of thumb**: arithmetic mean gives uniform precision within each binade. Geometric mean concentrates precision near the lower bound of each binade (good for logarithmic representations).

## Step 3: Instantiate and Test

```cpp
#include <universal/number/bisection/bisection.hpp>
#include <iostream>

using namespace sw::universal;

// A custom type: doubling generator with weighted refinement, 12 bits
using custom12 = bisection<DoublingGenerator, WeightedMean, 12>;

int main() {
    custom12 pi(3.14159265);
    custom12 e(2.71828183);

    std::cout << "pi  = " << double(pi) << "  " << to_binary(pi) << "\n";
    std::cout << "e   = " << double(e)  << "  " << to_binary(e) << "\n";
    std::cout << "pi*e= " << double(pi * e) << "\n";

    // Walk all representable values
    for (int i = 0; i < (1 << 12); ++i) {
        custom12 v;
        v.setbits(static_cast<uint64_t>(i));
        if (v.isnan()) continue;
        // ... analyze distribution, precision, dynamic range
    }
}
```

## Step 4: Evaluate Properties

Key properties to measure for your custom type:

### Dynamic Range

```cpp
custom12 maxv, minv;
maxv.maxpos();
minv.minpos();
double range = std::log10(double(maxv) / double(minv));
std::cout << "Dynamic range: " << range << " decades\n";
```

### Precision Profile

Walk all positive representable values and compute the gap between consecutive values:

```cpp
double prev = 0.0;
for (int i = 1; i < (1 << 12); ++i) {
    custom12 v;
    v.setbits(static_cast<uint64_t>(i));
    if (v.isnan() || double(v) <= 0.0) continue;
    double cur = double(v);
    double rel_gap = (cur - prev) / cur;  // relative precision
    std::cout << cur << "  gap=" << rel_gap << "\n";
    prev = cur;
}
```

### Compare Against Standard Types

Use the bisection REPL tool to compare interactively:

```bash
bisection "compare pi"
```

Or register your custom type in the REPL by adding it to `tools/bisection/bisection.cpp`.

## Step 5: Use with Higher Precision

For wider types (> 50 bits), the default `double` auxiliary may not provide enough precision for the binary search to converge accurately. Use `dd` or `qd`:

```cpp
using custom64_dd = bisection<DoublingGenerator, WeightedMean, 64, uint8_t, dd>;
```

This requires `#include <universal/number/dd/dd.hpp>`.

## Built-in Generator/Refinement Pairs

The library includes all generators and refinements from the Lindstrom paper:

| Generator | Function | Use case |
|-----------|----------|----------|
| `UnaryGenerator` | x + 1 | Maximum precision, minimal range |
| `EliasGammaGenerator` | 2x | Balanced (LNS-like exponent) |
| `EliasDeltaGenerator` | 2x^2 | Wide range |
| `PositGenerator<m>` | 2^(2^m) * x | Posit-compatible |
| `FibonacciGenerator` | round(phi*x) | Fibonacci coding |
| `GoldenRatioGenerator` | phi * x | Golden ratio base |
| `EliasOmegaGenerator` | 2^x | Enormous range |
| `URRGenerator` | max(2, x^2) | Hamada's URR |
| `LNSGenerator<m>` | 2^(2^(m-1)) * sqrt(x) | LNS(m) |

| Refinement | Formula | Character |
|------------|---------|-----------|
| `ArithmeticMean` | (a+b)/2 | Uniform spacing |
| `GeometricMean` | sqrt(a*b) | Logarithmic spacing |
| `HyperMean` | arith within binade, geom across | Posit-like adaptive |
| `NaturalPositRefinement<m>` | power mean M_{-2^(-m)} | Smooth posit (no wobble) |

## Reference

Peter Lindstrom. 2019. Universal Coding of the Reals using Bisection. CoNGA'19. DOI: 10.1145/3316279.3316286

See also: `docs/number-systems/bisection.md` for the full type reference.
