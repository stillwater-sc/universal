# Multi-component floating-point arithmetic

Four foundational threads of multi-component arithmetic underpin Universal's
extended-precision types (`dd`, `td`, `qd`, the `*_cascade` family, the
adaptive `ereal`, and the lazy `elreal`):

1. **Douglas Priest (1991)** -- theoretical foundations and the error-free
   transformations (EFTs) that make exact floating-point arithmetic possible.
2. **David Bailey / Yozo Hida (2000-2008)** -- production fixed-precision
   implementations: the QD library, double-double and quad-double.
3. **Jonathan Shewchuk (1996-1997)** -- adaptive-precision expansions for
   robust geometric predicates.
4. **Ryan McCleeary (2019)** -- lazy exact real arithmetic with
   precision-on-demand refinement.

This document is the educational companion to the implementation files. It
explains *what each thread contributes*, *what mathematics they share*, and
*how Universal's floatcascade<N> building block + lazy-stream elreal cover
the precision-engineering space*.

Related implementation reading:

- `include/sw/universal/internal/floatcascade/` -- the cascade building block
- `include/sw/universal/number/dd/dd_impl.hpp` -- double-double (Bailey/Hida)
- `include/sw/universal/number/qd/qd_impl.hpp` -- quad-double (Hida/Li/Bailey)
- `include/sw/universal/number/ereal/` -- adaptive multi-component real
- `include/sw/universal/number/elreal/` -- lazy exact real (McCleeary)
- `docs/algorithmic-details/lazy-real-arithmetic.md` -- elreal algorithm
  deep-dive
- `docs/multi-component/comparison-priest-bailey-shewchuk.md` -- earlier
  reference document this one supersedes

## 1. The unifying problem

All three threads attack the same question: how do we represent a real value
to *more* precision than `double` allows, *using `double` as the only
primitive*? Universal's implementation is `double`-only -- there is no
`float`-cascade or `long double`-cascade variant. The components are always
IEEE-754 binary64.

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
`double` inputs and produces a pair `(s, r)` such that `op(a, b) = s + r`
*exactly* in real arithmetic, with `s` the rounded result and `r` the
rounding residual.

Universal's EFTs live in
`include/sw/universal/numerics/error_free_ops.hpp` as free functions in
namespace `sw::universal`. They are *not* members of `floatcascade<N>` --
the cascade and its math operations *call* the EFTs, but the EFTs are
standalone primitives that any consumer (cascade, ereal, blas kernel, etc.)
can use. Each section below shows the pseudo-algorithm first (in the
mathematically clearer "returns a pair" form), followed by the actual
Universal implementation.

The runtime path uses `volatile` to defeat the optimizer's tendency to
reassociate the EFT expressions -- without that, an optimizer can prove
`(s - a) - b == 0` algebraically and erase the residual computation, which
is correct in real arithmetic but wrong in floating-point. The constexpr
path dispatches on `std::is_constant_evaluated()` and uses
`is_finite_cx` (a constexpr-safe isfinite replacement) in place of
`std::isfinite`. The two paths compute the same thing; only the runtime
path is shown below.

### 2.1 two_sum

The standard Knuth/Moller algorithm for exact addition. Works for *any* IEEE
inputs (no magnitude precondition).

```text
two_sum(a, b) -> (s, r):
    s  = a + b
    bb = s - a
    r  = (a - (s - bb)) + (b - bb)
    invariant: a + b = s + r exactly,  |r| <= ulp(s) / 2
```

Six floating-point operations, no branches. The bedrock of multi-component
addition.

Universal implementation (runtime branch):

```cpp
constexpr inline double two_sum(double a, double b, double& r) {
    volatile double s = a + b;
    if (std::isfinite(s)) {
        volatile double bb = s - a;
        r = (a - (s - bb)) + (b - bb);
    }
    else {
        r = 0.0;
    }
    return s;
}
```

The `if (isfinite(s))` guard handles overflow: when `a + b` overflows to
infinity, the residual algebra produces NaN, so the code returns
`(inf, 0)` instead. The lost-precision claim of the EFT is voided in that
case anyway -- if `s` overflowed, the residual is mathematically
meaningless.

### 2.2 quick_two_sum (Dekker's "fast two-sum")

Cheaper variant when `|a| >= |b|` is known in advance (typical after a
renormalization step).

```text
quick_two_sum(a, b) -> (s, r):           // assumes |a| >= |b|
    s = a + b
    r = b - (s - a)
    invariant: a + b = s + r exactly
```

