# Valid: Interval Arithmetic with Poxel Bounds

## Why

Floating-point computation gives you a single number as the answer, but the true mathematical result almost always lies *between* representable values. How far off is the answer? Standard floating-point doesn't tell you. You need to run the computation a second time in higher precision, or perform a separate error analysis, to have any idea of the accuracy.

The `valid` type pairs two poxels as interval bounds. An exact poxel denotes a posit-lattice point; an inexact poxel denotes the open interval above that point. Unlike a single poxel's sticky uncertainty flag, a Valid encloses every poxel between its bounds.

## What

`valid<nbits, es>` stores an interval as a pair of `poxel<nbits, es>` values:

| Parameter | Type | Description |
|-----------|------|-------------|
| `nbits` | `unsigned` | Bits per poxel bound, including its ubit |
| `es` | `unsigned` | Exponent bits in the `posit<nbits - 1, es>` lattice |

### Internal Structure

- **Lower poxel** (`lb`): `poxel<nbits, es>`
- **Upper poxel** (`ub`): `poxel<nbits, es>`
- Each poxel's ubit determines whether its outside endpoint is open.
- Total storage: 2 × nbits bits

### Interval Types

- **`[lb, ub]`**: closed interval, both bounds included
- **`(lb, ub]`**: open lower bound
- **`[lb, ub)`**: open upper bound
- **`(lb, ub)`**: open interval, both bounds excluded
- **Degenerate**: `[v, v]` is a point value (exact)

### Key Properties

- **Guaranteed containment**: true result always lies within the interval
- **Poxel-encoded bounds**: benefits from tapered precision and represents open endpoints
- **Uncertainty quantification**: interval width measures result uncertainty
- **UNUM Type II concept**: implements Gustafson's uncertainty tracking with posit encoding
- **Rigorous arithmetic**: interval operations follow standard inclusion algebra

## How It Works

Interval arithmetic rules ensure that every operation produces bounds that contain the true result:

- **Addition**: `[a,b] + [c,d] = [a+c, b+d]` (round lower bound down, upper bound up)
- **Subtraction**: `[a,b] - [c,d] = [a-d, b-c]`
- **Multiplication**: `[a,b] × [c,d] = [min(ac,ad,bc,bd), max(ac,ad,bc,bd)]`
- **Division**: `[a,b] / [c,d] = [a,b] × [1/d, 1/c]` (when 0 not in [c,d])

Arithmetic evaluates exact poxels at the endpoints and selects enclosing poxels for the result. Exact endpoints remain closed when attainable; inexact endpoints remain open. Division by an interval containing zero widens to the entire representable real line, while division by exact zero produces NaR.

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
valid<32, 2> c = a / b; // an open poxel containing 1/3

std::cout << "1/3 is contained in " << c << std::endl;
std::cout << "Bounds: " << c.lower<double>() << ", " << c.upper<double>() << std::endl;
```

`to_valid(poxel)` explicitly turns one poxel into a Valid. Use `add_valid`, `sub_valid`, `mul_valid`, and `div_valid` to perform enclosing arithmetic on two poxels. Ordinary poxel operators retain their single-poxel sticky-flag behavior.

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
