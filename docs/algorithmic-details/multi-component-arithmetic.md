# Multi-component floating-point arithmetic

Three foundational threads of multi-component arithmetic underpin Universal's
extended-precision types (`dd`, `td`, `qd`, the `*_cascade` family, and the
adaptive `priest` / `ereal` types):

1. **Douglas Priest (1991)** -- theoretical foundations and the error-free
   transformations (EFTs) that make exact floating-point arithmetic possible.
2. **David Bailey / Yozo Hida (2000-2008)** -- production fixed-precision
   implementations: the QD library, double-double and quad-double.
3. **Jonathan Shewchuk (1996-1997)** -- adaptive-precision expansions for
   robust geometric predicates.

This document is the educational companion to the implementation files. It
explains *what each thread contributes*, *what mathematics they share*, and
*how Universal's FloatCascade<N> building block unifies them*. Strict ASCII;
algorithms in standard imperative form.

Related implementation reading:

- `include/sw/universal/internal/floatcascade/` -- the cascade building block
- `include/sw/universal/number/dd/dd_impl.hpp` -- double-double (Bailey/Hida)
- `include/sw/universal/number/qd/qd_impl.hpp` -- quad-double (Hida/Li/Bailey)
- `include/sw/universal/number/ereal/` -- adaptive multi-component real
- `docs/multi-component/comparison-priest-bailey-shewchuk.md` -- earlier
  reference document this one supersedes

## 1. The unifying problem

All three threads attack the same question: how do we represent a real value
to *more* precision than the underlying floating-point type allows, *using
the underlying floating-point type as the only primitive*?

The answer they share: represent the value as an unevaluated sum of
floating-point components.

```text
x = x_0 + x_1 + x_2 + ... + x_{n-1}
```

with two invariants that make the representation well-defined:

- **Non-overlapping**: `|x_{i+1}| <= ulp(x_i) / 2` (a stronger bound:
  `|x_{i+1}| < 2^{-p} * |x_i|` for `p` bits of significand).
- **Ordered by magnitude**: `|x_0| >= |x_1| >= |x_2| >= ...`.

The value `x_0` is called the "leading component" or "estimate"; the trailing
components are corrections that recover the bits the rounded sum threw away.
A 2-component expansion (`dd`) gives roughly 106 bits of significand
(IEEE-754 double has 53). A 4-component expansion (`qd`) gives ~212 bits.

The three threads differ in three orthogonal choices:

| Choice | Priest | Bailey/Hida | Shewchuk |
|---|---|---|---|
| Component count | Variable (conceptual) | Fixed (N = 2, 4) | Dynamic (grows as needed) |
| Storage | Theoretical | `std::array<double, N>` | `std::vector<double>` |
| Optimization | Proofs | Hand-tuned for N=2, 4 | Optimized common case |
| Use case | Foundation | HPC scientific computing | Computational geometry |

Universal aims to provide *all three* paradigms behind a single building
block (`FloatCascade<N>` for fixed, `VariableCascade` / `ereal` for
adaptive) so a user can pick the trade-off without rewriting their kernel.

## 2. Priest's error-free transformations (EFTs)

The primitives every multi-component algorithm builds on. Each EFT takes two
floating-point inputs and produces a pair `(s, e)` such that `op(a, b) = s + e`
*exactly* in real arithmetic, with `s` the rounded result and `e` the
rounding error.

### 2.1 two_sum

The standard Knuth/Moller algorithm for exact addition. Works for *any* IEEE
inputs (no magnitude precondition):

```text
two_sum(a, b) -> (s, e):
    s  = a + b
    bb = s - a
    e  = (a - (s - bb)) + (b - bb)
    invariant: a + b = s + e exactly,  |e| <= ulp(s) / 2
```

Six floating-point operations, no branches. The bedrock of multi-component
addition.

### 2.2 quick_two_sum (Dekker's "fast two-sum")

Cheaper variant when `|a| >= |b|` is known in advance (typical after a
renormalization step):

```text
quick_two_sum(a, b) -> (s, e):           // assumes |a| >= |b|
    s = a + b
    e = b - (s - a)
    invariant: a + b = s + e exactly
```

