# DBNS: Double-Base Number System

## Why

The standard Logarithmic Number System (LNS) uses a single base (typically 2) to encode values. This means all representable values are of the form `±2^e`, and the spacing between representable values is determined entirely by the resolution of the binary exponent. For some applications, this spacing is suboptimal -- values cluster too densely at some scales and too sparsely at others.

The Double-Base Number System uses *two* bases simultaneously: values are represented as `±(0.5)^a × 3^b`, where `a` and `b` are independent integer exponents. This two-dimensional encoding space creates a more flexible distribution of representable values on the number line. The dual bases can achieve finer granularity in certain ranges than a single-base system of the same bit width, particularly for applications where mixed-radix representations arise naturally.

## What

`dbns<nbits, fbbits, bt, xtra>` is a dual-base logarithmic number:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Total bits |
| `fbbits` | `unsigned` | -- | Bits for first base (0.5) exponent |
| `bt` | typename | `uint8_t` | Storage block type |
| `xtra` | variadic | -- | Arithmetic behavior |

### Value Representation

```
value = ±(0.5)^a × 3^b
```

where:
- `a` uses `fbbits` bits (first base: 0.5 = 2^-1)
- `b` uses `sbbits = nbits - fbbits - 1` bits (second base: 3)
- 1 bit for sign

### Key Properties

- **Two independent bases**: 0.5 (binary) and 3 (ternary)
- **Two-dimensional exponent space**: finer value granularity than single-base LNS
- **Multiplication is addition**: `x × y → (a_x+a_y, b_x+b_y)` (add both exponent pairs)
- **Division is subtraction**: `x / y → (a_x-a_y, b_x-b_y)` (subtract both exponent pairs)
- **Saturation mode**: configurable overflow handling
- **Unity approximations**: certain (a,b) pairs closely approximate 1.0

### Unity Approximations

The key mathematical relationship is `2^(-log2(3)) ≈ 3^(-1)`, which means:
- `(0.5)^3 × 3^2 = 0.125 × 9 = 1.125 ≈ 1`
- `(0.5)^8 × 3^5 = 0.00390625 × 243 = 0.94921875 ≈ 1`

These near-unity combinations `(a,b) = (3,-2), (8,-5), (19,-12), (84,-53)` create a denser-than-expected grid of representable values.

## How It Works

The encoding stores two independent exponents:
- `fbbits` bits for the base-0.5 exponent (binary part)
- `sbbits = nbits - fbbits - 1` bits for the base-3 exponent (ternary part)
- 1 sign bit

Multiplication and division operate on both exponent pairs simultaneously:
```
x × y: sign = sign_x XOR sign_y
        a_result = a_x + a_y
        b_result = b_x + b_y
```

This is two independent integer additions -- even cheaper than two separate LNS multiplications, since both additions can run in parallel.

## How to Use It

### Include

```cpp
#include <universal/number/dbns/dbns.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
dbns<8, 3> a(0.5f);   // 8 bits total, 3 bits for base-0.5 exponent
dbns<8, 3> b(3.0f);

auto c = a * b;  // Cheap: parallel exponent addition
std::cout << "0.5 * 3.0 = " << c << std::endl;

// Inspect the dynamic range
std::cout << symmetry_range(a) << std::endl;
```

### Exploring the Representable Value Space

```cpp
// Enumerate all positive values in a small DBNS
dbns<6, 2> d;
for (unsigned i = 0; i < (1u << 5); ++i) {
    d.setbits(i);
    std::cout << "bits=" << to_binary(d) << " value=" << d << std::endl;
}
// The values are NOT uniformly spaced -- they form a 2D lattice
// in the log-domain, projected onto the real line
```

## Problems It Solves

| Problem | How DBNS Solves It |
|---------|-----------------------|
| Single-base LNS has sparse regions in its value distribution | Two bases create a denser, more flexible grid |
| Need finer value granularity without increasing bit width | Dual exponents provide more representable values per bit |
| Mixed-radix computations (e.g., involving factors of 2 and 3) | Natural representation for binary-ternary mixed radix |
| Research in alternative number representations | Concrete implementation for exploring dual-base arithmetic |
| Multiplication-heavy workloads with varied scale factors | Parallel exponent addition in two bases |
