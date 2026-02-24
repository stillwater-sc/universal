# ZFP Block: Compressed Floating-Point for Scientific Data

## Why

Scientific simulations produce enormous datasets: a 3D fluid dynamics simulation on a 1024^3 grid stores 4 billion floating-point values per time step. At double precision, one snapshot is 32 GB. A 1000-step run generates 32 TB. Storing, transmitting, and analyzing this data is the bottleneck in modern computational science.

ZFP (developed at Lawrence Livermore National Laboratory) is a block-based floating-point compression codec designed specifically for scientific data. Unlike general-purpose compressors (gzip, lz4), ZFP exploits the structure of floating-point arrays: spatial coherence, smooth variation between neighboring values, and the specific bit layout of IEEE-754. The result is 2-10x compression with controllable accuracy, including a lossless mode.

The Universal `zfpblock` type brings ZFP's compression pipeline into the type system, enabling compressed storage that integrates seamlessly with numerical computation.

## What

`zfpblock<Real, Dim>` is a compressed floating-point block:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `Real` | typename | `float` | Base floating-point type (float or double) |
| `Dim` | `unsigned` | 1 | Dimensionality (1D, 2D, or 3D) |

### Block Sizes

| Dimension | Block Shape | Values per Block |
|-----------|------------|-----------------|
| 1D | 4 | 4 |
| 2D | 4 × 4 | 16 |
| 3D | 4 × 4 × 4 | 64 |

### Standard Type Aliases

```cpp
using zfp1f = zfpblock<float, 1>;    // 1D float blocks
using zfp1d = zfpblock<double, 1>;   // 1D double blocks
using zfp2f = zfpblock<float, 2>;    // 2D float blocks
using zfp2d = zfpblock<double, 2>;   // 2D double blocks
using zfp3f = zfpblock<float, 3>;    // 3D float blocks
using zfp3d = zfpblock<double, 3>;   // 3D double blocks
```

### Compression Modes

| Mode | Control | Description |
|------|---------|-------------|
| `fixed_rate` | Bits per value | Constant bitstream size; predictable memory |
| `fixed_precision` | Max bit planes | Maximum number of bit planes to encode |
| `fixed_accuracy` | Error tolerance | Absolute error bound per value |
| `reversible` | Lossless | All bit planes preserved; exact round-trip |

### Key Properties

- **Block-based**: compresses 4^Dim values at a time
- **Progressive**: can decompress partial bit planes for preview
- **Lossy + lossless**: configurable accuracy/size trade-off
- **Spatial decorrelation**: lifting transform exploits local smoothness
- **Negabinary encoding**: efficient sign handling without separate bit
- **Random access**: decompress any block independently

## How It Works

The ZFP codec applies a multi-stage pipeline to each block:

### Compression Pipeline

1. **Block-float exponent**: extract shared exponent for the block (like block-float quantization)
2. **Lifting transform**: decorrelate values using a forward wavelet-like transform that predicts each value from its neighbors
3. **Reorder by sequency**: permute coefficients to group by frequency (low-frequency first)
4. **Negabinary encoding**: convert to sign-preserving two's complement where the sign is embedded in the least significant bits
5. **Bit-plane encoding**: emit bits from most significant plane to least, achieving progressive precision

### Decompression Pipeline

The pipeline runs in reverse: decode bit planes → negabinary decode → inverse reorder → inverse lifting transform → restore exponent.

### Compression Ratio

Depends on data smoothness and mode:
- **Smooth data** (temperature fields, pressure): 4-10x lossy, 1.5-2x lossless
- **Noisy data** (turbulence, particle positions): 2-4x lossy, 1.1-1.5x lossless
- **fixed_rate mode**: exact control over bits per value (e.g., 8 bits/value = 4x for float)

## How to Use It

### Include

```cpp
#include <universal/number/zfpblock/zfpblock.hpp>
using namespace sw::universal;
```

### Basic Block Compression

```cpp
// Compress a 4×4 block of float data
zfpblock<float, 2> block;

float data[4][4] = {
    { 1.0f, 1.1f, 1.2f, 1.3f },
    { 1.1f, 1.2f, 1.3f, 1.4f },
    { 1.2f, 1.3f, 1.4f, 1.5f },
    { 1.3f, 1.4f, 1.5f, 1.6f }
};

block.encode(&data[0][0], fixed_rate, 8);  // 8 bits per value = 4x compression

float reconstructed[4][4];
block.decode(&reconstructed[0][0]);

// Measure reconstruction error
for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
        std::cout << "error[" << i << "][" << j << "] = "
                  << std::abs(data[i][j] - reconstructed[i][j]) << std::endl;
```

### Compressed Array Container

```cpp
// zfparray provides a compressed array interface
zfparray<float, 2> compressed_field(1024, 1024);

// Write values (transparently compressed in blocks)
for (int i = 0; i < 1024; ++i)
    for (int j = 0; j < 1024; ++j)
        compressed_field(i, j) = std::sin(i * 0.01) * std::cos(j * 0.01);

// Read values (transparently decompressed)
float val = compressed_field(512, 512);

// Storage: ~4x less than uncompressed float array
```

### Simulation Data Checkpointing

```cpp
// Save simulation state with ZFP compression
template<typename Real, unsigned Dim>
void checkpoint(const std::string& filename,
                const Real* data, size_t n,
                int bits_per_value) {
    // Compress blocks and write to file
    zfpblock<Real, Dim> block;
    // ... iterate over blocks, compress, write ...
}

// 3D fluid dynamics: 64 values per block, 8 bits/value = 4x compression
// 32 TB simulation → 8 TB compressed checkpoints
```

## Problems It Solves

| Problem | How zfpblock Solves It |
|---------|-----------------------|
| Scientific simulation data is too large to store | 2-10x compression with controllable accuracy |
| General compressors don't understand floating-point | ZFP exploits IEEE-754 structure and spatial coherence |
| Need random access to compressed data | Each block is independently decompressible |
| Need both lossless and lossy modes | Reversible mode for exact round-trip, lossy modes for higher compression |
| Memory bandwidth limits simulation performance | Compressed in-memory storage reduces bandwidth needs |
| Checkpoint I/O dominates simulation runtime | 4-10x smaller checkpoints = proportionally faster I/O |
| Progressive visualization of large datasets | Bit-plane encoding enables multi-resolution preview |
