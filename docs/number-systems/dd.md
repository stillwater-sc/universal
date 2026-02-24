# DD: Double-Double Extended Precision

## Why

IEEE double provides 53 bits of significand (~15.9 decimal digits). For many scientific computations -- ill-conditioned linear systems, long-running simulations, Newton's method for high-degree polynomials -- this is not enough. But going to quad precision (128-bit) requires either rare hardware support or slow arbitrary-precision libraries.

Double-double (DD) achieves ~106 bits of significand (~31.4 decimal digits) using *two ordinary doubles*. The trick: the value is stored as an unevaluated sum `hi + lo`, where `hi` carries the high-order bits and `lo` carries the residual. Arithmetic uses error-free transformations (Dekker's algorithm, Knuth's two-sum) to maintain the invariant that `lo` captures exactly the rounding error of `hi`. The result is double the precision of double, using only standard double-precision hardware -- no special instructions, no library dependencies.

## What

`dd` is a fixed-format extended-precision type:

| Property | Value |
|----------|-------|
| Total bits | 128 (2 × 64-bit doubles) |
| Sign | 1 bit |
| Exponent | 11 bits (same as double) |
| Significand | ~106 bits (~31.4 decimal digits) |
| Dynamic range | Same as double (~10^-308 to ~10^308) |
| Epsilon | ~4.93 × 10^-32 |

### Key Properties

- **Not a template**: fixed format using two `double` values
- **~106 bits of precision**: double the precision of `double`
- **Same dynamic range as double**: 11-bit exponent, ~10^308
- **Software-only**: runs on any hardware with IEEE double
- **Error-free transformations**: no precision loss in intermediate computations
- **Supports infinity, NaN, subnormals**: full IEEE-754 special value semantics

### Internal Representation

```cpp
class dd {
    double hi;  // High-order component (carries most significant bits)
    double lo;  // Low-order component (captures rounding error of hi)
    // Invariant: |lo| <= 0.5 * ulp(hi)
};
```

The mathematical value is `hi + lo`, but the two components are never actually added together (that would lose precision). Instead, they are maintained as an unevaluated sum.

## How It Works

The core algorithms are error-free transformations:

**Two-Sum** (Knuth): computes `a + b = s + e` where `s` is the floating-point sum and `e` is the exact rounding error:
```
s = a + b
v = s - a
e = (a - (s - v)) + (b - v)
```

**Two-Product** (Dekker): computes `a * b = p + e` where `p` is the floating-point product and `e` is the exact rounding error (requires FMA or Dekker splitting).

DD arithmetic chains these transformations:
1. Compute `hi` of the result using standard double arithmetic
2. Compute the exact rounding error of that operation
3. Add the rounding error to `lo` (along with the `lo` components of the operands)
4. Renormalize the `(hi, lo)` pair

## How to Use It

### Include

```cpp
#include <universal/number/dd/dd.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
dd a("3.14159265358979323846264338327950");  // 32 digits of pi
dd b("2.71828182845904523536028747135266");  // 32 digits of e

dd product = a * b;
std::cout << std::setprecision(32) << product << std::endl;
// Accurate to ~31 decimal digits
```

### Solving Ill-Conditioned Systems

```cpp
template<typename Real>
Real hilbert_condition(int n) {
    // Hilbert matrix has condition number ~10^(3.5n)
    // For n=5, condition number ~10^17, exceeding double precision
    Real sum(0);
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= n; ++j) {
            sum += Real(1) / Real(i + j - 1);
        }
    }
    return sum;
}

// double loses all precision for large Hilbert matrices
double d_result = hilbert_condition<double>(10);

// dd maintains ~31 digits, enough for moderately ill-conditioned problems
dd dd_result = hilbert_condition<dd>(10);
```

### Newton's Method with Extended Precision

```cpp
// Finding roots of high-degree polynomials
dd x(1.5);  // Initial guess
for (int i = 0; i < 50; ++i) {
    dd fx = x * x * x - dd(2);          // f(x) = x^3 - 2
    dd fpx = dd(3) * x * x;             // f'(x) = 3x^2
    x = x - fx / fpx;
}
// x converges to cube_root(2) with ~31 digits of accuracy
```

### Precision Progression

```cpp
// Same computation at three precision levels
float  f_sum = 0.0f;
double d_sum = 0.0;
dd     dd_sum(0.0);

for (int i = 1; i <= 1000000; ++i) {
    float  term_f = 1.0f / (float)i;
    double term_d = 1.0 / (double)i;
    dd     term_dd = dd(1) / dd(i);

    f_sum += term_f;    // ~7 digits
    d_sum += term_d;    // ~15 digits
    dd_sum += term_dd;  // ~31 digits
}
```

## Problems It Solves

| Problem | How DD Solves It |
|---------|-----------------------|
| Double precision (15 digits) is insufficient | DD provides ~31 decimal digits |
| Quad precision requires special hardware or slow libraries | DD uses standard double hardware, no dependencies |
| Ill-conditioned systems lose all significant digits | Extra precision preserves meaningful digits |
| Newton's method stalls at double-precision accuracy | DD allows convergence to 31-digit accuracy |
| Error-free residual computation for iterative refinement | Error-free transformations capture exact rounding errors |
| Need extended precision but can't afford arbitrary-precision | DD is ~10-20x cost of double, not 100-1000x like MPFR |
| Cross-platform reproducibility at extended precision | Pure software, same result on every IEEE-754 platform |
