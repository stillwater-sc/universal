# Quire: Exact Accumulation for Reproducible Linear Algebra

## Why

The dot product is the most fundamental operation in numerical computing -- it underlies matrix multiplication, linear solvers, neural network inference, and signal processing. In IEEE-754 floating-point, every addition in a dot product introduces rounding error, and the result depends on the order of operations. Sum `a*b + c*d + e*f` in a different order and you get a different answer. This makes floating-point linear algebra *non-reproducible*: the same code on different hardware (or with different compiler flags) produces different results.

The quire is a super-accumulator that is wide enough to hold the exact sum of any number of posit products without *any* intermediate rounding. You multiply-and-accumulate as many times as you need, and only round once at the very end when converting back to a posit. The result is bit-exact, order-independent, and reproducible on every platform.

## What

`quire<nbits, es, capacity>` is a fixed-point super-accumulator:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Posit configuration width |
| `es` | `unsigned` | -- | Posit exponent bits |
| `capacity` | `unsigned` | 2 | Extra guard bits for accumulation headroom |

### Size

The quire is sized to hold the exact sum of products:
- Dynamic range: `2^es × (4×nbits - 8)` bits
- Total width: dynamic_range + capacity bits
- For `posit<32, 2>`: quire is ~512 bits wide
- For `posit<16, 1>`: quire is ~128 bits wide

### Key Properties

- **Exact accumulation**: no rounding between multiply-accumulate operations
- **Order-independent**: `a*b + c*d = c*d + a*b` always (unlike IEEE-754)
- **Single final rounding**: only rounds when converting quire → posit
- **Reproducible**: same result on every platform, every compiler, every time
- **Fixed-point internal**: radix point at midpoint of dynamic range

### Operations

- `quire += posit × posit` (fused multiply-accumulate)
- `quire -= posit × posit` (fused multiply-subtract)
- `posit = convert(quire)` (final rounding to posit)
- `quire.clear()` (reset to zero)

## How It Works

The quire is a fixed-point register wide enough to represent the exact sum of any number of products of posit values. The key insight is that the product of two `posit<nbits, es>` values has a bounded range and bounded precision, so a fixed-point accumulator of known width can hold the exact result.

The accumulation pipeline:
1. **Multiply** two posits to get an exact product (no rounding)
2. **Align** the product to the quire's fixed-point format
3. **Add** to the quire (exact integer addition, no rounding)
4. Repeat steps 1-3 for all terms
5. **Convert** the quire to a posit (single rounding at the end)

Because there is only one rounding step (at the very end), the result is:
- **Exact** for sums of products that fit in the posit range
- **Faithfully rounded** otherwise (within 1 ULP of the true result)
- **Order-independent** because addition in the fixed-point quire is associative

## How to Use It

### Include

```cpp
#include <universal/number/posit1/posit1.hpp>
using namespace sw::universal;
```

### Exact Dot Product

```cpp
using Posit = posit<32, 2>;
using Quire = quire<32, 2>;

Posit a[] = { Posit(1.0), Posit(1e-10), Posit(1e10), Posit(-1e10) };
Posit b[] = { Posit(1.0), Posit(1e10),  Posit(1.0),  Posit(1.0)  };

Quire q;
q.clear();
for (int i = 0; i < 4; ++i) {
    q += quire_mul(a[i], b[i]);
}

Posit result;
convert(q.to_value(), result);
// result = 2.0 exactly
// IEEE-754 would lose the 1.0 term due to catastrophic cancellation
```

### Reproducible Matrix Multiplication

```cpp
template<typename Posit, unsigned N>
void matmul_exact(const Posit A[N][N], const Posit B[N][N], Posit C[N][N]) {
    constexpr unsigned nbits = Posit::nbits;
    constexpr unsigned es = Posit::es;
    using Quire = quire<nbits, es>;

    for (unsigned i = 0; i < N; ++i) {
        for (unsigned j = 0; j < N; ++j) {
            Quire q;
            q.clear();
            for (unsigned k = 0; k < N; ++k) {
                q += quire_mul(A[i][k], B[k][j]);
            }
            convert(q.to_value(), C[i][j]);
        }
    }
    // C is bit-exact reproducible regardless of hardware or compiler
}
```

### Kahan-Like Summation (But Exact)

```cpp
// No need for Kahan compensated summation -- the quire is exact
using Posit = posit<32, 2>;
using Quire = quire<32, 2>;

Quire q;
q.clear();
Posit one(1.0);

// Sum 1.0 a million times: result is exactly 1,000,000
for (int i = 0; i < 1000000; ++i) {
    q += quire_mul(one, one);  // 1.0 * 1.0 = 1.0, accumulated exactly
}

Posit result;
convert(q.to_value(), result);
// result = 1000000.0 exactly
```

## Problems It Solves

| Problem | How quire Solves It |
|---------|-----------------------|
| Dot product results depend on summation order | Quire accumulation is order-independent |
| Different hardware/compilers give different results | Single final rounding = bit-exact on all platforms |
| Catastrophic cancellation in sums of large and small products | Quire is wide enough to hold all terms exactly |
| Kahan summation is complex and still not exact | Quire is truly exact -- no compensated summation needed |
| BLAS libraries don't guarantee reproducibility | Quire-based BLAS is reproducible by construction |
| Iterative refinement needs accurate residual computation | Quire computes exact residuals |
