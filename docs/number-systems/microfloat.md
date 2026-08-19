# Microfloat: Ultra-Low-Precision Element Types for AI Quantization

## Why

Modern AI inference processes billions of multiply-accumulate operations per second. At 32 bits per weight, a large language model consumes hundreds of gigabytes -- far more than fits in accelerator memory. Quantizing weights to 4 or 8 bits compresses models by 4-8x, enabling deployment on edge devices and reducing memory bandwidth bottlenecks on GPUs.

But not all 4-bit and 8-bit formats are the same. The OCP (Open Compute Project) Microscaling specification defines a family of micro-precision floating-point element types -- e2m1, e2m3, e3m2, e4m3, and e5m2 -- each tuned for different precision/range trade-offs. The Universal `microfloat` type implements all of these as a single parameterized template, letting you explore, validate, and benchmark any micro-precision format in software.

## What

`microfloat<nbits, es, hasInf, hasNaN, isSaturating>` is an ultra-low-precision floating-point:

| Parameter | Type | Description |
|-----------|------|-------------|
| `nbits` | `unsigned` | Total bits (4-8) |
| `es` | `unsigned` | Exponent bits (>= 1) |
| `hasInf` | `bool` | Enable infinity encoding |
| `hasNaN` | `bool` | Enable NaN encoding |
| `isSaturating` | `bool` | Overflow saturates instead of wrapping |

### OCP MX Standard Element Types

| Alias | Config | Range | Precision | Use Case |
|-------|--------|-------|-----------|----------|
| `e2m1` | `microfloat<4,2,false,false,true>` | [0.5, 6.0] | 1 fraction bit | Minimal storage, maximum compression |
| `e2m3` | `microfloat<6,2,false,false,true>` | [0.0625, 7.5] | 3 fraction bits | More fraction bits |
| `e3m2` | `microfloat<6,3,false,false,true>` | [0.03125, 28] | 2 fraction bits | More exponent range |
| `e4m3` (= `e4m3fn`) | `microfloat<8,4,false,true,false>` | [~0.002, 448] | 3 fraction bits | Balanced; the OCP OFP8 FP8 format |
| `e4m3_saturating` | `microfloat<8,4,false,true,true>` | [~0.002, 448] | 3 fraction bits | Same encoding, clamps on overflow; used by MX and NVFP4 blocks |
| `e5m2` | `microfloat<8,5,true,true,false>` | [~0.001, 57344] | 2 fraction bits | Wide range; IEEE-like, has infinity |

### The two e4m3 conversion policies

`e4m3` and `e4m3_saturating` are the same encoding -- no infinity, NaN at
`S.1111.111`, maxpos 448 at `0x7E` -- and all 256 patterns decode identically.
They differ only in what a conversion from a wider type does with a value past
maxpos:

| source | `e4m3` (OCP) | `e4m3_saturating` |
|--------|--------------|-------------------|
| 464.0 (the round-to-even tie) | `0x7E` (448) | `0x7E` (448) |
| 500.0 | `0x7F` (NaN) | `0x7E` (448) |
| `-inf` | `0xFF` (-NaN) | `0xFE` (-448) |
| `-NaN` | `0xFF` | `0xFF` |

