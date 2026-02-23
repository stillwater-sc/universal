# DD Cascade: Double-Double via Unified Float Cascade Framework

## Why

The classic `dd` type provides ~106 bits of precision using two doubles. The `dd_cascade` type achieves the same precision but is built on the unified `floatcascade<N>` template framework that also underlies `td_cascade` (triple-double) and `qd_cascade` (quad-double). This shared architecture means consistent APIs, shared optimizations, and easy switching between precision tiers.

Use `dd_cascade` when you want double-double precision within the modern cascade framework, or when you need to mix dd, td, and qd precision levels using a consistent interface.

## What

`dd_cascade` wraps `floatcascade<2>`:

| Property | Value |
|----------|-------|
| Components | 2 doubles |
| Total bits | 128 |
| Significand | ~106 bits (~31.4 decimal digits) |
| Epsilon | ~4.93 × 10^-32 |
| Framework | `floatcascade<2>` |

### Key Properties

- **Unified with td_cascade and qd_cascade**: same API, same algorithms, different N
- **Identical precision to classic `dd`**: both provide ~106-bit significand
- **Array-based access**: `operator[]` for component access
- **Fortified arithmetic**: volatile modifiers prevent unsafe compiler reordering
- **Modern C++20**: leverages constexpr and modern template patterns

## How to Use It

### Include

```cpp
#include <universal/number/dd_cascade/dd_cascade.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
dd_cascade a(1.0, 1e-17);  // High component + residual
dd_cascade b(2.0, 2e-17);
dd_cascade sum = a + b;

std::cout << to_components(sum) << std::endl;
std::cout << std::setprecision(32) << sum << std::endl;
```

### Precision Tier Switching

```cpp
// Easy to scale up precision using the same framework
dd_cascade  result_2x(0.0);  // ~31 decimal digits
// td_cascade result_3x(0.0);  // ~47 decimal digits (if needed)
// qd_cascade result_4x(0.0);  // ~64 decimal digits (if needed)

// Same API for all cascade types
```

## When to Use dd_cascade vs dd

| Use Case | Recommendation |
|----------|----------------|
| Standalone double-double computation | Either works; `dd` is simpler |
| Mixing dd/td/qd precision tiers | Use cascade family for consistent API |
| Need component-level array access | Use `dd_cascade` (operator[]) |
| Integration with new code using cascade framework | Use `dd_cascade` |
| Backward compatibility with existing dd code | Use classic `dd` |
