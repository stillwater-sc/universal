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