Three floating-point operations. The savings matter inside renormalization
loops where the magnitude ordering is maintained as a loop invariant.

### 2.3 two_prod

Exact multiplication. With a fused-multiply-add (FMA) primitive it is two
operations:

```text
two_prod(a, b) -> (p, e):
    p = a * b
    e = fma(a, b, -p)
    invariant: a * b = p + e exactly
```

Without FMA, the Veltkamp/Dekker splitting is required (~17 ops). Universal's
`internal/floatcascade/` implementation auto-selects FMA when the platform
flag `__FMA__` is defined and falls back to the split form otherwise. This
choice is invisible to the consumer but visible in the throughput numbers.

### 2.4 three_sum, renormalization

Higher-level helpers built on the EFTs above:

```text
three_sum(a, b, c) -> (x, y, z):
    (t, e1) = two_sum(b, c)
    (x, e2) = two_sum(a, t)
    (y, z)  = two_sum(e2, e1)
    invariant: a + b + c = x + y + z exactly, with |x| >= |y| >= |z|

renorm(x_0, ..., x_{N-1}) -> (y_0, ..., y_{N-1}):
    sweep: bubble large/small components, two_sum at each step, until
    non-overlapping invariant holds for the full N-tuple.
```

Renormalization is the operation that restores the multi-component
representation's invariants after an arithmetic step that may have left
overlapping components.

These primitives are entirely Priest's contribution: he established that
they exist, proved their error bounds, and gave them theoretical legs.
Everything multi-component above this layer is application of EFTs in some
pattern.

## 3. Bailey / Hida: fixed-precision production library

Bailey and Hida's QD library (Lawrence Berkeley National Lab, 2000-2008)
made multi-component arithmetic practical for scientific computing. The
core design choice is to fix the component count at a small constant
(`N = 2` for `dd`, `N = 4` for `qd`) and *hand-craft* the operation
sequence for each (operation, N) pair.

### 3.1 Double-double (dd)

A pair `(hi, lo)` with `|lo| <= ulp(hi) / 2`. Approximately:

```text
1 sign + 11 exponent + 106 fraction bits   (vs. double's 53)
~31 decimal digits
2x the storage, ~10x the cost per op
```

`dd` addition (from `include/sw/universal/number/dd/dd_impl.hpp`):

```cpp
dd& operator+=(const dd& rhs) {
    double s2;
    hi = two_sum(hi, rhs.hi, s2);
    if (std::isfinite(hi)) {
        double t2, t1 = two_sum(lo, rhs.lo, t2);
        lo = two_sum(s2, t1, t1);
        t1 += t2;
        three_sum(hi, lo, t1);
    }
    return *this;
}
```

`dd` multiplication uses six `two_prod` invocations to capture all partial
products at full precision, then `three_sum` and renormalization to settle
the result. The hand-optimization shows up here: a generic
`expansion_multiply(N)` routine would compute many partial products whose
contributions vanish at the precision of the result. Hand-crafted `dd_mul`
elides those partial products at the source level.

### 3.2 Quad-double (qd)

A 4-tuple. ~212 bits of significand, ~62 decimal digits. The hand-crafted
multiplication kernel (`qd_mul` in the same file) tracks the leading-order
seven partial products, applies `three_sum` to cluster magnitudes, and
renormalizes -- about 25 EFTs total. Generic adaptive code would compute
about 10 partial products of the same magnitude and discard most of them
during renormalization; the hand-crafted version skips that work.

### 3.3 Strengths and trade-offs

- **Fastest** for known precision requirements -- constant-time, constant
  storage, branch-predictable, vectorizes well.
- **Cache-friendly** -- the entire value fits in 16 bytes (`dd`) or 32
  (`qd`).
- **Easy to vectorize / parallelize** -- the SIMD-friendly fixed layout is
  what makes Bailey/Hida the dominant choice in HPC.
- **Wastes work** when intermediate computations need less precision than
  N components give.
- **Insufficient** when *more* precision is needed -- you have to switch
  types (or fall back to `priest` / `ereal`).

References:

