# SORN: Set of Operand Range Numbers

## Why

Traditional number systems associate each bit pattern with a single point value. When you store 3.14 in a float, the type claims to represent exactly 3.14 -- even though the true stored value is 3.14000010490417... and the true mathematical value might be something else entirely. This point-value illusion hides the inherent uncertainty of numerical computation.

SORN (Set of Operand Range Numbers) takes a fundamentally different approach: each bit pattern represents a *range* of real numbers. There is no pretense of point precision. When you operate on SORNs, you get back the set of all possible results, providing rigorous bounds on computational uncertainty without any additional overhead.

## What

`sorn<start, stop, steps, flagLin, flagHalfopen, flagNeg, flagInf, flagZero>` is an interval-based number system:

| Parameter | Type | Description |
|-----------|------|-------------|
| `start` | `signed int` | Start of interval range |
| `stop` | `signed int` | End of interval range |
| `steps` | `unsigned int` | Number of subdivision steps |
| `flagLin` | `bool` | Linear (true) or logarithmic (false) spacing |
| `flagHalfopen` | `bool` | Half-open intervals? |
| `flagNeg` | `bool` | Include negative number range? |
| `flagInf` | `bool` | Include infinity? |
| `flagZero` | `bool` | Include zero? |

### Key Properties

- **Interval semantics**: each bit pattern represents a range, not a point
- **Set-valued arithmetic**: operations produce unions of output ranges
- **Conservative bounds**: result set always contains the true answer
- **Configurable distribution**: linear or logarithmic interval spacing
- **Configurable topology**: open, closed, or half-open intervals
- **No precision illusion**: inherent uncertainty is explicit in the representation

### Interval Arithmetic Rules

Operations on SORNs follow interval algebra:
- `A + B` = set of all `a + b` where `a ∈ A` and `b ∈ B`
- `A × B` = set of all `a × b` where `a ∈ A` and `b ∈ B`
- Results may span multiple SORN intervals (union of ranges)

## How It Works

The SORN encoding divides the real number line into intervals. Each bit pattern maps to one interval. With `steps` subdivisions between `start` and `stop`:

- **Linear spacing**: intervals have equal width
- **Logarithmic spacing**: intervals have equal ratio (useful for wide dynamic range)

Operations produce a *set* of intervals (a "SORN set") that covers all possible results. If two input SORNs overlap, the output SORN set is the union of all possible outcomes. This is inherently conservative -- the output always contains the true answer, but may be wider than necessary (the "wrapping effect").

## How to Use It

### Include

```cpp
#include <universal/number/sorn/sorn.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
// 20-bit SORN with 8 linear steps, half-open, negative, with zero and infinity
sorn<-10, 10, 8, true, true, true, true, true> a, b, c;

a = 3.5;  // Assigned to the interval containing 3.5
b = 2.1;  // Assigned to the interval containing 2.1
c = a + b;
// c is the set of intervals covering all possible sums
std::cout << "Result range: " << c << std::endl;
```

### Rigorous Uncertainty Quantification

```cpp
// Sensor reading: value is somewhere in [4.9, 5.1]
// SORN naturally represents this as the covering set of intervals

sorn<0, 100, 16, true, true, false, false, true> sensor_a(4.95);
sorn<0, 100, 16, true, true, false, false, true> sensor_b(5.05);

// Combining uncertain measurements
auto combined = sensor_a + sensor_b;
// Result is the rigorous union of all possible sums
```

## Problems It Solves

| Problem | How SORN Solves It |
|---------|-----------------------|
| Floating-point pretends every result is exact | SORN explicitly represents value as a range |
| No way to track computational uncertainty cheaply | Interval semantics are built into the type, no extra cost |
| Interval arithmetic libraries are complex bolt-ons | SORN is interval arithmetic from the ground up |
| Need conservative bounds for safety-critical systems | Result set always contains true answer by construction |
| Linear vs logarithmic spacing for different applications | Configurable at compile time |
| Uncertainty quantification requires separate Monte Carlo runs | SORN propagates uncertainty through every operation |
