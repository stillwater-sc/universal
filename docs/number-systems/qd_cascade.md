# QD Cascade: Quad-Double via Unified Float Cascade Framework

## Why

The classic `qd` type provides ~212 bits of precision using four doubles. The `qd_cascade` type achieves the same precision but is built on the unified `floatcascade<N>` template framework, sharing its implementation with `dd_cascade` and `td_cascade`. This means one well-tested code path handles all three precision tiers.

Use `qd_cascade` when you want quad-double precision within the modern cascade framework, or when your application needs to dynamically select between DD, TD, and QD precision using a consistent interface.

## What

`qd_cascade` wraps `floatcascade<4>`:

| Property | Value |
|----------|-------|
| Components | 4 doubles |
| Total bits | 256 |
| Significand | ~212 bits (~63.8 decimal digits) |
| Epsilon | ~1.22 × 10^-63 |
| Framework | `floatcascade<4>` |

### Key Properties

- **Same precision as classic `qd`**: ~212-bit significand, ~64 decimal digits
- **Unified with dd_cascade and td_cascade**: same API, same algorithms
- **Array-based access**: `operator[]` for individual component manipulation
- **Reverse-order evaluation**: `e[3] + e[2] + e[1] + e[0]` preserves tail precision
- **Fortified arithmetic**: volatile modifiers prevent unsafe compiler optimizations

## How to Use It

### Include

```cpp
#include <universal/number/qd_cascade/qd_cascade.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
qd_cascade pi("3.14159265358979323846264338327950288419716939937510");
qd_cascade e("2.71828182845904523536028747135266249775724709369995");

qd_cascade result = pi * e;
std::cout << std::setprecision(64) << result << std::endl;
// Accurate to ~63 decimal digits
```

### Component Access

```cpp
qd_cascade val(3.14159265358979);
std::cout << "Component 0 (hi):  " << val[0] << std::endl;
std::cout << "Component 1:       " << val[1] << std::endl;
std::cout << "Component 2:       " << val[2] << std::endl;
std::cout << "Component 3 (lo):  " << val[3] << std::endl;
```

### Multi-Tier Precision Selection

```cpp
// Template function works with any cascade tier
template<typename Cascade>
Cascade compute_e(int terms) {
    Cascade sum(1.0);
    Cascade factorial(1.0);
    for (int i = 1; i < terms; ++i) {
        factorial *= Cascade(i);
        sum += Cascade(1.0) / factorial;
    }
    return sum;
}

auto e_dd = compute_e<dd_cascade>(30);   // ~31 digits
auto e_td = compute_e<td_cascade>(50);   // ~47 digits
auto e_qd = compute_e<qd_cascade>(70);   // ~63 digits
```

## When to Use qd_cascade vs qd

| Use Case | Recommendation |
|----------|----------------|
| Standalone quad-double computation | Either works; `qd` is more established |
| Mixing dd/td/qd precision in same codebase | Use cascade family for consistent API |
| Need component-level array access | Use `qd_cascade` |
| Template code parameterized by precision tier | Use cascade family |
| Backward compatibility with existing qd code | Use classic `qd` |