`e4m3` follows the [OCP 8-bit Floating Point
Specification](https://www.opencompute.org/documents/ocp-8-bit-floating-point-specification-ofp8-revision-1-0-2023-12-01-pdf-1),
which is what `ml_dtypes.float8_e4m3fn`, JAX and PyTorch implement: the format
has no infinity, so overflow has to signal as NaN, and the sign is preserved.

`e4m3_saturating` is what block quantization needs. MX and NVFP4 both scale the
block maximum into a range whose top sits above 448, so the largest element of a
block routinely lands past maxpos; clipping it is right, and poisoning the block
with a NaN is not. `mxfp8` and the NVFP4 block scale use this policy.

### Key Properties

- **4 to 8 bits per element**: extreme compression for model weights and activations
- **Configurable special values**: infinity, NaN, saturation are independently controllable
- **Designed for block quantization**: typically paired with a shared scale factor (e8m0 or e4m3)
- **Trivially copyable**: fixed-size, suitable for hardware memory layouts
- **Dense encoding**: no wasted bit patterns (unlike IEEE-754 which reserves many for NaN)

## How It Works

A microfloat follows the standard sign-exponent-fraction layout:

```
[sign : 1] [exponent : es] [fraction : nbits - 1 - es]
```

The key differences from standard IEEE-754 are:
1. **Overflow policy**: a value that rounds past maxpos becomes maxpos when `isSaturating=true`, infinity when the format has one, and NaN when it has a NaN but no infinity (the OCP rule). A format with none of the three has only maxpos to offer.
2. **No infinity/NaN**: when `hasInf=false` and `hasNaN=false`, all bit patterns encode valid numbers
3. **Block scaling**: microfloats are typically not used alone -- they are elements in an `mxblock` or `nvblock`, where a shared scale factor extends their dynamic range

For example, with e2m1 (4-bit):
- Bit pattern `0 00 0` = 0.0
- Bit pattern `0 00 1` = 0.5
- Bit pattern `0 01 0` = 1.0
- Bit pattern `0 01 1` = 1.5
- Bit pattern `0 10 0` = 2.0
- Bit pattern `0 11 1` = 6.0 (maxpos, saturates here)

## How to Use It

### Include

```cpp
#include <universal/number/microfloat/microfloat.hpp>
using namespace sw::universal;
```

### Exploring Element Type Properties

```cpp
// Inspect the e4m3 format used by NVIDIA FP8
e4m3 val(1.0f);
std::cout << "e4m3 properties:\n";
std::cout << "  maxpos: " << e4m3::maxpos() << std::endl;
std::cout << "  minpos: " << e4m3::minpos() << std::endl;
std::cout << "  encoding: " << to_binary(val) << std::endl;

// Enumerate all representable values in e2m1 (only 16 values!)
e2m1 a;
for (unsigned i = 0; i < 16; ++i) {
    a.setbits(i);
    std::cout << "bits=" << to_binary(a) << " value=" << a << std::endl;
}
```

### Quantization Simulation

```cpp
// Simulate quantizing float32 weights to e4m3
std::vector<float> fp32_weights = { 0.5f, -1.25f, 3.14f, 0.001f, 200.0f };

for (float w : fp32_weights) {
    e4m3 quantized(w);
    float dequantized = float(quantized);
    float error = std::abs(w - dequantized);
    std::cout << "fp32=" << w << " -> e4m3=" << quantized
              << " -> fp32=" << dequantized
              << " error=" << error << std::endl;
}
```

### Use with Block Formats

```cpp
// Microfloats are element types for block-scaled formats
#include <universal/number/mxfloat/mxfloat.hpp>

// MX block: 32 e4m3 elements sharing one e8m0 scale
mxblock<e4m3_saturating, 32> block;   // == mxfp8

// NVIDIA block: 16 e2m1 elements sharing one e4m3 scale
#include <universal/number/nvblock/nvblock.hpp>
nvblock<e2m1, 16, e4m3_saturating> nv_block;   // == nvfp4
```

## Problems It Solves

| Problem | How microfloat Solves It |
|---------|-----------------------|
| Large models don't fit in accelerator memory | 4-8 bit elements = 4-8x compression vs float32 |
| Memory bandwidth limits inference throughput | Smaller elements = more operations per byte transferred |
| Need to validate quantization error before hardware deployment | Software emulation of exact hardware behavior |
| Different accelerators use different FP8 conventions | Configurable hasInf/hasNaN/isSaturating matches any spec |
| OCP MX compliance testing | Direct implementation of OCP element type definitions |
| Comparing precision trade-offs across micro-formats | Single template, swap parameters to compare e4m3 vs e5m2 |
