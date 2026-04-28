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
This page covers that framework: the customization point, the five
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

Universal ships five algorithms covering the full SRAM-vs-accuracy
trade-off space. All are header-only and constexpr-friendly.

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

## Decision tree

| If your target ...                                                   | use                                  |
|----------------------------------------------------------------------|--------------------------------------|
| just needs the `lns<>` API to work, no perf concern                  | `DoubleTripAddSub` (default)         |
| needs full precision, can afford 2 transcendentals per op            | `DirectEvaluationAddSub`             |
| has SRAM for a 1-8 KB table, wants ~5e-5 rel error                   | `LookupAddSub` (default `IndexBits`) |
| has SRAM for a >100 KB table, wants 1-2 ULP accuracy                 | `LookupAddSub<Lns, 16+>`             |
| is SRAM-constrained, wants ~5e-6 abs error, ok with 1 transcendental | `PolynomialAddSub`                   |
| is energy-constrained, wants no transcendentals, ok with ~2.5% rel error | `ArnoldBaileyAddSub`             |

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
all five algorithms across representative `(nbits, rbits)`
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
