# Block Floating-Point Formats: Implementation Roadmap

## Executive Summary

This roadmap covers the implementation of three families of block-scaled
number formats in the Universal Numbers Library:

1. **OCP Microscaling (MX)** -- MXFP4, MXFP6, MXFP8, MXINT8
2. **NVIDIA NVFP4** -- 16-element blocks with E4M3 scaling
3. **LLNL ZFP** -- compressed floating-point arrays with transform coding

These formats are foundational to modern AI inference, HPC compression,
and mixed-precision algorithm design.

---

## Architecture Overview

```
include/sw/universal/number/
  microfloat/         # Micro-float element types (shared foundation)
  mxfloat/            # OCP Microscaling block formats
  nvblock/            # NVIDIA block formats (NVFP4)
  zfp/                # ZFP compressed floating-point arrays

static/
  microfloat/         # Regression tests for micro-float elements
  mxfloat/            # Regression tests for MX block formats
  nvblock/            # Regression tests for NVIDIA block formats
  zfp/                # Regression tests for ZFP
```

All block formats share the micro-float element types as a common
foundation. The element types are useful in their own right as
standalone scalar types for experimentation and inspection.

---

## Phase 1: Micro-Float Element Types (`microfloat`)

**Goal:** Implement standalone scalar types for all sub-byte and
mini-float formats used as elements in block formats.

**Timeline:** Foundation -- implement first

### Element Types

| Type     | Bits | Layout (S/E/M) | Bias | Inf | NaN    | Max   |
|----------|------|-----------------|------|-----|--------|-------|
| e2m1     | 4    | 1/2/1           | 1    | No  | No     | 6.0   |
| e2m3     | 6    | 1/2/3           | 1    | No  | No     | 7.5   |
| e3m2     | 6    | 1/3/2           | 3    | No  | No     | 28.0  |
| e4m3     | 8    | 1/4/3           | 7    | No  | 1 NaN  | 448   |
| e5m2     | 8    | 1/5/2           | 15   | Yes | Yes    | 57344 |
| e8m0     | 8    | 0/8/0           | 127  | No  | 0xFF   | 2^127 |

### Design Decisions

The `e8m0` type is unique: no sign bit, no mantissa, exponent-only.
It represents only non-negative powers of two (2^(E-127)) and NaN.
This requires its own implementation, not a parameterization of cfloat.

