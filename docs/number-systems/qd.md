# QD: Quad-Double Ultra-Extended Precision

## Why

Some computations demand precision far beyond what double or even double-double can provide. Evaluating the Riemann zeta function, computing constants like pi to hundreds of digits, verifying numerical algorithms, long-duration N-body simulations, and climate models that must preserve accuracy over millions of time steps -- all require 50+ decimal digits of precision.

Quad-double (QD) extends the double-double technique to four components, achieving ~212 bits of significand (~63.8 decimal digits). This is enough precision for virtually any numerical computation, at a cost of roughly 20-50x that of double -- far cheaper than general arbitrary-precision libraries. QD is the practical ceiling of the multi-component approach: five or more components would provide diminishing returns due to the overhead of maintaining the unevaluated sum invariant.

## What

`qd` is a fixed-format ultra-extended-precision type:

| Property | Value |
|----------|-------|
| Total bits | 256 (4 × 64-bit doubles) |
| Sign | 1 bit |
| Exponent | 11 bits (same as double) |
| Significand | ~212 bits (~63.8 decimal digits) |
| Dynamic range | Same as double (~10^-308 to ~10^308) |
| Epsilon | ~1.22 × 10^-63 |

### Key Properties

- **Not a template**: fixed format using four `double` values
- **~212 bits of precision**: quadruple the precision of `double`
- **Same dynamic range as double**: 11-bit exponent
- **Software-only**: runs on any hardware with IEEE double
- **Component ordering**: `x[0] >= x[1] >= x[2] >= x[3]` in decreasing magnitude

### Internal Representation

```cpp
class qd {
    double x[4];  // Components in decreasing magnitude
    // Value = x[0] + x[1] + x[2] + x[3]
    // Invariant: |x[i+1]| <= 0.5 * ulp(x[i])
};
```

### Precision Hierarchy

| Type | Bits | Decimal Digits | Epsilon |
|------|------|---------------|---------|
| float | 24 | 7.2 | ~6.0 × 10^-8 |
| double | 53 | 15.9 | ~1.1 × 10^-16 |
| dd | 106 | 31.4 | ~4.9 × 10^-32 |
| **qd** | **212** | **63.8** | **~1.2 × 10^-63** |

## How It Works

QD extends the DD technique to four components. Each arithmetic operation uses nested error-free transformations:

1. **Addition**: four-component two-sum cascade, maintaining decreasing magnitude order
2. **Multiplication**: multi-component Dekker products, cross-multiplying all pairs
3. **Division**: cascading Newton-Raphson or digit-by-digit approach
4. **Normalization**: after every operation, re-establish the magnitude ordering invariant

The evaluation strategy uses reverse-order summation (`x[3] + x[2] + x[1] + x[0]`) to preserve the tail components during conversion to a single value.

## How to Use It

### Include

```cpp
#include <universal/number/qd/qd.hpp>
using namespace sw::universal;
```

### Computing Pi to 60+ Digits

```cpp
// Machin's formula: pi/4 = 4*arctan(1/5) - arctan(1/239)
qd one(1), five(5), two39(239);

qd pi4 = qd(4) * atan(one / five) - atan(one / two39);
qd pi = qd(4) * pi4;

std::cout << std::setprecision(64) << pi << std::endl;
// Accurate to ~63 decimal digits
```

### Ultra-Precise Root Finding

```cpp
// Find roots of polynomials with near-multiple roots
// where double and dd lose all precision
qd x(1.0);  // Initial guess for x^5 - 1 = 0 (known root at x=1)

// Perturb slightly and iterate
x = qd("1.00000000000000000000000000000001");
for (int i = 0; i < 100; ++i) {
    qd fx = x * x * x * x * x - qd(1);
    qd fpx = qd(5) * x * x * x * x;
    x = x - fx / fpx;
}
// x converges to 1.0 with ~63 digits of accuracy
```

### Long-Duration Simulation

```cpp
// N-body simulation where energy conservation requires high precision
template<typename Real>
struct Body {
    Real x, y, z;      // Position
    Real vx, vy, vz;   // Velocity
    Real mass;
};

template<typename Real>
Real total_energy(const std::vector<Body<Real>>& bodies) {
    Real ke(0), pe(0);
    for (size_t i = 0; i < bodies.size(); ++i) {
        const auto& b = bodies[i];
        ke += Real(0.5) * b.mass * (b.vx*b.vx + b.vy*b.vy + b.vz*b.vz);
        for (size_t j = i + 1; j < bodies.size(); ++j) {
            Real dx = b.x - bodies[j].x;
            Real dy = b.y - bodies[j].y;
            Real dz = b.z - bodies[j].z;
            Real dist = sqrt(dx*dx + dy*dy + dz*dz);
            pe -= b.mass * bodies[j].mass / dist;
        }
    }
    return ke + pe;
}

// QD preserves energy conservation over millions of time steps
// where double would accumulate unacceptable drift
```

### Verification of Double-Precision Algorithms

```cpp
// Use QD as "ground truth" to measure accuracy of double algorithms
double  d_result = some_algorithm<double>(input);
qd      qd_result = some_algorithm<qd>(input);

qd error = abs(qd(d_result) - qd_result) / abs(qd_result);
std::cout << "Relative error of double: " << error << std::endl;
// Quantifies exactly how many digits the double algorithm preserves
```

## Problems It Solves

| Problem | How QD Solves It |
|---------|-----------------------|
| Need 50+ decimal digits of precision | 212-bit significand = ~64 decimal digits |
| Arbitrary-precision libraries are too slow | QD is 20-50x cost of double, not 1000x |
| Energy drift in long-duration simulations | Extra precision preserves conservation laws |
| Verification of numerical algorithms | QD as "ground truth" reference |
| Computing mathematical constants to high precision | 64 digits sufficient for most constants |
| Near-multiple roots cause catastrophic cancellation | 212 bits survive cancellation events |
| Climate/weather models accumulate error over months | QD maintains accuracy over millions of time steps |
