# ZFP: Compressed Floating-Point Arrays

## The Problem ZFP Solves

Scientific and engineering simulations produce enormous floating-point datasets.
A 3D fluid dynamics simulation on a 1024^3 grid with double precision uses
**8 GB per field variable** — and most simulations have dozens of fields across
thousands of time steps. Moving this data between memory, disk, and network is
the dominant bottleneck in modern HPC, not the arithmetic itself.

Traditional lossless compressors (gzip, LZ4) achieve poor ratios on
floating-point data because the bit patterns look nearly random. And lossy
image/video codecs (JPEG, H.264) are designed for 8-bit integers, not the
30+ significant bits of IEEE 754 floats.

ZFP fills this gap: **a lossy (and optionally lossless) compressor designed
specifically for structured floating-point data**, with rates and error bounds
the user controls directly.

## Why Not Just Truncate Bits?

The simplest approach — dropping low-order mantissa bits — does work, but it
ignores *spatial correlation*. Adjacent values in a simulation grid are usually
similar. ZFP exploits this correlation through a transform that concentrates
energy into a few large coefficients (like JPEG does for images, but adapted
for floats).

Consider a 1D block of four pressures from a smooth flow field:

```
[101325.0, 101327.4, 101330.1, 101332.5]
```

These values span a tiny range relative to their magnitude. After ZFP's
transform, most of the information is in the first (DC) coefficient, and the
remaining coefficients are near zero — highly compressible.

## How ZFP Works: The Pipeline

ZFP processes data in small blocks of **4^d** values (4 in 1D, 16 in 2D,
64 in 3D). Each block passes through a five-stage pipeline:

```
float block ──> block-float ──> lifting ──> reorder ──> negabinary ──> bit-planes ──> bits
```

### Stage 1: Block-Floating-Point Conversion

All values in a block share a single exponent (the maximum). Each float is
converted to a fixed-point integer relative to that shared exponent:

```
emax = max exponent across block (via frexp)
iblock[i] = (int32)(value[i] * 2^(30 - emax))
```

This is like scientific notation where all values use the same power of ten.
Values much smaller than the block maximum lose precision — this is the
fundamental trade-off of block-floating-point.

### Stage 2: Decorrelating Transform (Lifting)

A custom 4-point integer transform (similar to a discrete cosine transform)
decorrelates adjacent values. For smooth data, this concentrates most of the
energy into low-frequency coefficients:

```
Before transform: [1073741824, 1073744381, 1073747200, 1073749757]
After transform:  [1073745790, -2003, -1, 0]
```

The first coefficient holds the block average. The rest are small differences.
This is where the compression comes from — small numbers need fewer bits.

The transform is **separable**: in 2D, it applies along rows then columns;
in 3D, along x, y, then z. This keeps it O(n) per block.

### Stage 3: Coefficient Reordering

After the transform, coefficients are reordered by **total sequency**
(sum of indices). Low-frequency coefficients come first, high-frequency last.
This is analogous to JPEG's zigzag scan but generalized to arbitrary
dimensions.

In 2D, the ordering looks like:

```
 0  1  5  6
 2  3  7  8
 9 10  4 11
12 13 14 15
```

where 0 is the DC (average) coefficient.

### Stage 4: Negabinary Encoding

Signed integers are mapped to unsigned integers using **negabinary**
(base -2) encoding:

```
uint = (int + 0xAAAAAAAA) ^ 0xAAAAAAAA
```

Unlike two's complement, negabinary interleaves positive and negative values
so that magnitude corresponds directly to bit position. The most significant
bit is always the most important, regardless of sign. This allows the next
stage to process all coefficients uniformly.

### Stage 5: Embedded Bit-Plane Coding

The unsigned coefficients are encoded one bit plane at a time, from the most
significant bit down. Each plane is a single bit from every coefficient:

```
Plane 31: [0, 0, 0, 0]  ← all zero, skip
Plane 30: [1, 0, 0, 0]  ← only DC is significant
Plane 29: [1, 1, 0, 0]  ← second coefficient becomes significant
...
```

This is **progressive**: you can stop at any bit plane and get a valid
(but less precise) reconstruction. Earlier planes carry exponentially more
information than later ones.

The encoder uses **group testing** to efficiently skip runs of
zero bits: "are there any newly significant coefficients?" If not, a single
0 bit skips the entire group.

## Compression Modes

ZFP offers four modes that control where to stop encoding:

| Mode | You specify | ZFP guarantees |
|------|-------------|----------------|
| **Fixed-rate** | Bits per value | Exact compressed size (enables random access) |
| **Fixed-precision** | Number of bit planes | Relative error bound |
| **Fixed-accuracy** | Absolute tolerance | Max pointwise error |
| **Reversible** | Nothing | Bit-exact reconstruction (lossless) |

### Fixed-rate is the most distinctive feature