Three floating-point operations. The savings matter inside renormalization
loops where the magnitude ordering is maintained as a loop invariant.

Universal implementation (runtime branch):

```cpp
constexpr inline double quick_two_sum(double a, double b, double& r) {
    volatile double s = a + b;
    r = (std::isfinite(s) ? b - (s - a) : 0.0);
    return s;
}
```

### 2.3 two_prod

Exact multiplication. The Veltkamp/Dekker splitting form (always available,
~17 ops) or a single fused-multiply-subtract (FMS) when the platform
exposes one.

```text
two_prod(a, b) -> (p, r):
    p = a * b
    if FMS is available:
        r = fms(a, b, p)              // r = a*b - p, computed in one rounded op
    else:
        (a_hi, a_lo) = split(a)
        (b_hi, b_lo) = split(b)
        r = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo
    invariant: a * b = p + r exactly
```

Universal implementation (runtime branch):

```cpp
constexpr inline double two_prod(double a, double b, double& r) {
    volatile double p = a * b;
    if (std::isfinite(p)) {
#if defined( RELIABLE_FUSED_MULTIPLY_SUBTRACT_OPERATOR )
        r = UNIVERSAL_FMS(a, b, p);
#else
        double a_hi, a_lo, b_hi, b_lo;
        split(a, a_hi, a_lo);
        split(b, b_hi, b_lo);
        r = ((a_hi * b_hi - p) + a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo;
#endif
    }
    else {
        r = 0.0;
    }
    return p;
}
```

The `RELIABLE_FUSED_MULTIPLY_SUBTRACT_OPERATOR` macro is intentionally
conservative -- it is defined only on platforms where the FMS is *verified*
to round correctly. On platforms with software-emulated FMA (e.g. some
MinGW configurations) the software FMA can be a few ULPs off, breaking the
EFT invariant, so the split form is preferred there. This choice is
invisible to the consumer but visible in the throughput numbers.

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

Universal implementation:

```cpp
constexpr inline void three_sum(double& x, double& y, double& z) {
    double u, v, w;
    u = two_sum(x, y, v);
    x = two_sum(z, u, w);    // x = r0 (sum)
    y = two_sum(v, w, z);    // y = r1, and z = r2
}
```

The codebase also provides `three_sum2` (returns only the top two limbs)
and `three_sum3` (returns only the top limb) for cases where the lowest
contributions can be discarded -- common in multiplication kernels where
products at the `eps^2` and `eps^3` levels are below the result's
precision target.

Renormalization is the operation that restores the multi-component
representation's invariants after an arithmetic step that may have left
overlapping components. The cascade machinery in `floatcascade.hpp`
provides several renorm variants tailored to expansion length.

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
CONSTEXPRESSION dd& operator+=(const dd& rhs) {
    double s2;
    hi = two_sum(hi, rhs.hi, s2);
    if (is_finite_cx(hi)) {
        double t2, t1 = two_sum(lo, rhs.lo, t2);
        lo = two_sum(s2, t1, t1);
        t1 += t2;
        three_sum(hi, lo, t1);
    }
    else {
        lo = 0.0;
    }
    return *this;
}
```

`CONSTEXPRESSION` is a macro that expands to `constexpr` on toolchains where
the constexpr_math facility supports the operation and to nothing
otherwise. `is_finite_cx` is the constexpr-safe `std::isfinite` replacement
(`std::isfinite` is not constexpr in C++20).

`dd` multiplication uses **three** `two_prod` invocations plus one regular
product for the cross-term at `O(eps^2)` precision (which is below `dd`'s
output precision), then `three_sum` to settle the result:

```cpp
CONSTEXPRESSION dd& operator*=(const dd& rhs) {
    double p[7]{};
    p[0] = two_prod(hi, rhs.hi, p[1]);
    if (is_finite_cx(p[0])) {
        p[2] = two_prod(hi, rhs.lo, p[4]);
        p[3] = two_prod(lo, rhs.hi, p[5]);
        p[6] = lo * rhs.lo;                  // O(eps^2): plain product, no EFT
        three_sum(p[1], p[2], p[3]);
        p[2] += p[4] + p[5] + p[6];
        three_sum(p[0], p[1], p[2]);
        hi = p[0];
        lo = p[1];
    }
    else {
        hi = p[0]; lo = 0.0;
    }
    return *this;
}
```

The hand-optimization shows up clearly: the four `lo * rhs.lo` partial
product is computed plain (no `two_prod`) because its residual is at
`O(eps^3)`, below `dd`'s `O(eps^2)` output precision -- the EFT would only
collect bits the renormalization throws away. A generic
`expansion_multiply(N)` routine would compute that residual anyway.

### 3.2 Quad-double (qd)

A 4-tuple. ~212 bits of significand, ~62 decimal digits. The hand-crafted
multiplication kernel
(`approximate_multiplication` in `include/sw/universal/number/qd/qd_impl.hpp`)
uses **6 `two_prod` invocations** for the leading partial products, several
`three_sum` calls to cluster magnitudes by significance level, and a final
renormalization. The kernel deliberately *omits* the lowest-order EFTs --
products of the form `a[i] * b[j]` where `i + j >= 3` are accumulated as
plain `O(eps^3)` contributions because their residuals are below the qd
precision target.

A second kernel, `accurate_multiplication`, uses **10 `two_prod`
invocations** -- the additional four capture the lowest-order EFT residuals.
This kernel is selected at compile time by defining `ACCURATE_MULTIPLICATION`.
The default `qd` `operator*=` chooses `approximate_multiplication` (faster,
matches the published Bailey/Hida QD library behavior). Generic adaptive
code would compute the full N^2 = 16 EFTs and discard most of them during
renormalization; the hand-crafted version skips that work.

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

## 5. Universal's unification: floatcascade<N>

Universal's design choice is to express the fixed-size multi-component
types on a shared building block. The fixed-size case (Bailey/Hida) and
the adaptive case (Shewchuk) currently live in *separate* implementations
that share only the EFT primitives via `error_free_ops.hpp` -- a unified
cascade abstraction spans the fixed types, but the adaptive `ereal`
manages its own storage.

### 5.1 The fixed-size building block

The cascade abstraction is `floatcascade<N>` in
`include/sw/universal/internal/floatcascade/floatcascade.hpp`. It is a
single template parameterized only on component count -- the components
are always `double`:

```cpp
namespace sw::universal {

