# NVBlock: NVIDIA NVFP4 Block Format for GPU Inference

## Why

NVIDIA's AI accelerators (Hopper, Blackwell) need a quantization format optimized for their hardware architecture. While OCP MX uses 32-element blocks with a power-of-two scale (e8m0), NVIDIA's NVFP4 format uses 16-element blocks with a *fractional* scale (e4m3) and an additional tensor-level scale. This design achieves finer granularity than MX's power-of-two scaling, resulting in lower quantization error for the same compression ratio, at the cost of slightly more complex scale computation.

The `nvblock` type implements NVIDIA's two-level scaling architecture, enabling software validation and benchmarking of NVFP4-quantized models before deployment on GPU hardware.

## What

`nvblock<ElementType, BlockSize, ScaleType>` is a two-level block-scaled format:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `ElementType` | typename | `e2m1` | Element type (e2m1 for NVFP4) |
| `BlockSize` | `size_t` | 16 | Elements per block (NVIDIA default: 16) |
| `ScaleType` | typename | `e4m3` | Block scale type (fractional, not power-of-two) |

### Standard Configuration

```cpp
using nvfp4 = nvblock<e2m1, 16, e4m3>;  // Canonical NVFP4
```

### Two-Level Scaling Architecture

```
Tensor scale (float32, external)
    └─── Block scale (e4m3, per block of 16 elements)
              └─── Elements (e2m1, per value)
```

- **Tensor scale**: float32 value, one per tensor or per layer
- **Block scale**: e4m3 value (fractional precision), one per 16 elements
- **Elements**: e2m1 values (4-bit), one per weight/activation

### Key Differences from OCP MX

| Property | OCP MX (mxblock) | NVIDIA (nvblock) |
|----------|-------------------|-------------------|
| Block size | 32 | 16 |
| Scale type | e8m0 (power-of-two) | e4m3 (fractional) |
| Scale precision | 254 power-of-2 values | ~256 fractional values |
| Scale computation | `floor(log2(amax))` | `round_to_e4m3(amax / elem_max)` |
| External scale | None | Tensor-level float32 |
| RMSE for same data | Higher | Lower (finer granularity) |

### Key Properties

- **Fractional block scale**: e4m3 provides finer granularity than power-of-two
- **Two-level scaling**: tensor scale + block scale = better dynamic range
- **Smaller blocks (16)**: better spatial locality for GPU cache
- **NVIDIA hardware-native**: aligned with Tensor RT and GPU warp operations
- **Lower quantization error**: consistently lower RMSE than MX at same bit width

## How It Works

### Quantization (float → nvblock)

1. Pre-divide inputs by tensor_scale: `prescaled[i] = input[i] / tensor_scale`
2. Find absolute maximum of the prescaled block: `amax = max(|prescaled[i]|)`
3. Compute raw scale: `raw_scale = amax / elem_max` (where elem_max is the maximum e2m1 value)
4. Round to nearest e4m3: `block_scale = round_to_e4m3(raw_scale)` (not floor!)
5. Scale each element: `element[i] = round_to_e2m1(prescaled[i] / block_scale)`

### Dequantization (nvblock → float)

```
output[i] = tensor_scale × block_scale × element[i]
```

### Dot Product

```
result = tensor_scale_a × tensor_scale_b × Σ(block_scale_a × block_scale_b × element_a[i] × element_b[i])
```

The tensor scales can be factored out of the inner loop for efficiency.

## How to Use It

### Include

```cpp
#include <universal/number/nvblock/nvblock.hpp>
using namespace sw::universal;
```

### Quantize and Dequantize

```cpp
nvblock<e2m1, 16, e4m3> block;
float tensor_scale = 1.0f;  // Often computed per-layer

std::vector<float> weights(16);
// ... fill weights ...

block.quantize(weights, tensor_scale);

std::vector<float> reconstructed(16);
block.dequantize(reconstructed, tensor_scale);
```

### Tensor-Scaled Dot Product

```cpp
nvblock<e2m1, 16, e4m3> block_a, block_b;
float ts_a = 1.0f, ts_b = 1.0f;  // Tensor scales

std::vector<float> a(16), b(16);
// ... fill a and b ...
block_a.quantize(a, ts_a);
block_b.quantize(b, ts_b);

float dot = block_a.dot(block_b, ts_a, ts_b);
```

### Comparing with OCP MX

```cpp
#include <universal/number/mxfloat/mxfloat.hpp>
#include <universal/number/nvblock/nvblock.hpp>

std::vector<float> data(32);
// ... fill with neural network weights ...

// OCP MX: 32 elements, e8m0 power-of-two scale
mxblock<e2m1, 32> mx_block;
mx_block.quantize(data);

// NVIDIA: 16 elements, e4m3 fractional scale (process two blocks)
nvblock<e2m1, 16, e4m3> nv_block_lo, nv_block_hi;
std::vector<float> lo(data.begin(), data.begin() + 16);
std::vector<float> hi(data.begin() + 16, data.end());
nv_block_lo.quantize(lo, 1.0f);
nv_block_hi.quantize(hi, 1.0f);

// NVIDIA typically achieves lower RMSE due to fractional scale granularity
```

## Problems It Solves

| Problem | How nvblock Solves It |
|---------|-----------------------|
| MX's power-of-two scale is too coarse | e4m3 fractional scale provides finer granularity |
| Need NVIDIA GPU-native quantization format | Direct implementation of NVFP4 specification |
| Single-level scale can't capture both layer-wide and local dynamics | Two-level scaling (tensor + block) |
| 32-element blocks don't match GPU warp size | 16-element blocks align with GPU cache lines |
| Software validation of quantized models before GPU deployment | Bit-exact emulation of hardware quantization |
| Quantization RMSE too high with MX format | Fractional scale consistently produces lower error |
