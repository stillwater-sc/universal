# Universal Number Systems Guide

This directory contains comprehensive documentation for each number system in the Universal library. Each document explains **why** the number system exists, **what** it does, and **how** to use it to solve specific problems.

## Number Systems by Category

### Integer and Fixed-Point

| Type | Bits | Description | Best For |
|------|------|-------------|----------|
| [integer](integer.md) | N | Arbitrary-width signed integer | Cryptography, combinatorics, wide counters |
| [fixpnt](fixpnt.md) | N | Binary fixed-point with configurable radix | DSP, control systems, embedded (no FPU) |
| [rational](rational.md) | 2N | Exact numerator/denominator fraction | Symbolic math, exact geometry, financial |

### Configurable Floating-Point

| Type | Bits | Description | Best For |
|------|------|-------------|----------|
| [cfloat](cfloat.md) | 4-256 | Fully parameterized IEEE-compatible float | Mixed-precision research, custom HW design |
| [bfloat16](bfloat16.md) | 16 | Google Brain Float (8-bit exponent, 7-bit fraction) | Neural network training, TPU workloads |
| [areal](areal.md) | N | Faithful float with uncertainty bit | Verified computing, uncertainty tracking |
| [dfloat](dfloat.md) | N | Decimal floating-point (base-10) | Financial systems, regulatory compliance |

### Micro-Precision and Block-Scaled (AI Quantization)

| Type | Bits | Description | Best For |
|------|------|-------------|----------|
| [microfloat](microfloat.md) | 4-8 | OCP MX element types (e2m1, e4m3, e5m2) | AI model elements, quantization validation |
| [e8m0](e8m0.md) | 8 | Exponent-only power-of-two scale | Block scale factor for MX format |
| [mxfloat](mxfloat.md) | Block | OCP Microscaling block format | AI inference, model compression (OCP) |
| [nvblock](nvblock.md) | Block | NVIDIA NVFP4 block format | GPU inference, NVIDIA accelerators |

### Posit Family (UNUM Type III)

| Type | Bits | Description | Best For |
|------|------|-------------|----------|
| [posit](posit.md) | N | Tapered-precision floating-point (current v2) | General numeric, more precision than IEEE |
| [posit1](posit1.md) | N | Original posit implementation (legacy v1) | Quire/FDP support, backward compatibility |
| [posito](posito.md) | N | Experimental posit variant | Differential testing, research |
| [quire](quire.md) | Wide | Super-accumulator for exact dot products | Reproducible linear algebra, BLAS |
| [takum](takum.md) | N | Bounded-range tapered float | General computing, predictable range |

### Interval and Uncertainty Arithmetic

| Type | Bits | Description | Best For |
|------|------|-------------|----------|
| [valid](valid.md) | 2N | Interval arithmetic with posit-encoded bounds | Verified computing with posit precision |
| [interval](interval.md) | 2N | Generic interval over any scalar type | Tolerance analysis, uncertainty propagation |
| [sorn](sorn.md) | N | Set of operand range numbers | Rigorous uncertainty, safety-critical bounds |
| [unum2](unum2.md) | N | Configurable exact-value lattice | Research, custom value distributions |

### Logarithmic Number Systems

| Type | Bits | Description | Best For |
|------|------|-------------|----------|
| [lns](lns.md) | N | Single-base logarithmic (base 2) | DSP, multiply-heavy workloads, low-power HW |
| [dbns](dbns.md) | N | Double-base logarithmic (base 0.5 and 3) | Research, mixed-radix applications |

### Extended Precision (Multi-Component)

| Type | Bits | Decimal Digits | Description | Best For |
|------|------|---------------|-------------|----------|
| [dd](dd.md) | 128 | ~31 | Double-double (2 doubles) | Extended precision, ill-conditioned systems |
| [qd](qd.md) | 256 | ~64 | Quad-double (4 doubles) | Ultra-high precision, constant computation |
| [dd_cascade](dd_cascade.md) | 128 | ~31 | DD via unified cascade framework | Consistent API across precision tiers |
| [td_cascade](td_cascade.md) | 192 | ~48 | Triple-double (3 doubles) | Intermediate precision tier |
| [qd_cascade](qd_cascade.md) | 256 | ~64 | QD via unified cascade framework | Consistent API across precision tiers |

### Compressed Floating-Point

| Type | Description | Best For |
|------|-------------|----------|
| [zfpblock](zfpblock.md) | ZFP block-based float compression (1D/2D/3D) | Scientific data storage, simulation checkpoints |

### Complex Number Support

| Type | Description | Best For |
|------|-------------|----------|
| [complex](complex.md) | Complex arithmetic for any Universal scalar | FFT, signal processing, quantum computing |

## Choosing a Number System

### By Application Domain

| Domain | Recommended Types |
|--------|-------------------|
| **Deep Learning Inference** | microfloat, mxfloat, nvblock, bfloat16, cfloat(fp8) |
| **Deep Learning Training** | bfloat16, cfloat(fp16/fp32), posit |
| **DSP / Signal Processing** | fixpnt, lns, complex |
| **Financial / Accounting** | dfloat, rational, fixpnt |
| **Embedded (no FPU)** | fixpnt, integer |
| **Scientific HPC** | dd, qd, posit, cfloat |
| **Verified / Validated Computing** | interval, valid, areal, sorn |
| **Reproducible Linear Algebra** | posit + quire |
| **Cryptography / Big Numbers** | integer |
| **Data Compression** | zfpblock |
| **Custom Hardware Design** | cfloat, posit, takum, lns |

### By Precision Need

| Precision | Type | Decimal Digits |
|-----------|------|---------------|
| 2 digits | bfloat16 | ~2 |
| 3 digits | cfloat(fp8), microfloat | ~2-3 |
| 7 digits | cfloat(fp32), posit<32,2> | ~7-8 |
| 16 digits | cfloat(fp64), double | ~16 |
| 31 digits | dd, dd_cascade | ~31 |
| 48 digits | td_cascade | ~48 |
| 64 digits | qd, qd_cascade | ~64 |
| Exact | rational, integer, quire | Unlimited (within nbits) |

## Quick Start

Every number system is header-only. Include the type and start computing:

```cpp
#include <universal/number/posit/posit.hpp>  // or any type
using namespace sw::universal;

// Plug-in replacement pattern
template<typename Real>
Real my_algorithm(Real a, Real b) {
    return (a + b) * (a - b);
}

// Use with any Universal type
auto r1 = my_algorithm(posit<32,2>(3.0), posit<32,2>(4.0));
auto r2 = my_algorithm(cfloat<16,5,uint16_t,true,false,false>(3.0),
                        cfloat<16,5,uint16_t,true,false,false>(4.0));
auto r3 = my_algorithm(dd(3.0), dd(4.0));
```

For detailed usage patterns, see the `api/api.cpp` test file in each number system's regression test directory under `static/`.