- Hida, Li, Bailey 2000. "Algorithms for Quad-Double Precision Floating
  Point Arithmetic." *Proc. 15th IEEE Symposium on Computer Arithmetic*.
- Bailey, Hida 2008. "Library for Double-Double and Quad-Double Arithmetic."
  LBNL Technical Report.

## 4. Shewchuk: adaptive expansions for geometric predicates

Shewchuk's contribution (1996/1997) was the realization that for many
applications -- specifically computational geometry -- *the precision
needed varies wildly with the inputs*. A general-position orientation test
needs only a few bits of correction; a near-degenerate one (three points
nearly collinear) needs many. Pre-allocating the worst-case precision (as
Bailey/Hida do) burns cycles on the common case to handle the rare case.

### 4.1 Variable-length expansions

Shewchuk replaces the fixed `std::array<double, N>` storage with
`std::vector<double>` (or pre-allocated workspace), and provides expansion
operations that *grow* the result length only as needed:

```text
grow_expansion(e[0..elen-1], b)   ->  h[0..elen]
    invariant: sum(e) + b = sum(h),  h non-overlapping.

scale_expansion(e[0..elen-1], b)   ->  h[0..2*elen]
    invariant: sum(e) * b = sum(h),  h non-overlapping.

fast_expansion_sum(e[0..elen-1], f[0..flen-1])  ->  h[0..elen+flen]
    invariant: sum(e) + sum(f) = sum(h),  h non-overlapping.

compress(e[0..elen-1])  ->  h[0..k]    (k <= elen)
    discard components below precision target;  sum(e) ~= sum(h) within tol.
```

### 4.2 The adaptive predicate pattern

The Shewchuk geometric predicate idiom:

```text
result = cheap_approximation(inputs)            // hardware-precision estimate
err    = error_bound(inputs)                    // worst-case error of that estimate
if |result| > err:
    return sign(result)                         // estimate is conclusive

// Estimate is inconclusive; compute exactly via expansions.
exact  = expansion_sum_of_signed_products(inputs)
return sign(estimate(exact))
```

The cheap approximation handles the vast majority of inputs (general
position); the expensive expansion-based exact path only fires when the
estimate cannot resolve sign. This *amortizes* the worst-case cost over
the common case.

### 4.3 Strengths and trade-offs

- **Optimal asymptotically** for varying-precision workloads.
- **Fast common case**: a 2D orientation predicate is one estimate plus an
  error-bound comparison in the easy case.
- **Robust pathological case**: precision grows automatically until the
  predicate resolves.
- **No upfront precision commitment**: the algorithm decides how much work
  to do based on the inputs.
- **Slower worst case** (2-3.5x Bailey/Hida when fully extended).
- **Dynamic allocation**: less cache-friendly, less vectorizable.
- **Variable timing**: harder to predict, harder to bound for real-time
  use cases.

References:

- Shewchuk 1997. "Adaptive Precision Floating-Point Arithmetic and Fast
  Robust Geometric Predicates." *Discrete & Computational Geometry*,
  18(3), 305-363.
- Public-domain C implementation at https://www.cs.cmu.edu/~quake/robust.html

## 5. Universal's unification: FloatCascade<N>

Universal's design choice is to express the fixed-size case (Bailey/Hida)
and the adaptive case (Shewchuk/Priest) on the *same* building block
abstraction, so the user can switch tiers without rewriting the kernel and
so the library can share the EFT machinery across types.

### 5.1 The building block

```cpp
namespace sw::universal::internal {

    template<typename Limb, std::size_t N>
    class FloatCascade {
    public:
        using value_type = Limb;
        static constexpr std::size_t size = N;

        // The Priest EFTs and their compositions, defined once on the cascade
        // template and shared across all (Limb, N) instantiations:
        static constexpr void two_sum(Limb a, Limb b, Limb& s, Limb& e);
        static constexpr void quick_two_sum(Limb a, Limb b, Limb& s, Limb& e);
        static constexpr void two_prod(Limb a, Limb b, Limb& p, Limb& e);
        static constexpr void three_sum(Limb& a, Limb& b, Limb& c);
        static constexpr void renormalize(std::array<Limb, N>& comps);

        // Cascade arithmetic (template specialized per N for the hand-crafted
        // patterns Bailey/Hida established):
        FloatCascade operator+(const FloatCascade& rhs) const;
        FloatCascade operator*(const FloatCascade& rhs) const;
        // ...

    private:
        std::array<Limb, N> components_;
    };

}
```

