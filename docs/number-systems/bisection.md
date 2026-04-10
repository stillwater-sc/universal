# Bisection: Universal Coding of the Reals

## Why

Every classical number system -- IEEE floating-point, posits, LNS, fixed-point -- uses a different encoding scheme with different structural choices (exponent width, regime length, base). This makes it hard to compare them fairly or to prototype new formats.

The bisection framework (Lindstrom, CoNGA'19) unifies all these systems under a single abstraction: every number system is defined by just two functions -- a **generator** `g(x)` that produces a bracketing sequence and a **refinement** `f(a,b)` that bisects a bounded interval. The binary encoding is the sequence of comparison outcomes during the bisection. This lets you:

- **Rapidly prototype** a new number system with two lines of code
- **Compare** dynamic range, precision density, and rounding across systems under identical conditions
- **Explore** novel systems (Fibonacci, golden ratio, NaturalPosit) that do not fit any classical encoding scheme

## What

`bisection<Generator, Refinement, nbits, bt, AuxReal>` is a configurable number type:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `Generator` | typename | -- | Callable producing the bracketing sequence g(x) |
| `Refinement` | typename | -- | Callable bisecting a bounded interval f(a,b) |
| `nbits` | `unsigned` | -- | Total bits including sign (2 to 64) |
| `bt` | typename | `uint8_t` | Block storage type |
| `AuxReal` | typename | `double` | Auxiliary real type for the bisection arithmetic |

### Encoding

The encoding is a two's complement integer produced by binary search:

```
1. Start with interval (-inf, +inf)
2. For each bit (MSB to LSB):
   - Compute bisection point x_m using generator + refinement rules
   - If x >= x_m: emit 1, narrow interval to [x_m, x_h)
   - Else:        emit 0, narrow interval to [x_l, x_m)
3. Round to nearest at the final step
4. Flip the MSB so that encoding 0 maps to value 0
```

The result is a **monotone encoding**: x < y implies bits(x) < bits(y) (as signed integers). Comparisons on the encoding directly correspond to comparisons on the reals.

### Bisection Rules

The bisection point is computed from the interval [x_l, x_h] using a fixed set of rules applied in order:

| Rule | Condition | Bisection point |
|------|-----------|-----------------|
| Bootstrap | (-inf, +inf) | 0 |
| Bootstrap | (-inf, 0) | -1 |
| Bootstrap | (0, +inf) | 1 |
| Negation | x_h < 0 | -f(-x_h, -x_l) |
| Reciprocation | x_l = 0 | 1 / f(1/x_h, +inf) |
| Bracketing | x_h = +inf | g(x_l) |
| Refinement | finite [a, b] | f(a, b) |

The generator defines the coarse structure (dynamic range), and the refinement defines the fine structure (precision within a binade).

### Pre-built Generators

| Alias | Generator | Refinement | Character |
|-------|-----------|------------|-----------|
| `bisection_unary<p>` | g(x) = x + 1 | ArithmeticMean | Linear bracketing, uniform density |
| `bisection_posit<p, m>` | g(x) = 2^(2^m) * x | HyperMean | Matches posit(m) encoding |
| `bisection_natposit<p, m>` | g(x) = 2^(2^m) * x | NaturalPositRefinement | Posit without wobbling accuracy |
| `bisection_fibonacci<p>` | g(x) = round(phi*x) | ArithmeticMean | Fibonacci-coded bracketing |
| `bisection_golden<p>` | g(x) = phi * x | ArithmeticMean | Golden ratio base |
| `bisection_elias_delta<p>` | g(x) = 2x^2 | HyperMean | Super-exponential range |
| `bisection_elias_gamma<p>` | g(x) = 2x | HyperMean | Exponential (base-2) |
| `bisection_elias_omega<p>` | g(x) = 2^x | HyperMean | Tetration-level range |
| `bisection_lns<p>` | g(x) = 2x | GeometricMean | LNS-like (1-bit exponent) |
| `bisection_lns_m<p, m>` | g(x) = 2^(2^(m-1)) * sqrt(x) | GeometricMean | LNS(m) |
| `bisection_urr<p>` | g(x) = max(2, x^2) | HyperMean | Hamada's URR |

### Refinement Functions

| Refinement | Formula | Used by |
|------------|---------|---------|
| ArithmeticMean | (a+b)/2 | Unary, Fibonacci, Golden |
| GeometricMean | sqrt(a*b) | LNS, LNS(m) |
| HyperMean | arithmetic within binade, geometric across | Posit, Elias, URR |
| NaturalPositRefinement | power mean M_p, p = -2^(-m) | NaturalPosit |

### AuxReal: High-Precision Auxiliary Type

The `AuxReal` parameter controls the arithmetic precision of the bisection search itself. For `nbits` up to about 50, `double` (~16 significant digits) is sufficient. For wider encodings, use `dd` (~31 digits) or `qd` (~63 digits):

```cpp
using precise_posit32 = bisection_posit<32, 0, uint8_t, dd>;
```

This gives tighter round-trip accuracy without changing the encoding semantics.

## How

### Include

```cpp
#include <universal/number/bisection/bisection.hpp>
```

### Basic Usage

```cpp
using namespace sw::universal;

// 16-bit posit-like bisection type
using bposit16 = bisection_posit<16>;

bposit16 a(3.14159), b(2.71828);
bposit16 c = a * b;
std::cout << c << std::endl;              // 8.53973
std::cout << to_binary(c) << std::endl;   // 0b0111'0000'1010'0011
std::cout << type_tag(c) << std::endl;    // bisection<16>
```

### Comparing Number Systems

```cpp
// Same input, different generators
double x = 0.1;
bisection_posit<8>  bp(x);  // posit-like encoding
bisection_golden<8> bg(x);  // golden ratio encoding
bisection_unary<8>  bu(x);  // unary encoding

std::cout << "posit:  " << double(bp) << "  " << to_binary(bp) << "\n";
std::cout << "golden: " << double(bg) << "  " << to_binary(bg) << "\n";
std::cout << "unary:  " << double(bu) << "  " << to_binary(bu) << "\n";
```

### Designing a Custom Number System

Define a generator and refinement, then instantiate:

```cpp
// Custom generator: g(x) = 3x (tripling sequence)
struct TriplingGenerator {
    template<typename R>
    R operator()(const R& x) const { return R(3) * x; }
};

// Use geometric mean for logarithmic-style refinement
using my_type = bisection<TriplingGenerator, GeometricMean, 16>;

my_type v(42.0);
std::cout << double(v) << std::endl;
```

## Properties

### Dynamic Range

Dynamic range depends on the generator's growth rate:

| Generator | Growth | 8-bit range (decades) | 16-bit range |
|-----------|--------|--------------------|--------------|
| Unary | Linear | ~1.5 | ~4.4 |
| Posit(0) | 2x | ~4.2 | ~8.4 |
| Posit(1) | 4x | ~8.4 | ~16.8 |
| Elias delta | 2x^2 | ~33 | ~130 |
| Elias omega | 2^x | ~9800 | enormous |

### Precision Density

Precision is inversely related to dynamic range. Systems with larger dynamic range have fewer representable values per decade near 1.0, while those with smaller range pack more precision near 1.0.

The NaturalPosit refinement (`bisection_natposit`) eliminates the "wobbling accuracy" phenomenon observed in standard posits, providing a smoother precision profile.

## Reference

Peter Lindstrom. 2019. Universal Coding of the Reals using Bisection. In Conference on Next Generation Arithmetic (CoNGA'19). DOI: 10.1145/3316279.3316286
