# Valid: Interval Arithmetic with Posit Encoding

## Why

Floating-point computation gives you a single number as the answer, but the true mathematical result almost always lies *between* representable values. How far off is the answer? Standard floating-point doesn't tell you. You need to run the computation a second time in higher precision, or perform a separate error analysis, to have any idea of the accuracy.

The `valid` type pairs two posit values as interval bounds, rigorously tracking where the true result must lie. Every operation produces an interval that is *guaranteed* to contain the exact mathematical answer. This is verified computing: the output is not just an approximation, but a proven bound.

## What

`valid<nbits, es>` stores an interval as a pair of posits:

| Parameter | Type | Description |
|-----------|------|-------------|
| `nbits` | `unsigned` | Bits per posit bound |
| `es` | `unsigned` | Exponent bits per posit bound |

### Internal Structure

- **Lower bound** (`lb`): posit<nbits, es>
- **Upper bound** (`ub`): posit<nbits, es>
- **Lower ubit** (`lubit`): is the lower bound open?
- **Upper ubit** (`uubit`): is the upper bound open?
- Total storage: 2 × nbits + 2 flag bits

### Interval Types

- **`[lb, ub]`**: closed interval, both bounds included
- **`(lb, ub]`**: open lower bound
- **`[lb, ub)`**: open upper bound
- **`(lb, ub)`**: open interval, both bounds excluded
- **Degenerate**: `[v, v]` is a point value (exact)

### Key Properties

- **Guaranteed containment**: true result always lies within the interval
- **Posit-encoded bounds**: benefits from tapered precision for tight bounds near 1.0
- **Uncertainty quantification**: interval width measures result uncertainty
- **UNUM Type II concept**: implements Gustafson's uncertainty tracking with posit encoding
- **Rigorous arithmetic**: interval operations follow standard inclusion algebra

## How It Works

Interval arithmetic rules ensure that every operation produces bounds that contain the true result:

- **Addition**: `[a,b] + [c,d] = [a+c, b+d]` (round lower bound down, upper bound up)
- **Subtraction**: `[a,b] - [c,d] = [a-d, b-c]`
- **Multiplication**: `[a,b] × [c,d] = [min(ac,ad,bc,bd), max(ac,ad,bc,bd)]`
- **Division**: `[a,b] / [c,d] = [a,b] × [1/d, 1/c]` (when 0 not in [c,d])

The ubit flags track whether bounds are open or closed, providing tighter containment than closed-only intervals.

## How to Use It

### Include

```cpp
#include <universal/number/valid/valid.hpp>
using namespace sw::universal;
```

### Verified Computation

```cpp
valid<32, 2> a(1.0);    // [1.0, 1.0] - exact
valid<32, 2> b(3.0);    // [3.0, 3.0] - exact
valid<32, 2> c = a / b; // [0.333..., 0.333...] - interval containing 1/3

std::cout << "1/3 is contained in " << c << std::endl;
std::cout << "Width: " << c.ub() - c.lb() << std::endl;
```

### Uncertainty Propagation

```cpp
// Measurement with known uncertainty
valid<32, 2> measurement(9.81, 9.83);  // gravity: 9.81-9.83 m/s^2
valid<32, 2> mass(1.0, 1.01);          // mass: 1.00-1.01 kg
valid<32, 2> force = mass * measurement;
// force is a guaranteed interval containing the true force
```

## Problems It Solves

| Problem | How valid Solves It |
|---------|-----------------------|
| No way to know accuracy of floating-point result | Interval width directly measures uncertainty |
| Need mathematical proof that result is correct | Guaranteed containment: true value always within bounds |
| Measurement uncertainty propagation | Input intervals propagate through all operations |
| Detecting ill-conditioned computations | Wide output interval = numerically unstable problem |
| Verifying that an iterative solver has converged | Interval width below threshold = convergence proven |
| Sensitivity analysis of model parameters | Input range maps to output range through valid arithmetic |