`Limb` is typically `double` but can be any IEEE-754 limb the platform
supports (e.g. `float` for `float_cascade<float, N>` configurations used in
mixed-precision algorithm exploration; `long double` on platforms where it
adds precision).

### 5.2 Fixed-N types built on the cascade

```cpp
namespace sw::universal {
    using dd       = number::dd::dd<>;            // FloatCascade<double, 2> under the hood
    using qd       = number::qd::qd<>;            // FloatCascade<double, 4>

    using dd_cascade = ...;     // explicit FloatCascade<double, 2> exposure
    using td_cascade = ...;     // FloatCascade<double, 3>
    using qd_cascade = ...;     // FloatCascade<double, 4>
}
```

The legacy `dd` and `qd` types keep their Bailey/Hida-faithful operator
implementations; the `*_cascade` family exposes the same precision via the
unified cascade machinery. Performance is comparable; cascade types are
the educational on-ramp for adding new fixed-N expansions (triple-double
was added this way) and for algorithm exploration in the cascade math
library (`include/sw/universal/internal/floatcascade/math/`).

### 5.3 Adaptive (Shewchuk-style) types

The adaptive side of the architecture replaces `std::array<Limb, N>` with
a growable container:

```cpp
template<typename Limb>
class VariableCascade {
    std::vector<Limb> components_;
    // ... same EFT machinery, but with grow_expansion / compress
};

class ereal {            // elastic real
    VariableCascade<double> cascade_;
    AdaptivePrecision     config_;     // tolerance, max components, max iterations
};
```

`ereal` is Universal's main user-facing adaptive multi-component type.
`AdaptivePrecision` carries the termination policy: absolute or relative
tolerance, hard cap on component count (to bound the worst case), iteration
limits for division / square root / transcendental refinement loops.

### 5.4 Cross-tier conversion

The architectural payoff is that the fixed and adaptive tiers
interconvert losslessly:

```cpp
dd     d  = dd(1.0) / dd(3.0);              // fast, ~31 digits
ereal  e  = ereal(d);                       // promote: still ~31 digits, now adaptive
e        /= ereal(7.0);                     // adaptive division refines until tolerance met
dd     d2 = e.to_dd();                      // extract back to fixed for the hot loop
```

The user chooses the precision tier appropriate to *each phase* of their
algorithm, rather than committing to one tier at type-declaration time.

## 6. Comparison summary

| Aspect                    | Priest (theory)       | Bailey/Hida (`dd`, `qd`)            | Shewchuk (adaptive `ereal`)   |
|---------------------------|------------------------|-------------------------------------|-------------------------------|
| Precision                 | Variable (conceptual) | Fixed (2 or 4 components)            | Dynamic (grows as needed)     |
| Primitives                | Defined the EFTs       | Uses EFTs in fixed hand-crafted patterns | Uses EFTs with expansion growth |
| Storage                   | Theoretical            | `std::array<double, N>`              | `std::vector<double>`         |
| Per-op cost               | N/A                    | Constant time, small constant         | Variable, common case fast    |
| Per-op storage            | N/A                    | Constant                              | Grows with input separation   |
| Determinism               | N/A                    | Constant time and space               | Variable time and space       |
| Vectorization             | N/A                    | SIMD-friendly                         | Hard to vectorize            |
| Cache locality            | N/A                    | 16-32 bytes per value                 | Variable, may exceed L1       |
| Use case                  | Academic foundation    | HPC, physics, ML mixed-precision      | Computational geometry, CAD/CAM |
| Worst-case timing         | N/A                    | Same as common case                   | 2-3.5x common case            |
| Error control             | Proven bounds          | Pre-set precision                     | Adaptive until bound met      |
| Universal type            | -                      | `dd`, `qd`, `td_cascade`              | `ereal`, future `priest`       |

