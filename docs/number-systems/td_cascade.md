# TD Cascade: Triple-Double Precision

## Why

Sometimes double-double (~31 decimal digits) is not enough, but quad-double (~64 digits) is more than you need. Triple-double fills the gap: ~47 decimal digits using three doubles, at roughly 75% of the cost of quad-double. This precision tier is particularly useful for intermediate computations where you need more than DD for accumulation accuracy but don't want to pay the full QD cost.

## What

`td_cascade` wraps `floatcascade<3>`:

| Property | Value |
|----------|-------|
| Components | 3 doubles |
| Total bits | 192 |
| Significand | ~159 bits (~47.8 decimal digits) |
| Epsilon | ~1.39 × 10^-48 |
| Framework | `floatcascade<3>` |

### Key Properties

- **Triple the precision of double**: 159-bit significand
- **Intermediate tier**: fills the gap between DD (106 bits) and QD (212 bits)
- **Unified cascade API**: same interface as dd_cascade and qd_cascade
- **Memory efficient**: 24 bytes vs QD's 32 bytes
- **Same dynamic range as double**: 10^-308 to 10^308

### Precision Hierarchy

| Type | Decimal Digits | Storage | Relative Cost |
|------|---------------|---------|---------------|
| double | 15.9 | 8 bytes | 1x |
| dd_cascade | 31.4 | 16 bytes | ~10x |
| **td_cascade** | **47.8** | **24 bytes** | **~25x** |
| qd_cascade | 63.8 | 32 bytes | ~50x |

## How to Use It

### Include

```cpp
#include <universal/number/td_cascade/td_cascade.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
td_cascade a("3.14159265358979323846264338327950288");
td_cascade b("2.71828182845904523536028747135266249");

td_cascade product = a * b;
std::cout << std::setprecision(48) << product << std::endl;
// Accurate to ~47 decimal digits
```

### Extended Accumulation

```cpp
// When DD accumulation isn't precise enough but QD is overkill
td_cascade sum(0.0);
for (int i = 1; i <= 10000000; ++i) {
    sum += td_cascade(1.0) / td_cascade(i);
}
// Harmonic series sum with ~47 digits of accuracy
```

## Problems It Solves

| Problem | How td_cascade Solves It |
|---------|-----------------------|
| DD's 31 digits insufficient, QD's 64 digits wasteful | 47 digits: the right precision for the problem |
| Need more than DD for accumulation in iterative methods | Triple precision prevents error accumulation |
| Memory-constrained extended precision | 24 bytes vs QD's 32 bytes (25% savings) |
| Consistent API across precision levels | Same interface as dd_cascade and qd_cascade |