    template<size_t N>
    class floatcascade {
    private:
        // Components stored in DECREASING order of magnitude:
        // e[0] >= e[1] >= ... >= e[N-1].
        // Value = sum(e[0..N-1]).
        std::array<double, N> e;

    public:
        constexpr floatcascade();
        explicit constexpr floatcascade(double x);
        explicit constexpr floatcascade(const std::array<double, N>& components);

        // Component access
        constexpr double  operator[](size_t i) const noexcept;
        constexpr double& operator[](size_t i);
        constexpr size_t  size() const noexcept { return N; }
        // ...
    };

    // Expansion arithmetic primitives that take floatcascade<N> by reference
    // live in namespace expansion_ops inside the same header (renormalize,
    // add_cascades, multiply_cascades, etc.).

}  // namespace sw::universal
```

There is no `Limb` template parameter -- Universal's cascade types are
`double`-only. There is also no `internal/floatcascade/math/` subdirectory;
cascade math (transcendentals, etc.) lives per-type under
`include/sw/universal/number/<type>/math/`.

### 5.2 Fixed-N wrappers built on the cascade

The user-facing fixed-size types `dd_cascade`, `td_cascade`, `qd_cascade`
each contain a `floatcascade<N>` as a private member and add the
number-system veneer (specific-value handling, IEEE-style classifiers,
string parse/format, etc.):

```cpp
class dd_cascade {
    floatcascade<2> cascade;
public:
    static constexpr unsigned nbits = 128;
    static constexpr unsigned es    = 11;
    static constexpr unsigned fbits = 106;

    constexpr dd_cascade(double h, double l) noexcept : cascade{} {
        cascade[0] = h; cascade[1] = l;
    }
    // ... arithmetic, conversion, classifiers
};