The three approaches are complementary, not competing. Priest provided the
theoretical foundation. Bailey/Hida productionized it for the
known-precision case. Shewchuk extended it to the variable-precision case.
Universal ships all three, with the cascade building block sitting under
them as the shared substrate.

## 7. Picking a type

Use the following decision tree:

| If your workload ...                                                | use                  |
|---------------------------------------------------------------------|----------------------|
| needs ~106 bits of significand, knows it upfront, in a hot loop      | `dd` (Bailey/Hida)   |
| needs ~212 bits of significand, knows it upfront                     | `qd` (Hida/Li/Bailey)|
| needs ~159 bits (e.g. for a tighter solver iteration count)          | `td_cascade`         |
| explores the cascade math primitives directly (research / education) | `dd_cascade`, `qd_cascade` |
| has computational-geometry predicates where input separation varies  | `ereal`              |
| needs an *oracle* type to validate other types' precision behavior   | `ereal` or `elrealo` |
| writes mixed-precision code that needs to switch tiers per phase     | start adaptive, drop down to `dd` / `qd` for hot loops |

For the typical numerical-analysis workload (HPC, ML training, physics
simulation) the answer is `dd` or `qd`. For computational-geometry and
formal-verification work the answer is `ereal`. The cascade building block
is what makes the choice non-binding at type-declaration time.

## 8. Validation strategy

Multi-component types' correctness is established by two complementary
methods:

1. **EFT property tests.** For each `two_sum`, `quick_two_sum`, `two_prod`,
   over a sampled grid of input magnitudes, assert
   `a op b = s + e` exactly (no rounding) in MPFR-validated reference
   arithmetic.
2. **End-to-end accuracy tests.** For each composite operation (`+`, `*`,
   `/`, `sqrt`, transcendentals), compare against MPFR at higher precision
   than the type advertises, and assert the error stays within the
   advertised bound (typically `<= 2^-p` where `p` is the type's advertised
   precision).

Universal's regression suite for multi-component types lives under
`static/cascade/` and `elastic/ereal/`. The cascade math library has its
own tests under `static/cascade_math/`. The combined suite is the
operational answer to "does this type behave as the multi-component
literature says it should?"

## References

### Foundational papers

1. **Douglas M. Priest** (1991). "On Properties of Floating Point Arithmetics:
   Numerical Stability and the Cost of Accurate Computations." Ph.D.
   dissertation, University of California, Berkeley.
2. **Dekker, T. J.** (1971). "A Floating-Point Technique for Extending the
   Available Precision." *Numerische Mathematik*, 18(3), 224-242.
3. **Knuth, D. E.** (1969). *The Art of Computer Programming, Volume 2:
   Seminumerical Algorithms*. Section 4.2.2 (the original two_sum).

### Bailey / Hida

4. **Hida, Y., Li, X. S., Bailey, D. H.** (2000). "Algorithms for Quad-Double
   Precision Floating Point Arithmetic." *Proc. 15th IEEE Symposium on
   Computer Arithmetic (ARITH-15)*, 155-162.
   https://www.davidhbailey.com/dhbpapers/quad-double.pdf
5. **Bailey, D. H.** (2008). "Library for Double-Double and Quad-Double
   Arithmetic." LBNL Technical Report.
   https://www.davidhbailey.com/dhbpapers/qd.pdf

### Shewchuk

6. **Shewchuk, J. R.** (1996). "Robust Adaptive Floating-Point Geometric
   Predicates." *Proc. 12th Annual Symposium on Computational Geometry*,
   141-150.
7. **Shewchuk, J. R.** (1997). "Adaptive Precision Floating-Point Arithmetic
   and Fast Robust Geometric Predicates." *Discrete & Computational
   Geometry*, 18(3), 305-363.
   https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf

### Software

8. **QD Library** (Bailey/Hida).
   https://www.davidhbailey.com/dhbsoftware/
9. **Shewchuk's Geometric Predicates** (public domain).
   https://www.cs.cmu.edu/~quake/robust.html
10. **Universal Numbers Library**.
    https://github.com/stillwater-sc/universal
