# MX Float: OCP Microscaling Block Format for AI

## Why

Large language models contain billions of parameters. Storing each in 32-bit float costs hundreds of gigabytes -- far more than fits in GPU memory. Naive per-element quantization to 4 or 8 bits can lose too much precision, especially for outlier values that are critical to model accuracy.

The OCP (Open Compute Project) Microscaling (MX) format solves this by sharing a single scale factor across a *block* of elements. One `e8m0` scale value (a power-of-two exponent in a single byte) captures the block's magnitude, while each element stores only its relative value in 4-8 bits. This achieves 4-8x compression with controlled precision loss: the shared scale preserves the overall magnitude, and individual elements capture the relative distribution within each block.

## What

`mxblock<ElementType, BlockSize>` is a block-scaled number format:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `ElementType` | typename | -- | Microfloat element type (e2m1, e2m3, e3m2, e4m3, e5m2, or int8_t) |
| `BlockSize` | `size_t` | 32 | Elements per block (OCP default: 32) |

### Standard MX Format Aliases

| Alias | Element | Block Size | Bits/Element | Compression vs FP32 |
|-------|---------|-----------|--------------|---------------------|
| `mxfp4` | e2m1 | 32 | 4 | ~8x |
| `mxfp6` | e3m2 | 32 | 6 | ~5x |
| `mxfp6e2m3` | e2m3 | 32 | 6 | ~5x |
| `mxfp8` | e4m3 | 32 | 8 | ~4x |
| `mxfp8e5m2` | e5m2 | 32 | 8 | ~4x |
| `mxint8` | int8_t | 32 | 8 | ~4x |

### Block Structure

```
[e8m0 scale : 8 bits] [element[0] : N bits] [element[1] : N bits] ... [element[31] : N bits]
```

Total storage per block: 8 + BlockSize × element_bits

### Key Properties

- **Shared scale factor**: one e8m0 per block, amortized over 32 elements
- **Power-of-two scale**: `floor(log2(amax))` -- bit shift, not multiply
- **OCP MX v1.0 compliant**: industry standard for AI model quantization
- **Block-level quantization**: preserves relative precision within each block
- **Deterministic**: quantize/dequantize is bit-exact and reproducible

## How It Works

### Quantization (float → mxblock)

1. Find the absolute maximum of the input block: `amax = max(|input[i]|)`
2. Compute shared exponent: `shared_exp = floor(log2(amax))`
3. Clamp to e8m0 range: `scale = clamp(shared_exp, -127, 127)`
4. Scale each element: `element[i] = input[i] / 2^scale`
5. Round each scaled element to the nearest ElementType value

### Dequantization (mxblock → float)

```
output[i] = scale × element[i]
```

where `scale` is the e8m0 value (a power of 2).

### Dot Product

Dot product of two mxblocks accumulates in FP32:
```
result = scale_a × scale_b × Σ(element_a[i] × element_b[i])
```

## How to Use It

### Include

```cpp
#include <universal/number/mxfloat/mxfloat.hpp>
using namespace sw::universal;
```

### Quantize and Dequantize

```cpp
mxblock<e4m3, 32> block;

// Quantize float data
std::vector<float> weights = { 0.5f, -1.25f, 3.14f, /* ... 32 values */ };
block.quantize(weights);

// Dequantize back to float
std::vector<float> reconstructed(32);
block.dequantize(reconstructed);

// Measure quantization error
for (size_t i = 0; i < 32; ++i) {
    float error = std::abs(weights[i] - reconstructed[i]);
    std::cout << "element " << i << ": error = " << error << std::endl;
}
```

### Block Dot Product

```cpp
mxblock<e4m3, 32> block_a, block_b;

std::vector<float> a(32), b(32);
// ... fill a and b with weight/activation data ...
block_a.quantize(a);
block_b.quantize(b);

float dot = block_a.dot(block_b);
std::cout << "Block dot product: " << dot << std::endl;
```

### Comparing MX Format Variants

```cpp
std::vector<float> data(32);
// ... fill with neural network weights ...

mxblock<e2m1, 32> mx4;   // 4-bit: maximum compression
mxblock<e4m3, 32> mx8;   // 8-bit: balanced precision/size
mxblock<e5m2, 32> mx8w;  // 8-bit: wider range, less precision

mx4.quantize(data);
mx8.quantize(data);
mx8w.quantize(data);

// Compare RMSE for each format
std::cout << "mxfp4 RMSE:    " << mx4.rmse(data) << std::endl;
std::cout << "mxfp8 RMSE:    " << mx8.rmse(data) << std::endl;
std::cout << "mxfp8e5m2 RMSE:" << mx8w.rmse(data) << std::endl;
```

## Problems It Solves

| Problem | How mxfloat Solves It |
|---------|-----------------------|
| LLM weights don't fit in GPU memory | 4-8x compression with shared block scale |
| Per-element quantization loses outlier values | Block scale captures magnitude, elements capture distribution |
| Memory bandwidth limits inference throughput | Fewer bytes per element = higher effective bandwidth |
| Need industry-standard quantization for accelerator compatibility | OCP MX v1.0 compliant |
| Quantization error varies unpredictably | Deterministic quantize/dequantize, measurable RMSE |
| Model deployment on edge devices with limited memory | Extreme compression (mxfp4 = 8x vs FP32) |
