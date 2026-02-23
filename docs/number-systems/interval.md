# Interval: Generic Interval Arithmetic

## Why

Every floating-point operation introduces rounding error, but the error is invisible -- the result looks like an exact number. After thousands of operations, the accumulated error can be larger than the result itself, and you have no way to know. Interval arithmetic makes the error visible: instead of a single number, you carry a guaranteed lower and upper bound. If the interval is narrow, your result is accurate. If it's wide, the computation is unstable and you need a different approach.

The Universal `interval` type is generic over its scalar type: you can build intervals of `float`, `double`, `posit`, `cfloat`, `dd`, or any other Universal number type. This lets you add rigorous bounds to any computation by changing one `using` declaration.

## What

`interval<Scalar>` is a closed interval `[lo, hi]`:

| Parameter | Type | Description |
|-----------|------|-------------|
| `Scalar` | typename | Any arithmetic type (float, double, posit, cfloat, dd, etc.) |

### Internal Structure

```cpp
template<typename Scalar>
class interval {
    Scalar _lo;  // Lower bound
    Scalar _hi;  // Upper bound
    // Invariant: _lo <= _hi
};
```

### Key Properties

- **Guaranteed containment**: the true mathematical result always lies within [lo, hi]
- **Generic scalar**: works with any Universal number type
- **Closed intervals**: both bounds are included
- **Degenerate intervals**: [v, v] represents an exact point value
- **Standard interval algebra**: follows IEEE 1788-2015 principles

### Interval Queries

| Method | Returns |
|--------|---------|
| `mid()` | Midpoint: (lo + hi) / 2 |
| `rad()` | Radius: (hi - lo) / 2 |
| `width()` | Width: hi - lo |
| `mag()` | Magnitude: max(\|lo\|, \|hi\|) |
| `mig()` | Mignitude: min(\|lo\|, \|hi\|) if doesn't contain 0, else 0 |
| `contains_zero()` | Does the interval straddle zero? |
| `ispos()` | Entirely positive? |
| `isneg()` | Entirely negative? |

## How It Works

Interval arithmetic computes bounds that are guaranteed to contain the true result:

| Operation | Result |
|-----------|--------|
| `[a,b] + [c,d]` | `[a+c, b+d]` |
| `[a,b] - [c,d]` | `[a-d, b-c]` |
| `[a,b] × [c,d]` | `[min(ac,ad,bc,bd), max(ac,ad,bc,bd)]` |
| `[a,b] / [c,d]` | `[a,b] × [1/d, 1/c]` (when 0 not in [c,d]) |

For correct results, the lower bound should be rounded *downward* and the upper bound *upward*, ensuring the interval only grows (never shrinks) and always contains the true value.

The **dependency problem** is interval arithmetic's main limitation: when the same variable appears multiple times in an expression, intervals treat each occurrence as independent, leading to overestimation. For example, `x - x` for `x ∈ [1,2]` produces `[-1, 1]` instead of the correct `[0, 0]`.

## How to Use It

### Include

```cpp
#include <universal/number/interval/interval.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
interval<double> a(1.0, 2.0);    // x is somewhere in [1, 2]
interval<double> b(3.0, 4.0);    // y is somewhere in [3, 4]

auto sum = a + b;                  // [4, 6]
auto product = a * b;              // [3, 8]
auto ratio = a / b;                // [0.25, 0.666...]

std::cout << "sum:     " << sum << std::endl;
std::cout << "product: " << product << std::endl;
std::cout << "width:   " << product.width() << std::endl;
```

### Uncertainty Propagation in Engineering

```cpp
// Resistor voltage divider with component tolerances
// V_out = V_in × R2 / (R1 + R2)
interval<double> V_in(4.95, 5.05);     // 5V ± 1%
interval<double> R1(990.0, 1010.0);    // 1kΩ ± 1%
interval<double> R2(1980.0, 2020.0);   // 2kΩ ± 1%

auto V_out = V_in * R2 / (R1 + R2);
std::cout << "Output voltage: " << V_out << std::endl;
std::cout << "Nominal: " << V_out.mid() << std::endl;
std::cout << "Tolerance: ±" << V_out.rad() << std::endl;
```

### With Posit Scalars

```cpp
// Interval arithmetic with tapered-precision bounds
using P32 = posit<32, 2>;
interval<P32> x(P32(0.9), P32(1.1));   // x near 1.0 where posit has max precision
interval<P32> y(P32(0.95), P32(1.05));

auto result = x * y;
std::cout << "Posit interval result: " << result << std::endl;
// Tighter bounds than interval<float> because posit has more precision near 1.0
```

### Validated Root Finding

```cpp
// Prove that a root exists in an interval using the intermediate value theorem
template<typename Scalar>
bool root_exists(interval<Scalar> x) {
    // f(x) = x^2 - 2 (root at sqrt(2))
    auto fx = x * x - interval<Scalar>(Scalar(2), Scalar(2));
    return fx.contains_zero();
}

interval<double> search(1.0, 2.0);
if (root_exists(search)) {
    std::cout << "Root of x^2-2 exists in " << search << std::endl;
}
// This is a mathematical PROOF that sqrt(2) is in [1, 2]
```

### Detecting Numerical Instability

```cpp
template<typename Scalar>
interval<Scalar> unstable_formula(Scalar x) {
    // (1 - cos(x)) / x^2 is unstable near x=0
    interval<Scalar> ix(x, x);  // Point interval
    auto one = interval<Scalar>(Scalar(1), Scalar(1));
    auto result = (one - cos(ix)) / (ix * ix);
    return result;
}

// Wide interval = unstable computation
auto r1 = unstable_formula(1.0);      // Narrow: stable
auto r2 = unstable_formula(1e-8);     // Wide: catastrophic cancellation detected
std::cout << "x=1.0:  width=" << r1.width() << std::endl;
std::cout << "x=1e-8: width=" << r2.width() << std::endl;
```

## Problems It Solves

| Problem | How interval Solves It |
|---------|-----------------------|
| No way to know if floating-point result is accurate | Interval width directly measures uncertainty |
| Engineering tolerance analysis requires Monte Carlo | Deterministic interval propagation, no sampling needed |
| Need mathematical proof that root/solution exists | Containment test proves existence by IVT |
| Detecting catastrophic cancellation | Wide output interval = numerical instability |
| Different scalar types need different uncertainty analysis | Generic over any Universal number type |
| Accumulated rounding error is invisible | Interval bounds make error growth visible |
