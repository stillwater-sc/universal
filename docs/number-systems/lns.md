# LNS: Logarithmic Number System

## Why

In many signal processing and scientific computing workflows, multiplication and division dominate the computation. Think FFTs, power spectra, convolutions, and geometric transformations. In IEEE-754 floating-point, multiplication requires a full significand multiply -- a hardware-expensive operation that consumes area, power, and latency.

In the Logarithmic Number System (LNS), every value is stored as a *logarithm*. Multiplication becomes addition of logarithms. Division becomes subtraction. These are the cheapest operations in digital logic -- just integer add/subtract on the exponent field. If your workload is multiply-heavy, LNS can provide dramatic savings in hardware cost, power consumption, and latency.

The trade-off is symmetric: addition and subtraction, which are cheap in standard floating-point, become expensive in LNS (requiring table lookups or iterative algorithms). LNS is not a universal replacement for floating-point -- it's a specialized tool for workloads where the operation mix favors multiplication.

## What

`lns<nbits, rbits, bt, xtra>` is a logarithmic number:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Total bits |
| `rbits` | `unsigned` | -- | Fractional bits in the exponent (precision of the logarithm) |
| `bt` | typename | `uint8_t` | Storage block type |
| `xtra` | variadic | -- | Arithmetic behavior (Saturating or Modulo) |

### Value Representation

```
value = ±2^exponent
```

where `exponent` is a fixed-point number stored in `nbits - 1` bits (1 bit for sign), with `rbits` fractional bits.

- **Sign bit**: 1 bit for the sign of the *value* (not the exponent)
- **Exponent field**: `nbits - 1` bits of fixed-point exponent
- **Exponent precision**: `rbits` fractional bits control how finely the exponent is resolved

### Key Properties

- **Multiplication is addition**: `x * y → exp_x + exp_y` (cheap integer add)
- **Division is subtraction**: `x / y → exp_x - exp_y` (cheap integer subtract)
- **Addition is expensive**: requires converting to linear domain, adding, converting back
- **Configurable dynamic range**: `2^(2^(nbits - rbits - 1))`
- **Configurable precision**: `rbits` controls the granularity of representable values
- **Special values**: zero (encoded as specific exponent pattern), NaN

### Operation Cost Comparison

| Operation | IEEE Float | LNS |
|-----------|-----------|-----|
| Multiplication | **Expensive** (significand multiply) | **Cheap** (exponent add) |
| Division | **Expensive** (significand divide) | **Cheap** (exponent subtract) |
| Addition | **Cheap** (align + add) | **Expensive** (table lookup) |
| Subtraction | **Cheap** (align + subtract) | **Expensive** (table lookup) |
| Square | **Expensive** (full multiply) | **Cheap** (exponent shift) |
| Square root | **Expensive** (iterative) | **Cheap** (exponent shift) |

## How It Works

The exponent is stored as a fixed-point number in `blockbinary<nbits, bt, Signed>`. The `rbits` fractional bits allow sub-power-of-2 resolution. For example, with `rbits=3`:

- Exponent `001.000` = 1.0, representing 2^1.0 = 2.0
- Exponent `001.100` = 1.5, representing 2^1.5 ≈ 2.828
- Exponent `010.000` = 2.0, representing 2^2.0 = 4.0

Multiplication and division operate directly on exponent fields:
```
x * y: sign_x XOR sign_y, exp_x + exp_y  (integer add)
x / y: sign_x XOR sign_y, exp_x - exp_y  (integer subtract)
```

Addition and subtraction require converting to the linear domain (via 2^exp), performing the operation, and converting back (via log2), which is typically done through double-precision intermediates or table lookups.

## How to Use It

### Include

```cpp
#include <universal/number/lns/lns.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
lns<8, 3> a(2.0f);  // 8-bit, 3 fractional exponent bits
lns<8, 3> b(4.0f);
lns<8, 3> c = a * b;  // Cheap: exponent addition
std::cout << "2.0 * 4.0 = " << c << std::endl;  // 8.0

// Division is equally cheap
lns<8, 3> d = a / b;
std::cout << "2.0 / 4.0 = " << d << std::endl;  // 0.5
```

### Power Spectrum Computation

```cpp
// Power spectrum: all multiplications, no additions
template<typename Real>
std::vector<Real> power_spectrum(const std::vector<Real>& signal) {
    std::vector<Real> spectrum(signal.size());
    for (size_t i = 0; i < signal.size(); ++i) {
        spectrum[i] = signal[i] * signal[i];  // Cheap in LNS
    }
    return spectrum;
}

// LNS makes this operation O(n) additions instead of O(n) multiplications
using LNS16 = lns<16, 8>;
std::vector<LNS16> signal = { LNS16(1.5), LNS16(2.3), LNS16(0.7) };
auto ps = power_spectrum(signal);
```

### Geometric Mean (Naturally Cheap in LNS)

```cpp
template<typename Real>
Real geometric_mean(const std::vector<Real>& values) {
    Real product(1);
    for (const auto& v : values) {
        product *= v;  // Cheap in LNS: just exponent accumulation
    }
    // Nth root is also cheap: divide exponent by N
    // (implementation depends on how root is exposed)
    return product;
}
```

## Problems It Solves

| Problem | How LNS Solves It |
|---------|-----------------------|
| Multiplication is the bottleneck (DSP, FFT, spectra) | Multiplication becomes cheap addition |
| Hardware multiplier is too expensive (area, power, latency) | LNS needs only an adder for multiply |
| Square root is expensive in hardware | Just shift the exponent right by 1 bit |
| Need wide dynamic range with small bit width | Logarithmic encoding provides exponential range |
| Geometric computations dominated by products | All products become sums of logarithms |
| Embedded DSP with limited hardware resources | Multiply-heavy DSP reduces to add-only operations |