Most compressors produce variable-length output — you can't predict the size
in advance, and you can't jump to block #N without decoding blocks 0 through
N-1. Fixed-rate mode produces **exactly** the specified number of bits per
block, enabling:

- **Random access**: Read any block without decompressing neighbors
- **In-memory compression**: Allocate a fixed-size buffer, compress in place
- **Predictable bandwidth**: Network transfers have known size

This is what makes ZFP suitable as a **compressed array** backend, not just
a file format.

## What ZFP Is Good At

**Smooth, structured data** — the kind produced by PDE solvers, weather
models, seismic imaging, and CFD simulations. The decorrelating transform
assumes spatial locality, so correlated neighbors compress well.

Typical compression ratios on scientific data:

| Rate (bits/value) | Ratio | Typical RMSE (relative) |
|-------------------|-------|------------------------|
| 32 (uncompressed) | 1x | 0 |
| 16 | 2x | ~10^-6 |
| 8 | 4x | ~10^-4 |
| 4 | 8x | ~10^-2 |
| 2 | 16x | ~10^-1 |

These numbers vary by dataset. Smoother data compresses better.

## What ZFP Is Not Good At

- **Random/unstructured data**: Without spatial correlation, the transform
  doesn't help. Compression ratios approach 1x.
- **Point clouds or sparse data**: ZFP expects dense, regular grids.
  Scattered points need a different approach.
- **Small integer data**: If your data is already quantized (uint8 images,
  boolean masks), conventional compressors are simpler and faster.
- **Bit-exact requirements at high compression**: The lifting transform
  introduces rounding that prevents bit-exact lossless compression at
  ratios better than ~1x. Use reversible mode when exactness matters.

## Example: Using zfpblock in Universal

```cpp
#include <universal/number/zfpblock/zfpblock.hpp>
#include <iostream>
#include <cmath>

int main() {
    using namespace sw::universal;

    // --- 1D example: compress 4 temperature readings ---
    float temps[4] = { 20.1f, 20.3f, 20.2f, 20.4f };
    float restored[4];

    zfp1f block;

    // Fixed-rate: exactly 8 bits per value (4x compression)
    block.compress_fixed_rate(temps, 8.0);
    block.decompress(restored);

    std::cout << "1D fixed-rate (8 bpv):\n";
    for (int i = 0; i < 4; ++i)
        std::cout << "  " << temps[i] << " -> " << restored[i]
                  << " (err=" << std::abs(restored[i] - temps[i]) << ")\n";
    std::cout << "Ratio: " << block.compression_ratio() << "x\n\n";

    // --- 2D example: compress a 4x4 pressure field ---
    float pressure[16];
    for (int j = 0; j < 4; ++j)
        for (int i = 0; i < 4; ++i)
            pressure[j * 4 + i] = 101325.0f + (i + j) * 2.5f;

    float p_out[16];
    zfp2f block2d;
    block2d.compress_fixed_rate(pressure, 16.0);  // 16 bpv = 2x compression
    block2d.decompress(p_out);

    double max_err = 0;
    for (int i = 0; i < 16; ++i) {
        double err = std::abs(p_out[i] - pressure[i]);
        if (err > max_err) max_err = err;
    }
    std::cout << "2D fixed-rate (16 bpv): max error = " << max_err
              << ", ratio = " << block2d.compression_ratio() << "x\n\n";

    // --- Reversible (lossless) mode ---
    double precise[4] = { 3.141592653589793, 2.718281828459045,
                          1.414213562373095, 1.732050808568877 };
    double precise_out[4];
    zfp1d block_d;
    block_d.compress_reversible(precise);
    block_d.decompress(precise_out);

    std::cout << "1D reversible (double):\n";
    for (int i = 0; i < 4; ++i)
        std::cout << "  " << precise[i] << " -> " << precise_out[i] << "\n";
    std::cout << "Compressed: " << block_d.compressed_bits() << " bits"
              << " (" << block_d.compressed_bytes() << " bytes)\n";

    return 0;
}
```

## How This Relates to the Reference ZFP Library

The LLNL ZFP library (github.com/LLNL/zfp) is a production implementation
with streaming codecs, OpenMP/CUDA parallelism, compressed array containers,
Python/Fortran bindings, and extensive testing against real-world datasets.

This Universal `zfpblock` implementation is a **single-block codec** — it
implements the core transform and encoding pipeline for one 4^d block at a
time. It is useful for:

- Understanding how ZFP works at the algorithmic level
- Integrating ZFP-style compression into custom number type workflows
- Experimenting with block-level compression in mixed-precision pipelines
- Serving as the building block for a compressed array container (Phase 4b)

## Further Reading

- Lindstrom, P. "Fixed-Rate Compressed Floating-Point Arrays."
  *IEEE Transactions on Visualization and Computer Graphics*, 20(12), 2014.
- ZFP documentation: https://zfp.readthedocs.io
- ZFP source code: https://github.com/LLNL/zfp
