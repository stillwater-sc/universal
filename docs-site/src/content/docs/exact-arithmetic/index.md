---
title: Exact Arithmetic
description: Exact dot products and reproducible linear algebra via the Kulisch super-accumulator and the generalized quire
sidebar:
  order: 0
---

The most fundamental operation in numerical computing — the dot product — is also the primary source of precision loss and non-reproducibility in floating-point arithmetic.

The **Kulisch super-accumulator** eliminates this problem at its root: a fixed-point register wide enough to hold the exact sum of any number of floating-point products, with only a single rounding operation at the very end.

The Universal Numbers Library implements this concept as the **generalized quire** — a C++ template that provides exact dot products for *any* number system: IEEE-style floats, posits, fixed-point, logarithmic, and double-base.

## In This Section

- **[The Kulisch Super-Accumulator](/universal/exact-arithmetic/kulisch-super-accumulator/)** — History, theory, and practice of exact dot products, from Kulisch's original work through the Universal library's generalized quire. Includes worked examples, code, and a comprehensive reference list.

- **[Quire API Reference](/universal/number-systems/quire/)** — Template parameters, operations, and usage patterns for the quire type.

## Quick Start

```cpp
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>

using Real = sw::universal::cfloat<32, 8, uint32_t, true, false, false>;

std::vector<Real> x = { 1e8f, -1e8f, 0.5f, 0.25f };
std::vector<Real> y = { 1.0f,  1.0f, 1.0f, 1.0f  };

// Naive accumulation: 0.0 (WRONG — residual lost to cancellation)
// Fused dot product:  0.75 (CORRECT — exact via quire)
Real result = sw::universal::fdp(x, y);
```

## Supported Number Systems

| Type | Header | FDP Available |
|------|--------|--------------|
| `cfloat` | `<universal/number/cfloat/fdp.hpp>` | Yes |
| `posit` | `<universal/number/posit/fdp.hpp>` | Yes |
| `fixpnt` | `<universal/number/fixpnt/fdp.hpp>` | Yes |
| `lns` | `<universal/number/lns/fdp.hpp>` | Yes |
| `dbns` | `<universal/number/dbns/fdp.hpp>` | Yes |
