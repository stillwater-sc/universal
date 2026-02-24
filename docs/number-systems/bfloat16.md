# bfloat16: Google Brain Floating-Point for Deep Learning

## Why

Training neural networks requires a format that balances three concerns: the dynamic range must be wide enough to represent gradients (which can span many orders of magnitude), the storage must be compact enough to fit large models in memory, and arithmetic must be fast enough to process billions of operations per second.

IEEE half-precision (float16) has only 5 exponent bits, giving a dynamic range of ~6.5 x 10^4 -- far too narrow for gradient accumulation, where values routinely span 10^-8 to 10^4. Google's Brain Float (bfloat16) solves this by truncating float32 to 16 bits from the top: same 8 exponent bits as float32 (dynamic range ~3.4 x 10^38), with a reduced 7-bit fraction. The result is a format perfectly tuned for deep learning: wide range, compact storage, and trivial conversion to/from float32 (just truncate or zero-extend the lower 16 bits).

## What

`bfloat16` is a fixed-format 16-bit floating-point type:

```
[sign : 1 bit] [exponent : 8 bits] [fraction : 7 bits]
```

| Property | Value |
|----------|-------|
| Total bits | 16 |
| Exponent bits | 8 |
| Fraction bits | 7 |
| Exponent bias | 127 |
| Dynamic range | ~1.2 x 10^-38 to ~3.4 x 10^38 |
| Precision | ~2 decimal digits |
| Subnormals | No |
| Infinity | Yes |
| NaN | Yes |

### Key Properties

- **Not a template**: fixed format, no configuration parameters
- **Same dynamic range as float32**: 8 exponent bits, bias 127
- **Trivial conversion to/from float32**: top 16 bits of IEEE-754 binary32
- **Constexpr support**: compile-time evaluation
- **Trivially copyable**: suitable for hardware acceleration

### Comparison with Other 16-bit Formats

| Format | Exponent | Fraction | Dynamic Range | Precision |
|--------|----------|----------|--------------|-----------|
| IEEE float16 | 5 bits | 10 bits | ~6.5 x 10^4 | ~3.3 digits |
| **bfloat16** | **8 bits** | **7 bits** | **~3.4 x 10^38** | **~2 digits** |
| float32 | 8 bits | 23 bits | ~3.4 x 10^38 | ~7.2 digits |

## How It Works

bfloat16 is the upper 16 bits of a 32-bit IEEE-754 float. This means:
- **Conversion from float32**: truncate (or round) the lower 16 fraction bits
- **Conversion to float32**: zero-extend the lower 16 fraction bits
- **Arithmetic**: performed by promoting to float32, computing, then truncating back

The Universal implementation stores the encoding in a `uint16_t` and uses `std::memcpy`-based conversion to/from native `float`, ensuring constexpr compatibility where the compiler supports it.

## How to Use It

### Include

```cpp
#include <universal/number/bfloat16/bfloat16.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
bfloat16 weight(0.5f);
bfloat16 gradient(-0.001f);
bfloat16 learning_rate(0.01f);

// Gradient descent update
weight = weight - learning_rate * gradient;

std::cout << "Updated weight: " << weight << std::endl;
std::cout << "Encoding: " << to_binary(weight) << std::endl;
```

### Mixed-Precision Training Pattern

```cpp
// Typical mixed-precision workflow:
// 1. Store weights and activations in bfloat16 (saves memory)
// 2. Accumulate in float32 (preserves accuracy)
// 3. Cast result back to bfloat16 for storage

template<typename LowPrec>
float accumulate_gradients(const std::vector<LowPrec>& gradients) {
    float sum = 0.0f;  // Accumulate in float32
    for (const auto& g : gradients) {
        sum += float(g);
    }
    return sum;
}

std::vector<bfloat16> grads = { bfloat16(0.001f), bfloat16(-0.002f), bfloat16(0.0005f) };
float total = accumulate_gradients(grads);
bfloat16 avg_gradient(total / grads.size());
```

### Plug-in Replacement for Neural Network Layers

```cpp
template<typename Real>
void linear_layer(const std::vector<Real>& input,
                  const std::vector<std::vector<Real>>& weights,
                  std::vector<Real>& output) {
    for (size_t i = 0; i < weights.size(); ++i) {
        Real sum(0);
        for (size_t j = 0; j < input.size(); ++j) {
            sum += weights[i][j] * input[j];
        }
        output[i] = sum;
    }
}

// Use with bfloat16 for 2x memory reduction vs float32
std::vector<bfloat16> input = { bfloat16(1.0f), bfloat16(0.5f) };
std::vector<std::vector<bfloat16>> weights = {
    { bfloat16(0.3f), bfloat16(0.7f) },
    { bfloat16(-0.2f), bfloat16(0.4f) }
};
std::vector<bfloat16> output(2);
linear_layer(input, weights, output);
```

## Problems It Solves

| Problem | How bfloat16 Solves It |
|---------|-----------------------|
| Neural network weights don't fit in GPU memory | 2x compression vs float32, same dynamic range |
| float16 gradients overflow during training | 8-bit exponent matches float32 range |
| Memory bandwidth limits training throughput | Half the bytes per element = 2x bandwidth efficiency |
| Need fast conversion between training (float32) and storage (16-bit) | Truncate/extend top 16 bits -- no complex conversion |
| Google TPU and AI accelerator compatibility | Native bfloat16 format used by TPUs |
| Transformer models with billions of parameters | Compact storage enables larger models and batch sizes |