// Similar wrappers:
//   td_cascade  -- contains floatcascade<3>
//   qd_cascade  -- contains floatcascade<4>
```

The legacy `dd` and `qd` types in `include/sw/universal/number/dd/` and
`include/sw/universal/number/qd/` predate the cascade abstraction and keep
their hand-crafted Bailey/Hida operator implementations (calling the EFTs
directly, not going through `floatcascade<N>`). The `*_cascade` family is
the modernized rewrite of the same precision tiers on top of the shared
cascade machinery.

There is currently no implicit conversion between `dd` and `dd_cascade`,
between `dd_cascade` and `qd_cascade`, etc. Each type is independent at
the user-facing API level -- they share the EFT primitives, not class
identity.

### 5.3 The adaptive type (ereal)

The adaptive case is `ereal<maxlimbs>`, templated on the maximum number
of components (default 8, hard ceiling 19 -- past which the smallest limb
underflows below `DBL_MIN` and the EFTs break). Storage is
`std::vector<double>` directly, *not* a `floatcascade` variant:

```cpp
template<unsigned maxlimbs = 8>
class ereal {
    static_assert(maxlimbs <= 19,
        "maxlimbs must be <= 19 to maintain algorithmic correctness...");
private:
    std::vector<double> _limb;     // expansion in decreasing magnitude order
public:
    constexpr ereal();
    ereal(double iv) noexcept;
    ereal(const std::string& str);
    // ... Shewchuk-style expansion arithmetic via grow_expansion etc.
};
```

`ereal` does *not* contain a `floatcascade` and does *not* expose
direct conversion to or from the cascade types. The shared substrate is
purely the EFT primitives in `error_free_ops.hpp` plus the Shewchuk-style
expansion operations (`grow_expansion`, `scale_expansion`,
`fast_expansion_sum`, `compress`).

### 5.4 What the architecture does and does not deliver today

What the shared substrate gives you:

- One implementation of `two_sum`, `quick_two_sum`, `two_prod`,
  `three_sum`, `split`, etc. -- shared by `dd`, `qd`, `dd_cascade`,
  `td_cascade`, `qd_cascade`, and `ereal`.
- One implementation of the cascade arithmetic for fixed-N, shared by
  `dd_cascade`, `td_cascade`, `qd_cascade`.
- Compile-time selection of FMS vs split-form `two_prod` based on a
  verified-platform macro.

What is *not* unified today (worth knowing if you read the design doc
`docs/floatcascade-design.md`, which sketches a more ambitious
unification):

- No implicit `dd <-> dd_cascade` / `dd_cascade <-> ereal` conversions.
  The doc-document proposal of a single `expansion_base<N>` hierarchy
  spanning fixed and adaptive types is aspirational; the shipped code
  has separate fixed and adaptive paths sharing only the EFT layer.
- No `to_dd()` / `to_qd()` / `to_td()` extraction helpers on `ereal`.
- No `AdaptivePrecision` configuration struct on `ereal` carrying
  tolerance / max-components / max-iterations. The hard cap is the
  `static_assert(maxlimbs <= 19)` and the operation-internal iteration
  limits (e.g. fixed Newton-Raphson iteration counts in `sqrt` /
  division).

## 6. Comparison summary

| Aspect                    | Priest (theory)       | Bailey/Hida (`dd`, `qd`)            | Shewchuk (adaptive `ereal`)     | McCleeary (lazy `elreal`)            |
|---------------------------|------------------------|-------------------------------------|---------------------------------|--------------------------------------|
| Precision                 | Variable (conceptual) | Fixed (2 or 4 components)            | Dynamic (grows as needed)       | Lazy on demand (per-call refinement) |
| Primitives                | Defined the EFTs       | Uses EFTs in fixed hand-crafted patterns | Uses EFTs with expansion growth | Uses EFTs in a generator-driven stream |
| Storage                   | Theoretical            | `std::array<double, N>`              | `std::vector<double>`            | `std::vector<double>` + generator    |
| Per-op cost               | N/A                    | Constant time, small constant         | Variable, common case fast      | Common case is depth-0 (cheap)        |
| Per-op storage            | N/A                    | Constant                              | Grows with input separation     | Grows with refinement depth          |
| Determinism               | N/A                    | Constant time and space               | Variable time and space         | Bounded by refinement budget         |
| Vectorization             | N/A                    | SIMD-friendly                         | Hard to vectorize               | Hard to vectorize                    |
| Cache locality            | N/A                    | 16-32 bytes per value                 | Variable, may exceed L1         | Variable, grows with depth           |
| Use case                  | Academic foundation    | HPC, physics, ML mixed-precision      | Computational geometry, CAD/CAM | Geometric predicates, undecidable comparison, validation oracle |
| Worst-case timing         | N/A                    | Same as common case                   | 2-3.5x common case              | Budget-bounded refinement walk       |
| Error control             | Proven bounds          | Pre-set precision                     | Adaptive until bound met        | Per-call refinement budget           |
| Universal type            | -                      | `dd`, `qd`, `dd_cascade`, `td_cascade`, `qd_cascade`  | `ereal`              | `elreal`                              |

The four approaches are complementary, not competing. Priest provided the
theoretical foundation. Bailey/Hida productionized it for the
known-precision case. Shewchuk extended it to the variable-precision case.
McCleeary added the lazy paradigm for the precision-on-demand case.
Universal ships all four:

- Bailey/Hida via the original hand-crafted `dd`/`qd` and the
  cascade-based `dd_cascade`/`td_cascade`/`qd_cascade` rewrite
- Shewchuk via `ereal<maxlimbs>`
- McCleeary via `elreal` (epic #873, A-G shipped)

## 7. Picking a type

Use the following decision tree:

| If your workload ...                                                | use                  |
|---------------------------------------------------------------------|----------------------|
| needs ~106 bits of significand, knows it upfront, in a hot loop      | `dd` (Bailey/Hida)   |
| needs ~212 bits of significand, knows it upfront                     | `qd` (Hida/Li/Bailey)|
| same precision targets via the unified cascade framework             | `dd_cascade`, `td_cascade`, `qd_cascade` |
| needs ~159 bits (triple-double)                                      | `td_cascade`         |
| has computational-geometry predicates where input separation varies  | `elreal` (cheap common case) or `ereal` (eager expansion) |
| needs adaptive precision up to ~303 decimal digits, committed upfront | `ereal<19>`         |
| needs precision-on-demand with budget-bounded comparison              | `elreal`             |
| validating another type's math function via cross-implementation oracle | `elreal` via `check_against_elreal_oracle` |

For the typical numerical-analysis workload (HPC, ML training, physics
simulation) the answer is `dd` or `qd`. For computational-geometry work
the choice between `ereal` and `elreal` depends on whether the inputs
are typically general-position (then `elreal`'s depth-0 fast path wins)
or adversarially near-degenerate (then `ereal`'s eager expansion wins;
the cost is paid every call but is bounded by the type's `maxlimbs`).
For undecidable comparison (e.g. symbolic-system reals constructed via
different algebraic paths), `elreal`'s budgeted comparison is the
right tool.

### 7.1 The elreal-vs-ereal throughput crossover (post-Phase-K.2)

Per-operation throughput for `elreal` and `ereal<N>` at matched
precision on a 12th Gen Intel i7-12700K (gcc 13.3, clang 18.1, `-O3`),
after the Phase K.1 (inline `_components` buffer, #912) and Phase K.2
(tagged-union generator, #916) optimisations. The Phase I baseline
(#902) measured the pre-optimisation numbers; both the baseline and
the current numbers, plus the reproduction recipe, live in
`docs/algorithmic-details/elreal-performance-baseline.md`. The summary
shape for picker purposes:

| Op | `elreal` post-K.2 | `ereal<2>` (~106 bits) | Winner |
|---|---:|---:|---|
| `+` | ~17 Mops/s | ~19 Mops/s | `ereal<2>` (~ 1.1x; gap nearly closed) |
| `-` | ~17 Mops/s | ~20 Mops/s | `ereal<2>` (~ 1.2x) |
| `*` | ~16 Mops/s | ~11 Mops/s | **`elreal` (~ 1.5x)** |
| `/` (elreal depth 0 only) | (dominated by inlining) | ~680 Kops/s | `elreal` (apples-to-oranges) |
| `sqrt`, `exp`, `log` | ~36-43 Mops/s | n/a | `elreal` (ereal has no math functions) |

(All numbers gcc 13.3 on a 12th Gen i7-12700K post-Phase-K.2 of #903;
both sides constructing fresh operands inside the loop body.
Reproduction: `make benchmark_elreal_performance`. Phase I baseline
numbers before K.1/K.2 are in
`docs/algorithmic-details/elreal-performance-baseline.md`.)

Two reads from the table:

1. **The matched-precision arithmetic gap has nearly closed.** K.1
   eliminated the `_components` vector alloc; K.2 eliminated the
   `std::function` heap alloc by replacing it with a `std::variant`
   of small POD shapes. `elreal` arithmetic is now within ~ 20% of
   `ereal<2>` on `+`/`-` and wins by 1.5x on `*` (because
   `ereal<N>` multiplication is still O(N) in the eager expansion
   product).
2. **What `elreal` actually wins on is *correctness*, not throughput.**
   Decidable sign (Section 4 of `lazy-real-arithmetic.md`),
   precision-on-demand without committed-upfront budget, and access to
   `sqrt`/`exp`/`log`/`pow` and the geometric predicates -- none of
   which `ereal<N>` provides.

So the picker rule, refined: after K.2, `elreal` is both
throughput-competitive at matched precision *and* exposes correctness
features `ereal<N>` lacks (math functions, decidable sign, geometric
predicates, oracle validation). Choose `elreal` when any of those
capabilities matters. Choose `ereal<N>` when raw arithmetic at a
committed-upfront precision is the workload's hot path and the math
suite isn't needed.

Note: Universal does not currently provide implicit conversion between
`dd`, the cascade types, `ereal`, and `elreal`. Users that need to
switch tiers do so by going through `double` (e.g. `dd d = double(e);`)
-- which is lossy at the conversion point but unavoidable given the
current API.

## 8. Validation strategy

Universal does *not* use MPFR or Boost.Multiprecision as an external oracle.
The library is header-only with no required external dependencies, and the
validation strategy reflects that constraint. Instead, multi-component types
are validated by four distinct strategies depending on the type and the
operation. Each has explicit strengths and explicit ceilings.

### 8.1 Algebraic identities at full type precision (primary strategy)

The strongest tests in the codebase. They exploit the fact that certain
identities must hold *exactly at the type's precision* if the implementation
is correct, so no external higher-precision oracle is needed.

For the cascade types (`dd_cascade`, `td_cascade`, `qd_cascade`), the
infrastructure is in `static/highprecision/<type>/arithmetic/corner_cases.hpp`:

| Property                          | Tolerance (`DD_EPS = 2^-106`)        |
|-----------------------------------|--------------------------------------|
| `(a + b) - b == a`                | `10 * DD_EPS * |a|`                  |
| `(a - b) + b == a`                | `10 * DD_EPS * |a|`                  |
| `a - a == 0` (all components)     | exact                                |
| `(a * b) / b ~ a`                 | `100 * DD_EPS * |a|`                 |
| `a / a == 1`                      | `100 * DD_EPS`                       |
| `1 / (1 / a) ~ a`                 | `10000 * DD_EPS * |a|`               |
| `a * b == b * a`                  | `10 * DD_EPS * max`                  |
| `(a * b) * c == a * (b * c)`      | `100 * DD_EPS * max`                 |
| `a * (b + c) == a*b + a*c`        | `100 * DD_EPS * max`                 |
| `a * (power of 2)`                | bit-exact (must be exact in IEEE-754) |
| components non-overlapping         | structural (cascade invariant)       |

`DD_EPS` is the type's *own* epsilon, not double's -- so these tolerances
exercise full-precision behavior. `corner_cases.hpp` discusses why this
approach beats random-pair vs double comparison; the relevant excerpt is
worth quoting directly:

> Comparing dd_cascade arithmetic results to double references is
> fundamentally flawed: the reference is less precise than what we're
> testing, [so] differences in the lower ~106 bits appear as "failures"
> when they're actually correct.

For `ereal`, the equivalent infrastructure is in
`elastic/ereal/arithmetic/identities.cpp` -- which does *component-wise
exact equality* on EFT identities like `(a + b) - a == b`. That is the
strongest correctness check in the codebase.

### 8.2 Random pairs vs native double (legacy `dd` / `qd`)

The older `dd` and `qd` types use
`VerifyBinaryOperatorThroughRandoms<Type>` from
`include/sw/universal/verification/test_suite_randoms.hpp`. It generates
random bit patterns, computes the operation at the multi-component type
*and* at native `double`, and compares:

```cpp
testa.setbits(distr(eng));       // random multi-component
double da = double(testa);       // lossy
testresult = testa + testb;      // multi-component add
reference  = da + db;             // double add
testref    = reference;          // back into multi-component
if (testresult != testref) ++failed;
```

This only catches *gross* failures (sign flips, wrong exponent, NaN
propagation). It cannot detect lost precision in the lower components, by
construction. The cascade types replaced this pattern with the algebraic
identities above for exactly that reason.

### 8.3 Round-trip and cross-implementation tests (math functions)

For transcendentals and `sqrt`, where there is no simple algebraic identity
that recovers the input, the cascade math tests use two complementary
patterns:

- **Round-trip**: e.g. `(sqrt(a))^2 ~ a`, `exp(log(a)) ~ a`, `a / b * b ~ a`.
  Validates at the type's own precision because both forward and inverse run
  through the same precision.
- **Cross-implementation**: e.g.
  `internal/floatcascade/arithmetic/sqrt_precision_test.cpp` compares Karp's
  trick against Newton-Raphson on the same cascade type. Disagreement
  between two independent implementations of the same function is a strong
  signal of a bug in one of them.

Many of these tests are still under `MANUAL_TESTING 1` (print-and-eyeball
runs rather than automated pass/fail) -- this is a known gap.

### 8.4 Spot checks against `std::*` library functions (limited tier)

`elastic/ereal/math/functions/exponent.cpp` and the cascade math tests
include sanity-check cases against `std::exp(double)`, `std::log(double)`,
etc. The helper is `check_relative_error` from
`include/sw/universal/verification/test_suite_mathlib_adaptive.hpp`. The
tolerance threshold scales with the type's `digits10`, but the *comparison
itself* casts both sides to double:

```cpp
double expected_val = double(expected);    // both sides to double
double result_val   = double(result);
double rel_error    = abs((result_val - expected_val) / expected_val);
return rel_error < threshold;
```

This bounds the *verifiable* precision at ~15 to 16 decimal digits, even
though `dd` advertises 31 and `ereal<19>` advertises 303. Past double's
range, these tests can only confirm the result is *not wildly wrong*; they
cannot confirm it is *as accurate as the type claims*.

`elastic/ereal/ereal_numerics.md` is explicit about this:

> For precision beyond `ereal<19>`, external validation against
> arbitrary-precision libraries (MPFR, Boost.Multiprecision) would be
> required.

I.e., MPFR is listed as aspirational future work -- not as a current
dependency.

### 8.5 Hand-built ground-truth cases (geometric predicates)

`elastic/ereal/geometry/predicates.cpp` validates `orient2d` and friends
by constructing configurations with geometrically obvious sign (left turn,
right turn, collinear) and asserting the predicate returns the expected
sign. The test cases *are* the oracle; no numeric reference is needed.

### 8.6 elreal as a cross-implementation oracle (Phase G)

The validation tier added by Phase G of the `elreal` epic (#880). The
helper at `include/sw/universal/verification/test_suite_elreal_oracle.hpp`
takes a value computed in any Universal multi-component type and an
`elreal` reference computed via an *independent* code path, and asserts
agreement within the target type's `numeric_limits::digits10` precision:

```cpp
template<typename TargetType>
bool check_against_elreal_oracle(const TargetType& target,
                                 const elreal& oracle,
                                 int safety_margin = 2);