The `e2m1` through `e5m2` types are standard mini-floats that _could_
be parameterized via cfloat, but warrant dedicated lightweight types for:
- Sub-byte packing (4-bit and 6-bit types don't align to byte boundaries)
- No-infinity / saturation-to-finite semantics (e4m3, e2m1, etc.)
- Tight encoding tables (e2m1 has only 16 values)

### Directory Structure

```
include/sw/universal/number/microfloat/
  microfloat.hpp          # Public umbrella header
  microfloat_fwd.hpp      # Forward declarations
  e2m1.hpp                # FP4 E2M1 implementation
  e2m3.hpp                # FP6 E2M3 implementation
  e3m2.hpp                # FP6 E3M2 implementation
  e4m3.hpp                # FP8 E4M3 (fnuz) implementation
  e5m2.hpp                # FP8 E5M2 implementation
  e8m0.hpp                # Scale-only exponent type
  microfloat_traits.hpp   # Type traits and predicates
  numeric_limits.hpp      # std::numeric_limits specializations
  exceptions.hpp          # Exception definitions
  attributes.hpp          # Query functions
  manipulators.hpp        # I/O and debug utilities
```

### Regression Tests

```
static/microfloat/
  CMakeLists.txt
  api/
    api.cpp               # Comprehensive API walkthrough
    attributes.cpp
    number_traits.cpp
  conversion/
    assignment.cpp        # From/to native types
    tables.cpp            # Exhaustive value table verification
  logic/
    logic.cpp             # Comparison operators
  arithmetic/
    addition.cpp
    multiplication.cpp
```

### Deliverables

- [ ] `e8m0` type: encode/decode, NaN handling, power-of-two semantics
- [ ] `e2m1` type: all 16 encodings, subnormal support, saturation
- [ ] `e3m2` type: all 64 encodings, no inf/nan
- [ ] `e2m3` type: all 64 encodings, no inf/nan
- [ ] `e4m3` type: 256 encodings, single NaN, no inf, saturation
- [ ] `e5m2` type: 256 encodings, IEEE-754 compliant inf/nan
- [ ] Conversion to/from float, double, cfloat, posit
- [ ] std::numeric_limits specializations
- [ ] Exhaustive value table tests for all sub-byte types
- [ ] Arithmetic regression tests (add, sub, mul, div)
- [ ] I/O manipulators (binary display, color-coded fields)

---

## Phase 2: OCP Microscaling Formats (`mxfloat`)

**Goal:** Implement the OCP MX v1.0 block floating-point formats.

**Timeline:** After Phase 1 (depends on microfloat element types)

### MX Block Structure

Each MX block consists of:
- **1 shared scale:** E8M0 (8 bits, power-of-two scaling)
- **32 elements:** one of the micro-float types
- **Total storage:** 8 + 32*d bits (where d = element width)

| Format       | Element  | Block Bytes | Effective bits/value |
|--------------|----------|-------------|----------------------|
| MXFP4 E2M1  | e2m1 (4) | 17          | 4.25                 |
| MXFP6 E3M2  | e3m2 (6) | 25          | 6.25                 |
| MXFP6 E2M3  | e2m3 (6) | 25          | 6.25                 |
| MXFP8 E4M3  | e4m3 (8) | 33          | 8.25                 |
| MXFP8 E5M2  | e5m2 (8) | 33          | 8.25                 |
| MXINT8       | int8 (8) | 33          | 8.25                 |

### Quantization Algorithm

```
scale = 2^(clamp(floor(log2(max(|x_i|))), -127, 127) - max_elem_exp)
q_i   = round_to_nearest_even(x_i / scale)
```

### Dequantization Algorithm

```
v_i = scale * q_i
// If scale == NaN (E8M0 = 0xFF), all v_i = NaN
```

### Design: Template Signature

```cpp
template<typename ElementType, size_t BlockSize = 32, typename BlockType = uint8_t>
class mxblock {
    e8m0           _scale;
    ElementType    _elements[BlockSize];
public:
    // Quantize from float array
    void quantize(const float* src, size_t n = BlockSize);
    // Dequantize to float array
    void dequantize(float* dst, size_t n = BlockSize) const;
    // Element access (read-only; modification requires requantize)
    float operator[](size_t i) const;
    // Block-level operations
    float dot(const mxblock& rhs) const;
};

// Convenience aliases
using mxfp4  = mxblock<e2m1, 32>;
using mxfp6  = mxblock<e3m2, 32>;   // or e2m3
using mxfp8  = mxblock<e4m3, 32>;   // or e5m2
using mxint8 = mxblock<int8_t, 32>;
```

### Directory Structure

```
include/sw/universal/number/mxfloat/
  mxfloat.hpp             # Public umbrella header
  mxfloat_fwd.hpp         # Forward declarations
  mxblock_impl.hpp        # Block container implementation
  quantize.hpp            # Quantization/dequantization algorithms
  mxfloat_traits.hpp      # Type traits
  numeric_limits.hpp      # Limits specializations
  exceptions.hpp
  attributes.hpp
  manipulators.hpp        # Block visualization
```

### Regression Tests

```
static/mxfloat/
  CMakeLists.txt
  api/
    api.cpp               # API walkthrough for all MX variants
    attributes.cpp
  conversion/
    quantize_fp4.cpp      # FP32 -> MXFP4 -> FP32 round-trip
    quantize_fp6.cpp
    quantize_fp8.cpp
    quantize_int8.cpp
  arithmetic/
    dot_product.cpp       # Block dot product accuracy
  accuracy/
    quantization_error.cpp  # Error analysis vs FP32 reference
```

### Deliverables

- [ ] `mxblock` template container with E8M0 shared scale
- [ ] Quantization: FP32 -> MX (scale computation, element rounding)
- [ ] Dequantization: MX -> FP32
- [ ] Block NaN propagation (E8M0 = 0xFF)
- [ ] Block dot product with FP32 accumulation
- [ ] Round-trip accuracy tests (quantize -> dequantize -> compare)
- [ ] Exhaustive tests for MXFP4 (small enough for full enumeration)
- [ ] Statistical error analysis for MXFP6, MXFP8
- [ ] I/O manipulators (display scale + elements)
- [ ] Mixed-element dot product (e.g., MXFP4 dot MXFP8)

---

## Phase 3: NVIDIA Block Formats (`nvblock`)

**Goal:** Implement NVIDIA's NVFP4 two-level block scaling format.

**Timeline:** After Phase 2 (shares element types, adds hierarchical scaling)

### NVFP4 Block Structure

NVFP4 differs from MXFP4 in three ways:
- **Block size:** 16 elements (vs 32 for MX)
- **Per-block scale:** E4M3 (non-power-of-two, finer granularity)
- **Per-tensor scale:** FP32 (two-level hierarchy)

```
Tensor:
  global_scale: float32 (1 per tensor)
  blocks[]:
    block_scale: e4m3 (1 per 16 elements)
    elements[16]: e2m1 (4 bits each, packed 2 per byte)
```

**Dequantization:**
```
x_hat[i] = global_scale * block_scale * dequantize(element[i])
```

**Effective storage:** 4.5 bits per value (4 + 8/16)

### Design: Template Signature

```cpp
template<typename ElementType, size_t BlockSize = 16, typename ScaleType = e4m3>
class nvblock {
    ScaleType      _block_scale;
    ElementType    _elements[BlockSize];
public:
    void quantize(const float* src, float tensor_scale, size_t n = BlockSize);
    void dequantize(float* dst, float tensor_scale, size_t n = BlockSize) const;
    float dot(const nvblock& rhs, float scale_a, float scale_b) const;
};

using nvfp4 = nvblock<e2m1, 16, e4m3>;
```

### Directory Structure

```
include/sw/universal/number/nvblock/
  nvblock.hpp
  nvblock_fwd.hpp
  nvblock_impl.hpp
  quantize.hpp
  nvblock_traits.hpp
  numeric_limits.hpp
  exceptions.hpp
  attributes.hpp
  manipulators.hpp
```

### Regression Tests

```
static/nvblock/
  CMakeLists.txt
  api/
    api.cpp
  conversion/
    quantize_nvfp4.cpp
    twolevel_scaling.cpp
  arithmetic/
    dot_product.cpp
  accuracy/
    mx_vs_nv_comparison.cpp   # Compare MXFP4 vs NVFP4 accuracy
```

### Deliverables

- [ ] `nvblock` template with per-block E4M3 scale
- [ ] Two-level quantization (tensor scale + block scale)
- [ ] Two-level dequantization
- [ ] Sub-byte packing (2 x e2m1 per byte)
- [ ] Block dot product with two-level scale factoring
- [ ] Accuracy comparison: NVFP4 vs MXFP4 on reference vectors
- [ ] Round-trip error analysis

---

## Phase 4: ZFP Compressed Floating-Point (`zfp`)

**Goal:** Implement ZFP block-transform compressed arrays as a Universal
number container, supporting 1D-4D arrays of float and double.

**Timeline:** Independent of Phases 1-3 (different algorithm family)

### ZFP Pipeline (per block of 4^d values)

```
float[] -> block-float conversion -> decorrelating transform
        -> coefficient reorder -> negabinary conversion
        -> bit-plane encoding -> rate-controlled truncation
```

### Compression Modes

| Mode            | Control Parameter    | Guarantee                    |
|-----------------|----------------------|------------------------------|
| Fixed-rate      | bits per block       | Predictable memory, random access |
| Fixed-precision | bit planes encoded   | Bounded relative error       |
| Fixed-accuracy  | absolute tolerance   | |f - g| <= tolerance         |
| Reversible      | (none)               | Bit-exact lossless           |

### Design Considerations

ZFP is fundamentally different from MX/NVFP4:
- It is a **compressed array container**, not a scalar or block-scalar type
- The core transform is a 4-point lifting scheme applied separably
- All internal computation is integer (after initial float-to-int conversion)
- Random access requires fixed-rate mode (blocks are independently decodable)

### Template Signature

```cpp
template<typename Scalar, size_t Dims>
class zfpblock {
    // Single block: encode/decode 4^Dims values
    static constexpr size_t block_size = ipow(4, Dims);
    // ...
};

template<typename Scalar, size_t Dims>
class zfparray {
    // Compressed array with block cache and proxy access
    // ...
};

// Convenience aliases
using zfparray1f = zfparray<float, 1>;
using zfparray2f = zfparray<float, 2>;
using zfparray3f = zfparray<float, 3>;
using zfparray1d = zfparray<double, 1>;
using zfparray2d = zfparray<double, 2>;
using zfparray3d = zfparray<double, 3>;
```

### Core Components

1. **Bitstream** -- bit-level I/O with random seek
2. **Forward/inverse lifting transform** (`fwd_lift` / `inv_lift`)
3. **Block-floating-point conversion** (`fwd_cast` / `inv_cast`)
4. **Negabinary conversion** (`int2uint` / `uint2int`)
5. **Coefficient reordering** (dimension-specific permutation tables)
6. **Embedded bit-plane codec** (group-test run-length encoding)
7. **Compressed array** with write-back block cache and proxy references

### Directory Structure

```
include/sw/universal/number/zfp/
  zfp.hpp                 # Public umbrella header
  zfp_fwd.hpp             # Forward declarations
  bitstream.hpp           # Bit-level I/O
  transform.hpp           # Forward/inverse lifting transform
  codec.hpp               # Bit-plane encode/decode
  zfpblock_impl.hpp       # Single-block compress/decompress
  zfparray_impl.hpp       # Compressed array container
  cache.hpp               # Write-back block cache
  proxy.hpp               # Proxy reference/pointer types
  zfp_traits.hpp
  exceptions.hpp
  attributes.hpp
  manipulators.hpp
```

### Regression Tests

```
static/zfp/
  CMakeLists.txt
  api/
    api.cpp
    block_api.cpp
    array_api.cpp
  codec/
    transform_1d.cpp       # Lift/unlift round-trip
    transform_2d.cpp
    transform_3d.cpp
    negabinary.cpp          # int2uint/uint2int round-trip
    bitplane.cpp            # Encode/decode bit planes
  compression/
    fixed_rate.cpp
    fixed_precision.cpp
    fixed_accuracy.cpp
    reversible.cpp
  accuracy/
    error_bounds.cpp        # Verify error guarantees
    compression_ratio.cpp
```

### Deliverables

- [ ] Bitstream class with bit-level read/write/seek
- [ ] 4-point lifting transform (forward and inverse)
- [ ] Block-floating-point conversion (float->int, int->float)
- [ ] Negabinary conversion
- [ ] Coefficient reordering (1D, 2D, 3D permutation tables)
- [ ] Embedded bit-plane codec
- [ ] `zfpblock`: single-block compress/decompress for 1D, 2D, 3D
- [ ] Fixed-rate mode
- [ ] Fixed-precision mode
- [ ] Fixed-accuracy mode
- [ ] Reversible (lossless) mode
- [ ] `zfparray`: compressed array with block cache
- [ ] Proxy reference/pointer types for transparent element access
- [ ] Iterator support
- [ ] Round-trip accuracy tests for all modes
- [ ] Error bound verification for fixed-accuracy mode
- [ ] Compression ratio benchmarks

---

## Governance and Accountability

### Status Tracking

Each deliverable uses a three-state progression:

| State         | Meaning                                        |
|---------------|------------------------------------------------|
| `[ ]` Pending | Not started                                    |
| `[~]` Active  | Implementation in progress                     |
| `[x]` Done    | Implemented, tested, merged                    |

### Phase Gates

A phase is **complete** when:
1. All deliverables are `[x]`
2. All regression tests pass at REGRESSION_LEVEL_1
3. `api.cpp` demonstrates all public API patterns
4. The type is registered in `number_systems.hpp` (or equivalent)

A phase is **ready to start** when all its dependency phases have
passed their phase gate.

### Dependency Graph

```
Phase 1: microfloat (element types)
    |
    +---> Phase 2: mxfloat (OCP MX blocks)
    |         |
    |         +---> Phase 3: nvblock (NVIDIA NVFP4)
    |
Phase 4: zfp (independent, can start in parallel with Phase 1)
```

### Quality Criteria

**Per-type minimum requirements:**
- Exhaustive value table test for types with <= 256 encodings
- Round-trip conversion test: native -> type -> native
- Arithmetic correctness vs FP32 reference
- std::numeric_limits specialization
- api.cpp walkthrough demonstrating all public interfaces

**Per-block-format minimum requirements:**
- Quantization/dequantization round-trip accuracy test
- Block dot product correctness vs FP64 reference
- NaN and special-value propagation tests
- Error distribution analysis (mean, max, std)

### Review Checklist

For each PR introducing a new type or format:

- [ ] Follows Universal directory structure conventions
- [ ] Header-only, no external dependencies
- [ ] C++20 compatible
- [ ] All regression tests pass (`cmake -DUNIVERSAL_BUILD_REGRESSION_SANITY=ON`)
- [ ] api.cpp demonstrates the complete public API
- [ ] numeric_limits specialization provided
- [ ] I/O manipulators show internal representation
- [ ] No compiler warnings with `-Wall -Wpedantic`

### Milestone Summary

| Milestone | Phase | Key Deliverable                     |
|-----------|-------|-------------------------------------|
| M1        | 1     | All 6 micro-float element types     |
| M2        | 2     | MXFP4/MXFP6/MXFP8 quantize + dot   |
| M3        | 3     | NVFP4 two-level scaling + dot       |
| M4        | 4a    | ZFP single-block codec (1D/2D/3D)   |
| M5        | 4b    | ZFP compressed array container      |

---

## References

- [OCP Microscaling Formats (MX) v1.0 Specification](https://www.opencompute.org/documents/ocp-microscaling-formats-mx-v1-0-spec-final-pdf)
- [Microsoft microxcaling Reference Implementation](https://github.com/microsoft/microxcaling)
- [Microscaling Data Formats for Deep Learning (arXiv:2310.10537)](https://arxiv.org/pdf/2310.10537)
- [NVIDIA Blog: Introducing NVFP4](https://developer.nvidia.com/blog/introducing-nvfp4-for-efficient-and-accurate-low-precision-inference/)
- [CUDA Math API: __nv_fp4_e2m1](https://docs.nvidia.com/cuda/cuda-math-api/cuda_math_api/struct____nv__fp4__e2m1.html)
- [LLNL ZFP Project](https://computing.llnl.gov/projects/zfp)
- [ZFP GitHub Repository](https://github.com/LLNL/zfp)
- [Lindstrom, "Fixed-Rate Compressed Floating-Point Arrays," IEEE TVCG, 2014](https://vis.cs.ucdavis.edu/vis2014papers/TVCG/papers/2674_20tvcg12-lindstrom-2346458.pdf)
- [ZFP Algorithm Documentation](https://zfp.readthedocs.io/en/release1.0.1/algorithm.html)
