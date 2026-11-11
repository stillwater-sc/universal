# LNS Add/Sub Algorithms

LNS makes multiplication and division trivial -- they are integer
operations on the exponent. The hard problem is *addition*: it requires
computing a transcendental correction term `sb_add(d) = log2(1 + 2^d)`
(and `sb_sub(d) = log2(1 - 2^d)` for mixed-sign / subtraction), and
there are many ways to compute these corrections, each with a different
accuracy / SRAM / latency / energy trade. See the [Introduction](../lns/)
for the Gauss log-add derivation that produces these functions, and for
the [Naming convention](../lns/#how) note explaining the `sb_` prefix.

Universal ships a configurable framework that lets users select, per
`lns<>` instantiation, which `sb_add` / `sb_sub` implementation is used.
This page covers that framework: the customization point, the eight
shipped algorithms, and the decision tree for picking one.

For the math behind why log-domain addition is the hard case, see
[Introduction](../lns/). For how the regression suite handles the
inherent approximation in non-exact algorithms, see
[Algorithm Tolerance](../lns-tolerance-traits/).

## Customization point: `lns_addsub_traits`

`lns_addsub_traits<Lns>` is the per-instantiation customization point.
The default selects `DoubleTripAddSub` (the historical placeholder, see
below), preserving prior behavior for any code that does not explicitly
opt in. Users override per type via specialization:

```cpp
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/lns_addsub_algorithms.hpp>

namespace sw::universal {
    template<>
    struct lns_addsub_traits<lns<16, 8, std::uint16_t>> {
        using type = LookupAddSub<lns<16, 8, std::uint16_t>, 12>;
    };
}
```

The `lns::operator+=` and `lns::operator-=` methods route through
`lns_addsub_algorithm_t<lns>::add_assign(*this, rhs)`, so a single
trait specialization changes the behavior of every `+`, `-`, `+=`, and
`-=` for that specific `lns<>` type. Other instantiations are
unaffected.

A traits class was chosen over a fifth template parameter because the
`lns<>` template signature is `lns<_nbits, _rbits, bt, auto... xtra>`
and the `auto... xtra` parameter pack must remain last; adding a fifth
parameter would break every existing site that pins the `xtra...`
arguments. The traits-class pattern -- the same as `std::char_traits`
for `std::basic_string` -- is zero-breaking and self-documenting.

## The shared dispatcher

All algorithms share the same special-value routing and sign dispatch
through `detail::gauss_log_add<Policy>`. A new policy only needs to
provide the per-algorithm correction functions:

```cpp
template<typename Lns>
struct MyAddSub {
    static constexpr double sb_add(double d);   // log2(1 + 2^d) for d <= 0
    static constexpr double sb_sub(double d);   // log2(1 - 2^d) for d <  0

    static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs) {
        double r = detail::gauss_log_add<MyAddSub>(double(lhs), double(rhs));
        return lhs = r;
    }
    static constexpr Lns& sub_assign(Lns& lhs, const Lns& rhs) {
        double r = detail::gauss_log_add<MyAddSub>(double(lhs), double(-rhs));
        return lhs = r;
    }
};
```

The dispatcher handles NaN propagation, +/-0 short-circuits, +/-inf
delegation to native double arithmetic, and the same-sign /
mixed-sign / cancellation routing.

## Shipped algorithms

Universal ships eight algorithms covering the full SRAM-vs-accuracy
trade-off space, plus a hardware-codesign tier and a fast-faithful-rounding
tier. All are header-only, and all but `CORDICDatapathAddSub` are
constexpr-friendly.

### 1. `DoubleTripAddSub` (default)

Casts both operands to `double`, computes the sum / difference with
native `+` / `-`, casts back. Pays a full encode / decode for every
operation. This is the historical placeholder from before the
`constexpr_math` facility (Epic #763) existed; it is the default trait
so existing `lns` users see no behavior change.

Useful as a correctness oracle for cross-validating the other
algorithms.

```cpp
template<typename Lns>
struct DoubleTripAddSub {
    static constexpr Lns& add_assign(Lns& lhs, const Lns& rhs) {
        double sum = double(lhs) + double(rhs);
        return lhs = sum;
    }
    // ...
};
```

### 2. `DirectEvaluationAddSub`

Implements the Gauss log-add formulation directly via
`sw::math::constexpr_math::log2` and `cm::exp2`. Two transcendentals
per add, no approximation; accuracy is dominated by the
`cm::log2` / `cm::exp2` implementations (a few ULP at double precision).

The recommended default for high-precision software targets, and the
correctness oracle that the faster algorithms cross-validate against.

```cpp
template<typename Lns>
struct DirectEvaluationAddSub {
    static constexpr double sb_add(double d) {
        return cm::log2(1.0 + cm::exp2(d));
    }
    static constexpr double sb_sub(double d) {
        double t = 1.0 - cm::exp2(d);
        return cm::log2(t);
    }
    // ...
};
```

### 3. `LookupAddSub`

The original Mitchell 1962 LNS algorithm in modern form: precompute
`sb_add(d)` on a finite d-grid at compile time, look up + linearly
interpolate at runtime.

```cpp
template<typename Lns,
         unsigned IndexBits = ((Lns::rbits + 2 < 10) ? Lns::rbits + 2 : 10)>
struct LookupAddSub { /* ... */ };
```

`IndexBits` (log2 of table entries) defaults to `min(rbits + 2, 10)` so
the default table is modest (1024 entries = 8 KB at double precision);
users override per instantiation to trade accuracy for SRAM. The
d-range is `[-rbits-2, 0]` because past that point the correction is
below LNS ULP and rounds to 0 anyway.

`sb_sub` falls back to direct `cm::log2` / `cm::exp2` evaluation in the
lowest-cell cancellation regime (`|d| < step`) where linear
interpolation has unbounded error. This costs one transcendental pair
per sub but only in that narrow band.

### 4. `PolynomialAddSub`

Closed-form, no table. Uses the classical log-domain identity

```text
log2((1+x)/(1-x)) = (2/ln 2) * (x + x^3/3 + x^5/5 + x^7/7 + ...)
```

with the substitution `x = u/(2+u)` where `u = 2^d`, mapping
`u in (0, 1]` to `x in (0, 1/3]`. The series converges fast over the
reduced domain: degree-7 truncation has worst-case error
`x^9/9 = (1/3)^9 / 9 ~= 5.6e-6` in the log domain.

For `sb_sub`, the analogous substitution `x = u/(2-u)` requires
`u <= 0.5` to keep `x` in `[0, 1/3]`; past that we are in the
catastrophic-cancellation regime where the series diverges, and the
implementation falls back to direct `cm::log2` / `cm::exp2` -- same
hybrid pattern as `LookupAddSub`.

Cost: one transcendental at runtime (the `2^d` to recover `u` from
`d`), one division. No table.

### 5. `ArnoldBaileyAddSub`

Cheapest of the family: piecewise-linear approximation matching
`sb_add` at integer-d knots `(d = 0, -1, -2, -3, -4, -5)`. Knot values
are computed at compile time via `cm::log2` so we do not bake in
approximate constants. Each piece evaluates as `a + b*d` -- two
multiplies, one add, no transcendentals at runtime, no table, no
division.

Worst-case error ~2.5% relative near `d = 0` (where the curvature of
`log2(1 + 2^d)` is highest), dropping rapidly toward the tail.
`sb_sub` is analogous, with a direct-evaluation fallback in the
cancellation regime `u > 0.5`.

The Arnold-Bailey 1990s LNS arithmetic series in IEEE Trans. Computers
proposed several closed-form, no-table `sb_add` approximations of this
style as the foundation for LNS hardware co-processors. Universal's
implementation is "in the style of" that family rather than a direct
port of any single paper -- it picks a small, hand-readable knot set
sufficient to bound worst-case error at ~2.5% over the LNS dynamic
range.

### 6. `CORDICAddSub` (hardware-codesign tier)

Classical hyperbolic CORDIC: bit-by-bit refinement using only adds,
shifts, and a small table of `atanh(2^-k)` constants. Two passes --
rotation mode to compute `2^d` via `exp(d * ln 2)`, then vectoring mode
to compute `log2(1 +/- v)`. Iterations 4, 13, and 40 are repeated to
ensure convergence (this is a standard hyperbolic-CORDIC requirement,
not an implementation quirk).

```cpp
template<typename Lns, unsigned MaxIterations = Lns::rbits>
struct CORDICAddSub { ... };
```

`MaxIterations` is exposed as a non-type template parameter so a
hardware-codesign team can study truncated iteration budgets
independently of `rbits`. The default `MaxIterations = rbits` matches
"one iteration per bit of precision".

CORDIC is **slow per operation in software** -- one dependent iteration
per rbit, no SIMD or pipeline parallelism. It exists because it maps
directly to the area / latency model of an FPGA or ASIC LNS pipeline,
where each iteration is one cycle of a fully-bypassed datapath with no
transcendental hardware. Use it when you are targeting hardware and
need the cost model to be representative; for software targets, prefer
`Polynomial` or `Lookup`.

The cancellation regime (`d in (-1, 0)` for `sb_sub`) falls back to
direct evaluation via `cm::log2` / `cm::exp2`, matching `Lookup`,
`Polynomial`, and `ArnoldBailey`. Hardware retargets would substitute a
co-transformation or small lookup in that band rather than a
transcendental. `CORDICDatapathAddSub` (section 8) computes that band
with CORDIC on a wider fixpnt bus.

See `docs/design/cordic-precision-assessment.md` for the per-iteration
convergence study, ULP histograms, and worst-case input table across
the rbits sweep.

### 7. `ArnoldCotransformationAddSub` (faithful-rounding tier)

Implements the **Novel Cotransformation Combination** from Vouzis,
Collange, Arnold (J. Signal Processing Systems 58:29-40, 2010). Fills the
"fast + faithful-rounding" quadrant: ~1 ULP of `DirectEvaluation` across
the full `d` domain (including the cancellation regime near `d = 0`),
with zero runtime transcendentals.

```cpp
template<typename Lns,
         unsigned IndexBits = ...,   // sb_add table size, default min(rbits+2, 10)
         unsigned SplitJ    = ...>   // bit-split for cotransformation, default min((rbits+4)/2, 10)
struct ArnoldCotransformationAddSub { ... };
```

The cancellation singularity at `d -> 0` is bypassed by an algebraic
identity: `sb_sub(d)` is re-expressed as a combination of two small LUTs
(`F3`, `F4`) plus a single call to `sb_add` (which has no singularity).
Both LUTs are precomputed at compile time via `cm::log2` / `cm::exp2`, so
the singular values are baked in exactly -- the runtime path only does
lookups and arithmetic.

**When to use:** Anywhere you would have used `DirectEvaluation` for its
accuracy but couldn't afford the two transcendentals per operation --
typical example is LNS subtraction in same-magnitude scenarios (ML
inference, filter design, scientific kernels). Memory cost is the F3 +
F4 + sb_add LUTs, typically a few KB at `rbits <= 16`.

See `docs/design/lns-add-sub.md` Section 7 for the full algorithmic
derivation, references, and LUT-sizing analysis.

### 8. `CORDICDatapathAddSub` (hardware datapath tier)

The same two passes as `CORDICAddSub`, run through the fixpnt CORDIC
engine (`include/sw/universal/number/fixpnt/math/cordic.hpp`) instead of
`double`, so the policy models the datapath and not only the algorithm:
every shift truncates, the guard width and the iteration count are the
hardware's, and each unit rounds its result once. It is opt-in, because it
brings in fixpnt, and it is not `constexpr`:

```cpp
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/lns_addsub_cordic_datapath.hpp>

template<typename Lns,
         unsigned Iterations = 0,                    // 0: the engine's default for each unit
         unsigned Guard      = cordic::auto_guard>   // bus guard bits, default ceil(log2(rbits + 8)) + 2
struct CORDICDatapathAddSub { ... };
```

Each correction is a chain of units joined by a fixpnt bus with
`rbits + g` fraction bits:

```text
t  = d * ln 2          constant multiplier
v  = cordic::exp(t)    hyperbolic rotation, v = 2^d
w  = 1 +/- v           adder, exact on the bus
L  = cordic::log(w)    hyperbolic vectoring
sb = L * (1 / ln 2)    constant multiplier
```

The cancellation band `d in (-1, 0)` of `sb_sub` is CORDIC too. There,
`1 - 2^d` shrinks to about `ln 2 * 2^-rbits`, so a bus error of `2^-R` in
`v` becomes a relative error of `2^(rbits - R)` in `w`. The band runs on a
bus `rbits + 2` bits wider, which keeps the log-domain error at about
`2^-(rbits + g)`. No path evaluates a transcendental from `<cmath>` or
`constexpr_math`.

Accuracy, as the worst log-domain error in lns units (`2^-rbits`) over
every `d = -k 2^-rbits`:

| configuration | `sb_add` | `sb_sub`, `d <= -1` | `sb_sub` band |
|---|---|---|---|
| `CORDICAddSub<lns<16,8>>` (double, `k = rbits`) | 3.41 | 3.89 | direct evaluation |
| `CORDICDatapathAddSub<lns<16,8>>` | 0.033 | 0.044 | 0.003 |

From `lns<8,4>` to `lns<32,24>` the worst error at the default knobs is
0.044 lns units. The sums and differences then agree with
`DirectEvaluationAddSub` to the encoding: on `lns<8,4>`, exhaustively,
within one encoding step, and on every benchmark sample from `lns<16,8>`
up, exactly.

Both knobs are template parameters. `Iterations` counts the repeated
shifts, as the engine does, and applies to every unit; the band needs the
most, since it amplifies whatever angle is left over by up to
`2^(rbits + 1)`. In `lns<16,8>`:

| iterations | `sb_add` | `sb_sub` | band |
|---|---|---|---|
| 8 | 6.59 | 7.12 | 372 |
| 12 | 0.429 | 0.470 | 24.4 |
| 16 | 0.072 | 0.086 | 1.90 |
| 20 | 0.033 | 0.044 | 0.215 |
| 24 | 0.033 | 0.044 | 0.006 |
| default (30 narrow, 41 wide) | 0.033 | 0.044 | 0.003 |

| guard bits | `sb_add` | `sb_sub` | band |
|---|---|---|---|
| 0 | 1.77 | 2.41 | 0.127 |
| 2 | 0.437 | 0.724 | 0.037 |
| 4 | 0.112 | 0.131 | 0.027 |
| default (6) | 0.033 | 0.044 | 0.003 |

The engine's default iteration count runs every shift down to the
resolution of the unit's own datapath, past the point where the error
stops falling: 24 iterations already reach the default's accuracy in
`lns<16,8>`. Truncated far enough, `2^d` can round to 1, and `sb_sub` then
gives `-inf`, a result of 0.

In software it costs fixpnt emulation, 3 to 8 times `CORDICAddSub`; the
cost that matters is the hardware's, which the knobs model.
`docs/design/cordic-precision-assessment.md` has the tables for every
configuration in the sweep, and the regression test is
`static/logarithmic/lns/arithmetic/cordic_datapath.cpp`.

## Decision tree

| If your target ...                                                   | use                                  |
|----------------------------------------------------------------------|--------------------------------------|
| just needs the `lns<>` API to work, no perf concern                  | `DoubleTripAddSub` (default)         |
| needs full precision, can afford 2 transcendentals per op            | `DirectEvaluationAddSub`             |
| has SRAM for a 1-8 KB table, wants ~5e-5 rel error                   | `LookupAddSub` (default `IndexBits`) |
| has SRAM for a >100 KB table, wants 1-2 ULP accuracy                 | `LookupAddSub<Lns, 16+>`             |
| is SRAM-constrained, wants ~5e-6 abs error, ok with 1 transcendental | `PolynomialAddSub`                   |
| is energy-constrained, wants no transcendentals, ok with ~2.5% rel error | `ArnoldBaileyAddSub`             |
| is targeting FPGA / ASIC and wants the cost model that matches       | `CORDICAddSub` (or `CORDICAddSub<Lns, K>` for a truncated iteration budget) |
| wants software-fast AND faithful rounding (no transcendentals)       | `ArnoldCotransformationAddSub` (~1 ULP, small LUT footprint) |
| is designing an FPGA / ASIC CORDIC unit and wants its errors bit for bit | `CORDICDatapathAddSub` (iteration count and guard width as knobs) |

For most general-purpose software the choice is between
`DirectEvaluation` (cleanest, exact) and `Polynomial` (saves one
transcendental, maintains good accuracy with no SRAM cost). Hardware
co-processors and edge AI accelerators are where `Lookup` and
`ArnoldBailey` shine.

## Worked example: opting into Lookup for production code

```cpp
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/lns_addsub_algorithms.hpp>

namespace sw::universal {
    template<>
    struct lns_addsub_traits<lns<16, 8, std::uint16_t>> {
        // Larger IndexBits -> larger table, tighter error bound
        using type = LookupAddSub<lns<16, 8, std::uint16_t>, 12>;
    };
}

int main() {
    using LNS16 = sw::universal::lns<16, 8, std::uint16_t>;
    LNS16 a(2.5);
    LNS16 b(0.7);
    LNS16 c = a + b;       // routes through LookupAddSub<LNS16, 12>
    LNS16 d = a * b;       // multiplication is unaffected (still integer add on exponent)
    return 0;
}
```

The trait specialization must be in the `sw::universal` namespace and
must be visible at the point of every translation unit that uses the
specialized type -- the easiest way is to put it in a header that gets
included alongside the rest of your `lns<>` use.

## Benchmarks

The benchmark target
`benchmark/performance/arithmetic/lns/log_add_algorithms.cpp` measures
all eight algorithms across representative `(nbits, rbits)`
configurations. It produces a Markdown table of throughput (ops / sec)
and per-algorithm value-domain error vs the `DirectEvaluation` oracle.

Sample output for `lns<24, 16, uint32_t>` on a desktop x86_64 build:

| Algorithm           | Throughput      | Max rel error vs Direct |
|---------------------|-----------------|--------------------------|
| DoubleTrip          | ~1.6 M ops/sec  | 0                        |
| DirectEvaluation    | ~1.1 M ops/sec  | 0 (oracle)               |
| Lookup              | ~1.1 M ops/sec  | ~9.5e-5                  |
| Polynomial          | ~1.1 M ops/sec  | ~1.1e-5                  |
| ArnoldBailey        | ~1.1 M ops/sec  | ~5e-2                    |
| CORDIC              | ~0.9 M ops/sec  | ~3.2e-5                  |
| ArnoldCotr          | ~1.1 M ops/sec  | ~1.1e-5 (faithful, 1 ULP)|
| CORDICDatapath      | ~0.16 M ops/sec | 0                        |

The throughput differential between algorithms is small at the `lns<>`
class API level because the encode / decode dominates the per-call
cost. The differential at the raw `sb_add(d)` level on `double`
operands -- the regime hardware co-processors actually pipeline -- is
much larger but is not exposed by this benchmark.

The accuracy column matches the analytical bounds for each algorithm.

## See also

- [Introduction](../lns/) for the LNS history and the Gauss log-add
  derivation
- [Implementation](../lns-implementation/) for the `lns<>` template,
  fixpnt exponent storage, and the cheap operations (mul, div, pow)
- [Algorithm Tolerance](../lns-tolerance-traits/) for how the
  regression suite handles the inherent approximation in
  Lookup / Polynomial / ArnoldBailey
- The design doc at `docs/design/lns-add-sub.md` for the deeper
  algorithm derivations and references
- The header `include/sw/universal/number/lns/lns_addsub_algorithms.hpp`
  for the policy class definitions
- The cross-validation regression at
  `static/logarithmic/lns/arithmetic/log_add_algorithms.cpp` for
  algorithm-level tests
