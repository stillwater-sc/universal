# Areal: Faithful Floating-Point with Uncertainty Bit

## Why

IEEE-754 floating-point silently rounds every result to the nearest representable value. After a chain of operations, you have no idea how much rounding error has accumulated -- the final answer looks just as precise as every intermediate result, even if it's completely wrong. The only way to discover the error is to re-run the computation in higher precision, which is expensive and often impractical.

The `areal` type solves this with a single-bit innovation: the **uncertainty bit (ubit)**. The least significant bit of every areal value indicates whether the value is exact or approximate. When an operation produces a result that falls between two representable values, the ubit is set to 1, meaning "the true value lies between this encoding and the next." You get faithful floating-point arithmetic where every result honestly reports whether it was rounded.

## What

`areal<nbits, es, bt>` is a faithful floating-point with an uncertainty bit:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Total bits (minimum: es + 3) |
| `es` | `unsigned` | -- | Exponent bits |
| `bt` | typename | `uint8_t` | Storage block type |

### Encoding

```
[sign : 1 bit] [exponent : es bits] [fraction : fbits] [ubit : 1 bit]
```

Where `fbits = nbits - 2 - es` (the uncertainty bit takes one bit from what would be fraction in a standard float).

### The Uncertainty Bit

- **ubit = 0**: The value is *exactly* the represented floating-point value
- **ubit = 1**: The true value lies *strictly between* this encoding and the next representable value

This provides a faithful bound: the true result is always within one ULP of the stored value, and you *know* when it's not exact.

### Key Properties

- **Faithful rounding**: every result is within 1 ULP of the true value, with ubit indicating exactness
- **Gradual underflow**: subnormal numbers for smooth transition to zero
- **Gradual overflow**: values beyond maxpos mapped with ubit=1
- **No rounding modes**: the ubit replaces the complexity of IEEE rounding modes
- **Configurable precision**: any combination of nbits and es

## How It Works

When an arithmetic operation produces a result that is exactly representable, the result is stored with ubit=0. When the result falls between two consecutive representable values, the lower value is stored with ubit=1, indicating "the true value is between here and the next encoding." This is simpler than IEEE rounding modes and provides strictly more information: you always know whether the result was exact.

The overflow behavior is also graceful: instead of jumping to infinity, an areal beyond maxpos is stored as maxpos with ubit=1, meaning "the true value is somewhere above maxpos." Similarly, underflow toward zero sets the ubit to indicate imprecision near the bottom of the range.

## How to Use It

### Include

```cpp
#include <universal/number/areal/areal.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
areal<8, 2> a(1.0f);    // Exact: ubit = 0
areal<8, 2> b(0.1f);    // Not exactly representable: ubit = 1

auto c = a + b;
std::cout << to_binary(c) << " = " << c << std::endl;
// The ubit tells you whether this result is exact
```

### Verified Computation

```cpp
template<typename Real>
bool is_result_exact(Real a, Real b) {
    Real result = a * b;
    // Check the uncertainty bit to verify exactness
    return !result.test(0);  // ubit is bit 0
}

areal<16, 5> x(2.0f), y(3.0f);
// 2.0 * 3.0 = 6.0, which is exactly representable
assert(is_result_exact(x, y));

areal<16, 5> p(1.0f), q(3.0f);
// 1.0 / 3.0 is not exactly representable
// The result will have ubit = 1
```

### Tracking Precision Loss

```cpp
// Count how many operations in a chain produce inexact results
template<typename Real>
size_t count_roundings(const std::vector<Real>& values) {
    size_t inexact_count = 0;
    Real sum(0);
    for (const auto& v : values) {
        sum += v;
        if (sum.test(0)) ++inexact_count;  // ubit set means rounding occurred
    }
    return inexact_count;
}
```

## Problems It Solves

| Problem | How areal Solves It |
|---------|-----------------------|
| No way to know if a floating-point result was rounded | Uncertainty bit explicitly marks inexact results |
| IEEE rounding modes are complex and rarely used correctly | Single ubit replaces all rounding mode logic |
| Overflow jumps to infinity, destroying information | Gradual overflow with ubit=1 preserves "above maxpos" |
| Underflow flushes to zero prematurely | Gradual underflow with subnormals + ubit |
| Validated numerics requires expensive interval arithmetic | Single extra bit provides faithful bounds |
| Reproducibility debates about rounding mode choices | Ubit is deterministic: no mode selection needed |