```

The validation it provides today is at double precision -- the same
ceiling as `check_relative_error` -- because elreal's lazy refinement
currently caps at depth 1 (per the Phase C / Phase E docblocks). The
distinguishing property is *cross-implementation*: the reference is
computed by elreal's lazy-arithmetic backend, not by the std library
directly, so an agreement at the helper's tolerance is genuine
two-paths confirmation, not a self-check.

What this catches today:

- Target-type implementations that produce a wrong sign or wrong
  magnitude (catastrophic bugs); both paths diverge at double precision.
- Target-type implementations that match `std::cmath` but disagree with
  elreal's algebraic path -- a subtle-algorithmic-issue signal.
- Regressions where the target type's precision drops below double's
  ~16 decimal digits.

What it doesn't yet catch (deferred until elreal gains depth-2+
refinement):

- Sub-double-ULP precision drift in the target type. Catching this
  requires elreal to deliver >53 bits, which is the depth-2+ follow-up
  filed against the `elreal` epic.

The demo test at `elastic/elreal/oracle/dd_cascade_exp.cpp` exercises
the pipeline end-to-end on `dd_cascade::exp(x)` versus `elreal::exp(x)`
across a representative input set, including a sanity-check assertion
that the helper rejects a deliberately-wrong oracle.

### 8.6 Where the regression tests live

| Type            | Arithmetic                                            | Math                                            |
|-----------------|-------------------------------------------------------|-------------------------------------------------|
| `dd`            | `static/highprecision/dd/arithmetic/`                  | `static/highprecision/dd/math/`                  |
| `qd`            | `static/highprecision/qd/arithmetic/`                  | `static/highprecision/qd/math/`                  |
| `dd_cascade`    | `static/highprecision/dd_cascade/arithmetic/`          | `static/highprecision/dd_cascade/math/`          |
| `td_cascade`    | `static/highprecision/td_cascade/arithmetic/`          | `static/highprecision/td_cascade/math/`          |
| `qd_cascade`    | `static/highprecision/qd_cascade/arithmetic/`          | `static/highprecision/qd_cascade/math/`          |
| `ereal`         | `elastic/ereal/arithmetic/`                            | `elastic/ereal/math/functions/`                  |
| FloatCascade    | `internal/floatcascade/arithmetic/`                    | (cascade math via the cascade types above)       |

The cascade types share the corner-case infrastructure pattern; each
`<type>/arithmetic/corner_cases.hpp` is type-specialized.

### 8.7 Known gaps

In the interest of an honest picture:

- **Higher-precision oracle gap: PIPELINE in place, PRECISION CEILING
  pending.** The elreal-as-oracle tier added by Phase G (section 8.6)
  establishes the validation pipeline -- cross-implementation agreement
  is now part of the validation contract for `dd_cascade::exp` and is
  extensible to other multi-component types. The actual precision of the
  oracle is currently capped at double (elreal's depth-1 cap), so
  sub-double-ULP precision drift in target types is still uncatchable
  by automated tests. Lifting that cap is the depth-2+ refinement work
  filed against the elreal epic (Phase F-or-later); when it lands, the
  oracle's precision improves transparently and existing oracle tests
  pick it up without API changes.
- **Manual-testing scaffolding.** A non-trivial fraction of the math tests
  are still in `MANUAL_TESTING 1` mode -- they print results for human
  inspection rather than asserting pass/fail. Automating these is open work.
- **Cross-cascade validation aspirational.** The `corner_cases.hpp` comment
  block lists "use qd as oracle for dd_cascade" as strategy 5, but the
  shipped tests do not yet exercise that path. The Phase G helper provides
  an alternative: use elreal (rather than qd) as the cross-implementation
  oracle for cascade-type validation.

MPFR / Boost.Multiprecision integration remains a natural complement for
the high-precision-ceiling problem once elreal's depth-2+ refinement
lands; it would provide a third independent precision reference. Not
currently in the build or the validation path.

## 9. Geometric predicates: ereal vs elreal

Phase F of the elreal epic (#873, #879) shipped a second implementation
of the four classical geometric predicates: `orient2d`, `orient3d`,
`incircle`, `insphere`. The two paths represent the two main exact-
arithmetic philosophies discussed throughout this document.

### The two paths

`ereal` (Shewchuk-style expansion arithmetic) lives at
`include/sw/universal/number/ereal/geometry/predicates.hpp`. The
predicate evaluates the determinant via expansion arithmetic; the result
is an `ereal<N>` whose components are the Shewchuk expansion. Sign is
the sign of the leading non-zero component.

`elreal` (McCleeary-style lazy refinement) lives at
`include/sw/universal/number/elreal/geometry/predicates.hpp`. The
predicate evaluates the same determinant via the lazy operators
(Phase C), then calls `sign(result, budget)` (Phase D), which walks
the lazy stream to the first non-zero component.

The implementations are syntactically near-identical -- the determinant
expressions are the same, only the underlying real type differs.

### Behavioural comparison

**General-position inputs** (vertices in clearly distinct quadrants,
points well inside or outside circles). Both paths terminate at depth 0:

- `ereal`: produces an expansion whose leading component has the
  decisive sign; the expansion is trimmed to its leading term as the
  predicate returns.
- `elreal`: produces an elreal whose `at(0)` has the decisive sign;
  `sign(result, default_budget = 8)` walks only the leading component.

Cost is roughly the same: a handful of double-precision multiplies and
sums.

**Near-degenerate inputs** (almost-collinear / almost-coplanar /
almost-cocircular). The two paths diverge:

- `ereal` accumulates expansion limbs as the result narrows. The
  expansion may grow to many components before the leading non-zero
  is identified. Worst-case cost for `insphere` can hit ~16 components.
- `elreal` walks the lazy stream depth by depth; each `at(k)` call
  generates one more component on demand. The refinement budget bounds
  the worst case; the default 8-component budget covers the
  near-degenerate range for all four predicates.

**Exactly-degenerate inputs** (collinear, coplanar, cocircular,
cospherical configurations whose determinant evaluates to exactly zero
with integer-valued coordinates). Both paths return 0 cleanly:

- `ereal`: the expansion is all-zero.
- `elreal`: every component in the stream is zero; `sign` returns 0
  once the budget is exhausted.

For the four hand-built test cases in
`elastic/elreal/geometry/predicates.cpp` and the analogous
`elastic/ereal/geometry/predicates.cpp`, both paths produce identical
signs on every input -- as required by Phase F's cross-validation
acceptance criterion.

### When to prefer which

- **ereal** when the precision target is *known up front* and the cost
  of one-shot expansion building is acceptable. Best fit for batch
  geometric processing where every predicate goes to maximum precision.
- **elreal** when the *common case dominates*. The lazy refinement
  cheap-paths general-position queries at depth 0 and only spends the
  budget on the truly near-degenerate configurations. Best fit for
  mesh generation pipelines where general-position predicates outnumber
  near-degenerate ones by orders of magnitude.

A formal benchmark study would quantify this; the natural follow-up is
Phase I (#882). For Phase F, the equivalence of signs across the four
predicates and the divergence of cost in the near-degenerate band is
the qualitative comparison the issue asked for.

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
