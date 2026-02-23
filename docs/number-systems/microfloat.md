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
| `e4m3` | `microfloat<8,4,false,true,true>` | [~0.002, 240] | 3 fraction bits | Balanced; NVIDIA FP8 |
| `e5m2` | `microfloat<8,5,true,true,false>` | [~0.001, 57344] | 2 fraction bits | Wide range; non-saturating |

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
1. **Saturation**: when `isSaturating=true`, overflow clamps to maxpos instead of producing infinity
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
mxblock<e4m3, 32> block;

// NVIDIA block: 16 e2m1 elements sharing one e4m3 scale
#include <universal/number/nvblock/nvblock.hpp>
nvblock<e2m1, 16, e4m3> nv_block;
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
