# Posit v1 (Legacy): Original Reference Implementation

## Why

The posit v1 implementation (`posit1`) is the original reference implementation of John Gustafson's posit arithmetic, dating to ~2017. It is maintained for backward compatibility and is the only posit implementation that provides full quire (super-accumulator) support with fused dot product (FDP) operations.

If you are starting a new project, use `posit<nbits, es, bt>` (the current v2 implementation). Use `posit1` only if you need:
- Compatibility with code written against the original posit API
- The original quire implementation with full FDP support
- Specialized posit configurations (posit_2_0, posit_3_0, etc.)

## What

`posit<nbits, es>` (in the `posit1` namespace) is the original two-parameter posit:

| Parameter | Type | Description |
|-----------|------|-------------|
| `nbits` | `unsigned` | Total bits |
| `es` | `unsigned` | Maximum exponent bits |

Note: unlike the current posit (v2), posit1 does not have a `BlockType` template parameter. It uses `bitblock<>` internally with `uint64_t` storage.

### Key Differences from Posit v2

| Feature | Posit v1 (posit1) | Posit v2 (posit) |
|---------|-------------------|------------------|
| Template params | 2: nbits, es | 3: nbits, es, bt |
| Internal storage | bitblock<> (legacy) | blockbinary<> (modern) |
| Arithmetic engine | internal::value<> | blocktriple<> |
| Block type | Fixed uint64_t | Configurable (uint8_t-uint64_t) |
| Quire | Full support | Bridge header available |
| Status | Legacy/maintenance | Active development |

### Quire (Super-Accumulator)

The quire is the distinguishing feature of posit1:

```cpp
template<unsigned nbits, unsigned es, unsigned capacity = 2>
class quire;
```

The quire is a fixed-point accumulator wide enough to hold the exact sum of any number of posit products without intermediate rounding. For `posit<32, 2>`, the quire is approximately 512 bits wide.

## How to Use It

### Include

```cpp
#include <universal/number/posit1/posit1.hpp>
using namespace sw::universal;
```

### Exact Dot Product with Quire

```cpp
using Posit = posit<32, 2>;
using Quire = quire<32, 2>;

std::vector<Posit> a = { Posit(1.0), Posit(2.0), Posit(3.0) };
std::vector<Posit> b = { Posit(4.0), Posit(5.0), Posit(6.0) };

Quire q;
q.clear();
for (size_t i = 0; i < a.size(); ++i) {
    q += quire_mul(a[i], b[i]);  // Exact: no intermediate rounding
}

Posit result;
convert(q.to_value(), result);
// result = 32.0 exactly (1*4 + 2*5 + 3*6)
```

### Fused Dot Product (FDP)

```cpp
// fdp: fused dot product -- accumulate in quire, round once at the end
Posit fdp_result = fdp(a, b);  // Equivalent to quire-based accumulation
```

## When to Use Posit v1 vs Posit v2

| Use Case | Recommendation |
|----------|----------------|
| New project, general posit arithmetic | Use `posit` (v2) |
| Need configurable block type for hardware | Use `posit` (v2) |
| Need full quire/FDP support | Use `posit1` |
| Backward compatibility with existing code | Use `posit1` |
| Reproducible linear algebra (BLAS) | Use `posit1` with quire |
| Mixed-precision algorithm exploration | Use `posit` (v2) |
