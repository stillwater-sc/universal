# Posit: Tapered Floating-Point (UNUM Type III)

## Why

IEEE-754 floating-point distributes its precision uniformly across the entire exponent range. A 32-bit float has the same number of fraction bits whether the value is 10^-30 or 10^30. But most real-world computations cluster their values near 1.0 -- neural network weights, probabilities, normalized sensor readings, physical quantities in natural units. IEEE-754 wastes precision on extreme exponents that are rarely used while providing no more precision where it matters most.

Posits, invented by John Gustafson as UNUM Type III, solve this with **tapered precision**: values near 1.0 get the most fraction bits (highest precision), while values at the extremes of the range trade fraction bits for exponent bits (widest range). The result is a format that provides more useful precision than IEEE-754 at the same bit width, with a cleaner encoding that eliminates the redundancy and complexity of NaN, denormals, and signed zeros.

## What

`posit<nbits, es, bt>` is a tapered-precision floating-point:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Total bits (2 to 256+) |
| `es` | `unsigned` | -- | Maximum exponent bits (0 to 9+) |
| `bt` | typename | `uint8_t` | Storage block type |

### Encoding

A posit encodes four fields in a variable-length format:

```
[sign : 1] [regime : variable] [exponent : ≤ es] [fraction : remaining]
```

