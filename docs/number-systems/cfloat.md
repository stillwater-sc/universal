# Classic Floating-Point (cfloat): Configurable IEEE-754 Compatible Arithmetic

## Why

IEEE-754 defines a handful of fixed formats: half (16-bit), single (32-bit), double (64-bit), and quad (128-bit). But modern workloads -- especially deep learning, DSP, and mixed-precision HPC -- need floating-point formats that don't exist in the standard: 8-bit floats for inference, 24-bit floats for AMD GPUs, formats with supernormal numbers for gradual overflow, or saturating arithmetic instead of infinity.

The Universal `cfloat` type is a fully parameterized floating-point that can emulate *any* IEEE-754 format and extend beyond it. You configure the total bit width, exponent size, subnormal behavior, supernormal behavior, and overflow semantics at compile time. The result is a type that behaves exactly like hardware floating-point but with the precision and range *you* choose.

## What

`cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>` is a configurable floating-point:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Total bits (4 to 256+) |
| `es` | `unsigned` | -- | Exponent bits |
| `bt` | typename | `uint8_t` | Storage block type |
| `hasSubnormals` | `bool` | -- | Enable gradual underflow |
| `hasSupernormals` | `bool` | -- | Enable gradual overflow |
| `isSaturating` | `bool` | -- | Overflow saturates instead of producing infinity |

### Encoding

Standard sign-exponent-fraction layout:

```
[sign : 1 bit] [exponent : es bits] [fraction : nbits - 1 - es bits]
```

- Exponent bias: `2^(es-1) - 1`
- Hidden bit: 1 for normal numbers, 0 for subnormals
- Infinity: exponent all-1s, fraction all-0s (when not hasSupernormals)
- NaN: exponent all-1s, fraction non-zero (when not hasSupernormals)
- When `hasSupernormals`: the all-1s exponent encodes additional normal values instead of inf/NaN
- When `isSaturating`: overflow produces maxpos/maxneg instead of infinity

### Standard IEEE-754 Aliases

```cpp
using fp8e2m5   = cfloat<8, 2, uint8_t, true, false, false>;     // quarter precision
using fp8e3m4   = cfloat<8, 3, uint8_t, true, false, false>;     // ML training
using fp8e4m3   = cfloat<8, 4, uint8_t, true, false, false>;     // NVIDIA FP8
using fp8e5m2   = cfloat<8, 5, uint8_t, true, false, false>;     // NVIDIA FP8
using half      = cfloat<16, 5, uint16_t, true, false, false>;   // IEEE-754 binary16
using single    = cfloat<32, 8, uint32_t, true, false, false>;   // IEEE-754 binary32
using duble     = cfloat<64, 11, uint32_t, true, false, false>;  // IEEE-754 binary64
using quad      = cfloat<128, 15, uint32_t, true, false, false>; // IEEE-754 binary128
```

### Deep Learning Format Aliases

```cpp
using bfloat_t = cfloat<16, 8, uint16_t, true, false, false>;    // Google Brain float
using msfp8    = cfloat<8, 2, uint8_t, false, false, false>;     // Microsoft FP8
using amd24    = cfloat<24, 8, uint32_t, true, false, false>;    // AMD 24-bit
```

## How It Works

Arithmetic is performed using a `blocktriple` intermediate representation that carries sufficient precision to compute exact products and sums before rounding. The pipeline is:

1. **Decode** operands into blocktriple (sign, scale, significant)
2. **Compute** in extended precision (the blocktriple is wider than the target)
3. **Round** using round-to-nearest-even (or configurable rounding mode)
4. **Encode** back into cfloat format

This mirrors how hardware FPUs work internally but is fully parameterized at the template level.

## How to Use It

### Include

```cpp
#include <universal/number/cfloat/cfloat.hpp>
using namespace sw::universal;
```

### Custom 8-bit Float for Deep Learning

```cpp
// FP8 with 4 exponent bits, 3 fraction bits, saturating
using fp8 = cfloat<8, 4, uint8_t, true, false, true>;

fp8 weight(0.5f);
fp8 activation(0.75f);
fp8 result = weight * activation;  // Saturates on overflow, no infinity

// Explore the encoding
std::cout << to_binary(result) << " = " << result << std::endl;
std::cout << "maxpos: " << fp8::maxpos() << std::endl;
std::cout << "minpos: " << fp8::minpos() << std::endl;
```

### Mixed-Precision Algorithm Development

```cpp
template<typename HighPrec, typename LowPrec>
HighPrec mixed_precision_dot(const std::vector<LowPrec>& a,
                              const std::vector<LowPrec>& b) {
    HighPrec sum(0);
    for (size_t i = 0; i < a.size(); ++i) {
        sum += HighPrec(a[i]) * HighPrec(b[i]);  // accumulate in high precision
    }
    return sum;
}

using FP8  = cfloat<8, 4, uint8_t, true, false, false>;
using FP32 = cfloat<32, 8, uint32_t, true, false, false>;

std::vector<FP8> weights = { FP8(0.5), FP8(0.25), FP8(0.125) };
std::vector<FP8> inputs  = { FP8(1.0), FP8(2.0), FP8(3.0) };
FP32 result = mixed_precision_dot<FP32>(weights, inputs);
```

### Gradual Overflow with Supernormals

```cpp
// Standard: overflow -> infinity
using Standard = cfloat<8, 4, uint8_t, true, false, false>;

// Supernormal: overflow -> extended normal range (no infinity)
using Extended = cfloat<8, 4, uint8_t, true, true, false>;

Standard s(200.0f);  // may produce infinity
Extended e(200.0f);  // uses supernormal encoding, stays finite
```

## Problems It Solves

| Problem | How cfloat Solves It |
|---------|-----------------------|
| Need 8-bit float for ML inference but IEEE has no 8-bit format | `cfloat<8, es>` with configurable exponent width |
| Infinity corrupts neural network training | `isSaturating=true` clamps to maxpos |
| Exploring precision/range trade-offs for new hardware | Any combination of nbits and es |
| Need IEEE-754 semantics in software for testing | Exact emulation of half/single/double/quad |
| Custom float for FPGA/ASIC design exploration | Parameterized at compile time, matches hardware encoding |
| Subnormal handling is too slow on some hardware | `hasSubnormals=false` to disable gradual underflow |
| Need wider-than-quad precision (128-bit+) | `cfloat<256, 19>` for octuple precision |