- **Sign**: 0 = positive, 1 = negative (2's complement for the entire encoding)
- **Regime**: run-length encoded scale factor
  - Run of k ones followed by a zero: regime value = k (scale = useed^k)
  - Run of k zeros followed by a one: regime value = -k (scale = useed^(-k))
- **Exponent**: up to `es` bits of binary exponent
- **Fraction**: remaining bits encode the significand

The **useed** = 2^(2^es) is the regime scale factor:
- `es=0`: useed = 2 (each regime step doubles/halves)
- `es=1`: useed = 4
- `es=2`: useed = 16
- `es=3`: useed = 256

### Tapered Precision

For values near 1.0, the regime is short (1-2 bits), leaving most bits for fraction (high precision). For extreme values, the regime consumes most bits, providing enormous dynamic range at the cost of precision:

```
posit<32, 2>:
  Near 1.0:  regime=2 bits, exponent=2 bits, fraction=27 bits  (~8 decimal digits)
  Near max:  regime=29 bits, exponent=2 bits, fraction=0 bits  (range only)
```

### Key Properties

- **Two special values only**: all-zeros = 0, MSB-only = NaR (Not a Real)
- **No NaN, no signed zero, no denormals**: cleaner than IEEE-754
- **Reciprocal symmetry**: posit(x) and posit(1/x) have the same precision
- **Exact integers**: small integers near 1.0 are represented exactly
- **More useful precision**: typically 1-2 extra decimal digits vs same-size IEEE float
- **Fused operations**: atomic fma(), fam(), fmma() using blocktriple intermediate

### Dynamic Range

| Configuration | Dynamic Range |
|--------------|---------------|
| `posit<8, 0>` | 2^(-6) to 2^6 |
| `posit<16, 1>` | 2^(-28) to 2^28 |
| `posit<32, 2>` | 2^(-120) to 2^120 |
| `posit<64, 3>` | 2^(-496) to 2^496 |

## How It Works

The regime field is the key innovation. It uses run-length encoding to create a variable-width exponent:

1. **Decode regime**: count the run of identical bits after the sign bit
2. **Compute regime scale**: useed^regime_value
3. **Decode fixed exponent**: next `es` bits (if available)
4. **Decode fraction**: remaining bits form the significand with an implicit leading 1

The value is: `(-1)^sign × useed^regime × 2^exponent × (1 + fraction)`

This variable-length structure is what creates tapered precision: when the regime is short (value near 1), more bits remain for fraction; when the regime is long (extreme value), precision is sacrificed for range.

Arithmetic uses a `blocktriple` intermediate representation that carries sufficient precision for exact computation before rounding back to the target posit format.

## How to Use It

### Include

```cpp
#include <universal/number/posit/posit.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
posit<32, 2> a(3.14159265358979);
posit<32, 2> b(2.71828182845905);
posit<32, 2> c = a * b;

std::cout << "pi * e = " << c << std::endl;
std::cout << "encoding: " << to_binary(c) << std::endl;
std::cout << "components: " << components(c) << std::endl;
```

### Comparing Precision with IEEE-754

```cpp
// posit<32,2> has more useful precision than float32 near 1.0
float f = 1.0f;
posit<32, 2> p(1.0);

for (int i = 0; i < 10; ++i) {
    f += 1.0e-7f;
    p += posit<32, 2>(1.0e-7);
}
// posit preserves more digits near 1.0 due to tapered precision
```

### Mixed-Precision Deep Learning

```cpp
template<typename Real>
Real sigmoid(Real x) {
    return Real(1) / (Real(1) + exp(-x));
}

// 8-bit posit for inference: more precision near 0-1 range than fp8
using Inference = posit<8, 0>;
Inference weight(0.5);
Inference input(0.3);
Inference output = sigmoid(weight * input);
```

### Plug-in Replacement

```cpp
template<typename Real>
Real newton_sqrt(Real x, int iterations = 10) {
    Real guess = x / Real(2);
    for (int i = 0; i < iterations; ++i) {
        guess = (guess + x / guess) / Real(2);
    }
    return guess;
}

// Compare convergence: posit vs float
auto sqrt_f = newton_sqrt(2.0f);
auto sqrt_p = newton_sqrt(posit<32, 2>(2.0));
// posit converges to more accurate result at same bit width
```

### Quire: Exact Dot Products

```cpp
#include <universal/number/posit/posit.hpp>
// The quire provides exact accumulation with no intermediate rounding
using Posit = posit<32, 2>;

// Fused dot product: exact accumulation in quire, single rounding at the end
Posit a[] = { Posit(1.0), Posit(2.0), Posit(3.0) };
Posit b[] = { Posit(4.0), Posit(5.0), Posit(6.0) };

quire<32, 2> q;
for (int i = 0; i < 3; ++i) {
    q += quire_mul(a[i], b[i]);  // No rounding between multiplications
}
Posit dot_product;
convert(q.to_value(), dot_product);  // Single rounding at the end
```

### Math Library

The posit standard requires its functions to be correctly rounded for every argument, so that results
are reproducible bit for bit. The functions in the table below are correctly rounded at every `nbits`
(the transcendental ones by a bounded test, described below), including some the standard does not
require, such as the error and gamma functions. None of their results depends on a platform's libm.
For `nbits <= 64`, `sqrt` uses `double` for the seed of its integer square root and for one
correction estimate, and an exact fix-up then decides the root (#1855). `ipow(a, b)` in
`posit/mathlib.hpp` is a separate integer-only helper on `uint64_t`; `pow(a, int)` is the correctly
rounded integer power.

| functions | how |
|---|---|
| `sqrt` | the integer square root of the significand, exact remainder as the sticky bit (#1856, #1857) |
| `trunc`, `round`, `floor`, `ceil` | exact, on the encoding |
| `fmod`, `remainder`, `frac` | exact in efloat, rounded once (a posit's taper can make `x - n y` inexact) |
| `exp`, `exp2`, `exp10`, `expm1`, `log`, `log2`, `log10`, `log1p` | efloat, rounded once |
| `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `cot`, `sec`, `csc` | efloat, exact argument reduction at any magnitude |
| `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh` | efloat, rounded once |
| `pow` (posit, `int` and `double` exponents), `hypot` (2 and 3 arguments) | efloat, rounded once |
| `erf`, `erfc`, `tgamma`, `lgamma` | efloat, rounded once |

The transcendental functions are [efloat](/universal/number-systems/efloat/)'s correctly rounded
functions. A value rounded once in efloat and again into the posit is rounded twice, so the posit
side applies Ziv's test. The efloat value at W bits is within 1/2 ulp of the exact result, so when
it lies more than 1 ulp from every posit tie point it rounds to the correct posit. When it doesn't,
W doubles. The first W carries 30 bits past the posit's fraction, so a retry happens about once
in 2^28 calls. The retries stop at 8 times the first W. A result still unresolved there is rounded
as it is, which is right for an exact result on a tie point, since efloat forms exact results
exactly. An inexact result that close to a tie, within about 2^-(8 W) relative, is not expected
(the hardest cases known for such functions lie near 2^-(2 p)), but nothing proves it cannot occur.
So the transcendental functions are designed to be correctly rounded, and are verified rather than
proven; the exact operations and `sqrt` are correctly rounded by construction.

```cpp
#include <universal/number/posit/posit.hpp>
using namespace sw::universal;

posit<128, 4> x(0.5);
auto y = exp(x);        // correctly rounded to all 121 fraction bits posit<128,4> has near 1
auto z = sin(x * 1e30); // exact argument reduction, at any magnitude
auto g = tgamma(posit<128, 4>(200.5));  // about 1e373: past double's range, inside the posit's
```

Special values follow the posit rules. NaR in gives NaR out. A pole (`log(0)`, `atanh(1)`, `tgamma(0)`,
`cot(0)`, `pow(0, -1)`) and a domain error (`asin(2)`, `log(-1)`, `pow(-2, 0.5)`) give NaR, which
takes the place of IEEE-754's infinities and NaNs. A result past the posit's range saturates at
`maxpos` or `minpos` with its sign, since a posit never rounds a nonzero value to 0 or to NaR:
`exp(maxpos)` is `maxpos` and `exp(-maxpos)` is `minpos`.

Correct rounding has a price at narrow widths, where the old `double` path was fast and usually,
though not always, right. Measured in nanoseconds per call on an i7-12700K, with g++ 15 at `-O3`:

| | posit<16,1> | posit<32,2> | posit<64,3> | posit<128,4> | posit<256,5> |
|---|---:|---:|---:|---:|---:|
| `exp` | 655 | 662 | 710 | 850 | 1203 |
| `sin` | 917 | 944 | 1094 | 1279 | 1635 |
| `pow` | 1452 | 1498 | 1635 | 1868 | 2438 |
| `hypot` | 191 | 195 | 247 | 322 | 466 |

The regression tests in `static/tapered/posit/math` run exhaustively at narrow widths, randomly up
to 256 bits, and on exact values, poles and saturation. Each family has its own reference:

- `sqrt.cpp` uses the rounding rule itself, in integers: the squares of the neighbouring tie
  points bracket the argument.
- `truncate.cpp` compares with the `double` functions up to 32 bits, where they are exact, and past
  that checks the defining properties.
- `fractional.cpp` compares with `std::fmod` and `std::remainder` in `double` up to 32 bits, and past
  that with constructed exact cases.
- `elementary.cpp` compares the transcendental functions with the same efloat function at
  4 (fbits + 1) + 64 bits, rounded once, and widens that reference near a tie point. This tests
  the posit side, meaning the rounding, the retries and the special values, rather than efloat's
  functions, which `benchmark/accuracy/efloat/mpfr_check.cpp` verifies against MPFR.
- `efloat_bridge.cpp` checks the posit/efloat conversions against the posit tie points.

## Problems It Solves

| Problem | How posit Solves It |
|---------|-----------------------|
| IEEE float wastes precision on extreme values | Tapered precision: most bits near 1.0 where values cluster |
| NaN propagation corrupts entire computation | Only one non-real value (NaR), explicit and predictable |
| Signed zero and multiple NaN encodings waste bit patterns | Two special values only: 0 and NaR |
| 8-bit float has too little precision for ML inference | posit<8,0> has more precision near 0-1 than fp8 |
| Dot product rounding errors accumulate | Quire provides exact accumulation, single final rounding |
| Need better precision without increasing bit width | posit<32,2> typically provides 1-2 more decimal digits than float32 |
| Reproducible linear algebra across platforms | Quire + posit = same result on every platform |
| Math functions that differ between platforms' libm | The posit math functions are correctly rounded, and none of their results depends on libm |
