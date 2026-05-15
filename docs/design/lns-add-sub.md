# lns native add/sub: configurable algorithm framework

This document describes the design of the configurable add/sub algorithm
framework for the logarithmic number system (`lns`) in the Universal Numbers
Library. The framework lets users choose, per `lns` instantiation, how the
single hard operation in the log domain -- addition -- is computed, and ships
with six algorithms covering the full SRAM-vs-accuracy trade-off space
plus a hardware-codesign tier.

The framework was developed in five phases tracked under Epic #777
(Phase E added under issue #783):

| Phase | PR     | Deliverable                                                      |
|-------|--------|------------------------------------------------------------------|
| A     | #784   | scaffolding, `DoubleTripAddSub` baseline, `DirectEvaluationAddSub` |
| B     | #785   | `LookupAddSub` (Mitchell-style precomputed table)                  |
| C     | #786   | `PolynomialAddSub`, `ArnoldBaileyAddSub`                           |
| D     | #787   | benchmark suite + design doc                                       |
| E     | #783   | `CORDICAddSub` (hardware-codesign tier) + precision/accuracy assessment |

---

## Why log-domain addition is the hard case

In the log domain, multiplication and division are integer operations on the
exponent: `a * b -> La + Lb`, `a / b -> La - Lb`. Addition is the difficult
one because there is no closed-form `Lc = f(La, Lb)` that just uses log-domain
arithmetic -- it requires going through the linear domain via a transcendental.

The Gauss log-add formulation makes the structure explicit. Given two same-sign
values `a, b > 0` with logs `La = log2(a)`, `Lb = log2(b)`, and arranging
`La >= Lb`:

```text
log2(a + b) = log2(2^La + 2^Lb)
            = log2(2^La * (1 + 2^(Lb - La)))
            = La + log2(1 + 2^d),  d = Lb - La <= 0
            = La + sb_add(d)
```

The correction term `sb_add(d) = log2(1 + 2^d)` is a one-dimensional, smooth,
monotonic function of `d`:

- `sb_add(0) = log2(2) = 1`
- as `d -> -inf`, `sb_add(d) -> 0` (the smaller operand vanishes in the log
  domain when it is many orders smaller than the larger)
- always positive over `d <= 0`

For mixed-sign operands the analogous correction is
`sb_sub(d) = log2(1 - 2^d)`, with a fundamental difficulty: `sb_sub` has
unbounded slope as `d -> 0` (catastrophic cancellation), and goes to `-inf`
exactly at `d = 0` (which is the `a + (-a) = 0` case).

So the entire game in lns add/sub is computing `sb_add(d)` and `sb_sub(d)` --
the rest is bookkeeping (sign routing, special values, encode/decode). The
six algorithms in this framework differ only in *how* they compute `sb_add`
and `sb_sub`.

> **Naming convention.** Universal's policy class API uses the prefix `sb_`
> (read as "sum-base") for these correction functions. The LNS literature
> uses different conventions for the same mathematics: Coleman et al.
> (European Logarithmic Microprocessor) write `phi(z) = log_b(1 + b^z)` and
> `psi(z) = log_b(1 - b^z)`; Arnold et al. write `F+(d)` and `F-(d)`. The
> `sb_` prefix was chosen as a short, ASCII-friendly tag for the C++ API
> at the start of the configurable add/sub framework (PR #784) and is
> Universal-internal -- it's not standard literature.

---

## The customization point

`lns_addsub_traits<Lns>` is the per-instantiation customization point. The
default selects `DoubleTripAddSub` (the historical placeholder) for backward
compatibility, but users override per type:

```cpp
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/lns_addsub_algorithms.hpp>

namespace sw::universal {
    template<>
    struct lns_addsub_traits<lns<16, 8, std::uint16_t>> {
        using type = LookupAddSub<lns<16, 8, std::uint16_t>, 12>;  // 4096-entry table
    };
}
```

`lns::operator+=` and `lns::operator-=` route through
`lns_addsub_algorithm_t<lns>::add_assign(*this, rhs)`, so a single
specialization changes the behavior of every `+`, `-`, `+=`, and `-=` for that
specific `lns` instantiation without breaking other instantiations.

A traits class was chosen over a fifth template parameter because the lns
template signature is `lns<_nbits, _rbits, bt, auto... xtra>` and the
`auto... xtra` parameter pack must remain last -- adding a fifth parameter
would either break every existing `lns<16, 8, std::uint16_t, Saturating>`
site or hide behind defaults that defeat the customization purpose. The
traits-class pattern (the same as `std::char_traits` for `std::basic_string`)
is zero-breaking and self-documenting.

### The shared dispatcher

All algorithms share the same special-value routing and sign dispatch through
`detail::gauss_log_add<Policy>`. A new policy only needs to provide:

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

The dispatcher handles NaN propagation, +/-0 short-circuits, +/-inf delegation
to native double arithmetic, and the same-sign / mixed-sign / cancellation
routing.

---

## Shipped algorithms

### 1. DoubleTripAddSub (default)

Casts both operands to `double`, computes the result with native `+`/`-`, casts
back. Pays a full encode/decode for every operation. Preserves the long-standing
behavior from before the `constexpr_math` facility (Epic #763) existed; the
default trait selects this so existing `lns` users see no change.

Useful as a correctness oracle for cross-validating other algorithms.

### 2. DirectEvaluationAddSub

Implements the Gauss log-add formulation directly via
`sw::math::constexpr_math::log2` and `cm::exp2`. Two transcendentals per add,
no approximation; accuracy is dominated by the `cm::log2`/`cm::exp2`
implementations (a few ulp at double precision). The recommended default for
high-precision software targets and a correctness oracle for the faster
algorithms.

### 3. LookupAddSub

The original Mitchell 1962 LNS algorithm in modern form: precompute `sb_add(d)`
on a finite d-grid at compile time, look up + linearly interpolate at runtime.

```cpp
template<typename Lns,
         unsigned IndexBits = ((Lns::rbits + 2 < 10) ? Lns::rbits + 2 : 10)>
struct LookupAddSub { ... };
```

`IndexBits` (log2 of table entries) defaults to `min(rbits + 2, 10)` to keep
the default table modest (1024 entries = 8 KB doubles); users override per
instantiation to trade accuracy for SRAM. d-range is `[-rbits-2, 0]` because
past that the correction is below lns ULP and rounds to 0 anyway.

`sb_sub` falls back to direct `cm::log2`/`cm::exp2` in the lowest-cell
cancellation regime (`|d| < step`) where linear interpolation has unbounded
error.

### 4. PolynomialAddSub

Closed-form, no table. Uses the classical log-domain identity
`log2((1+x)/(1-x)) = (2/ln 2) * (x + x^3/3 + x^5/5 + x^7/7 + ...)` with the
substitution `x = u/(2+u)` where `u = 2^d`, mapping `u in (0, 1]` to
`x in (0, 1/3]`. Degree-7 truncation gives ~5.6e-6 absolute error worst-case
(the next omitted term, `x^9/9` at `x = 1/3`, is 5.6e-6).

For `sb_sub` the analogous substitution is `x = u/(2-u)`, with the same
catastrophic-cancellation fallback as Lookup.

Cost: one transcendental at runtime (the `2^d` to recover `u` from `d`), one
division. No table.

### 5. ArnoldBaileyAddSub

Cheapest of the family: piecewise-linear approximation matching the function
value at integer-d knots `(d = 0, -1, -2, -3, -4, -5)`. Knots computed at
compile time via `cm::log2` so we don't bake in approximate constants. Each
piece is a single `a + b*d` evaluation -- two multiplies, one add, no
transcendentals at runtime, no division.

Worst-case error ~2.5% relative near `d = 0` (where the curvature of
`log2(1 + 2^d)` is highest), dropping rapidly toward the tail. Same
cancellation-regime fallback as Lookup and Polynomial.

References: Mitchell 1962 (the foundational LNS-add paper); Arnold,
Bailey, Cowles, Cuthbertson 1990 (Journal of VLSI Signal Processing);
Arnold and Walter 2000 (Symposium on Computer Arithmetic). The
implementation is "in the style of" that family -- the knot count and
interval breakdown vary by source, and the version here is a small
hand-readable subset.

### 6. CORDICAddSub (hardware-codesign tier)

Classical hyperbolic CORDIC: bit-by-bit refinement using only adds, shifts,
and a small table of `atanh(2^-k)` constants. Two passes:

- **Rotation mode** computes `v = 2^d` via `exp(d * ln 2)`, with range
  reduction `d = q + f` (integer + fractional) so only the fractional part
  feeds CORDIC. The `2^q` factor is a binary shift.
- **Vectoring mode** then computes `log2(1 +/- v)` by driving `y` to zero
  in the iteration `(x_{i+1}, y_{i+1}, z_{i+1}) = (x_i + sigma * y_i * 2^-i,
  y_i + sigma * x_i * 2^-i, z_i - sigma * atanh(2^-i))` starting from
  `(w+1, w-1, 0)`. The accumulator `z` converges to `(1/2) * ln(w)`.

```cpp
template<typename Lns, unsigned MaxIterations = Lns::rbits>
struct CORDICAddSub { ... };
```

`MaxIterations` is exposed as a non-type template parameter so a hardware
team can sweep truncated iteration budgets independently of `rbits`. The
default `MaxIterations = rbits` preserves the "within rbits of accuracy"
acceptance criterion.

**Hyperbolic-CORDIC convergence requires repeating iterations 4, 13, 40, ...**
(`r_{k+1} = 3*r_k + 1, r_0 = 4`). Without those repeats, the per-iteration
angle radius drops below the residual remaining-z range past index 4 and
the algorithm fails to converge. For software `MaxIterations <= 60` only
iterations 4, 13, and 40 fall in range; the implementation handles all
three.

**Why CORDIC at all in software?** It is the wrong choice on a CPU: one
dependent iteration per rbit (no SIMD parallelism, no batched throughput),
slower than `Polynomial` or `Lookup` per operation, no win in any cost
column. The reason it ships is exactly the reason it is named the
"hardware-codesign tier": it maps directly to the area / latency model of
an FPGA or ASIC LNS pipeline, where each CORDIC iteration is one cycle of
a fully-bypassed datapath with no transcendental hardware. The software
implementation lets a hardware-codesign consumer characterize the
iteration-budget vs accuracy curve in advance of committing to silicon
(see the per-iteration assessment artifact at
`docs/design/cordic-precision-assessment.md`).

**Cancellation regime.** `sb_sub(d) = log2(1 - 2^d)` has unbounded slope
as `d -> 0`. At any `MaxIterations`, CORDIC's residual exp error exceeds
the true magnitude of `u - 1` once `|d|` is small enough, and the chain
produces invalid arguments to `log2`. The software implementation uses
the same direct-evaluation fallback (`cm::log2` / `cm::exp2`) as
`PolynomialAddSub`, `LookupAddSub`, and `ArnoldBaileyAddSub` in the
`d in (-1, 0)` band. Hardware retargets would substitute a
co-transformation (a la Arnold/Vouzis/Coleman; see #829) or a small
lookup here -- not a transcendental.

**Error envelope.** For the pure-CORDIC paths (sb_add and the `d <= -1`
branch of sb_sub), per-iteration error is approximately `4 * 2^-k`
log-domain at iteration count `k`. The assessment artifact tabulates the
empirical envelope per (rbits, k) configuration, with histograms and
worst-case witness inputs.

---

## Decision tree

| If your target ...                                            | use                       |
|---------------------------------------------------------------|---------------------------|
| just needs the lns API to work, no perf concern               | `DoubleTripAddSub` (default) |
| needs full precision, can afford 2 transcendentals per op     | `DirectEvaluationAddSub`  |
| has SRAM for a 1-8 KB table, wants ~5e-5 rel error            | `LookupAddSub` (default IndexBits) |
| has SRAM for a > 100 KB table, wants 1-2 ULP accuracy         | `LookupAddSub<Lns, 16+>`  |
| is SRAM-constrained, wants ~5e-6 abs error, ok with 1 transcendental | `PolynomialAddSub` |
| is energy-constrained, wants no transcendentals, ok with ~2.5% rel error | `ArnoldBaileyAddSub` |
| is targeting FPGA / ASIC and wants the cost model that matches | `CORDICAddSub` (or `CORDICAddSub<Lns, K>` for a truncated iteration budget) |

For most general-purpose software code the choice is between
`DirectEvaluation` (cleanest, exact) and `Polynomial` (saves one
transcendental, maintains good accuracy with no SRAM cost). Hardware
co-processors and edge accelerators are where `Lookup` and `ArnoldBailey`
shine.

---

## Measured throughput and accuracy

The benchmark at `benchmark/performance/arithmetic/lns/log_add_algorithms.cpp`
measures all six algorithms across representative configs. Sample output
(host-dependent; numbers below are illustrative from a single run on a
desktop x86_64 build):

### lns<16, 8, uint16_t>

| Algorithm           | Throughput      | Max rel error vs Direct |
|---------------------|-----------------|--------------------------|
| DoubleTrip          | ~1.6 M ops/sec  | 0                        |
| DirectEvaluation    | ~1.1 M ops/sec  | 0 (oracle)               |
| Lookup              | ~1.1 M ops/sec  | 0                        |
| Polynomial          | ~1.1 M ops/sec  | 0                        |
| ArnoldBailey        | ~1.1 M ops/sec  | ~5e-2                    |

### lns<24, 16, uint32_t>

| Algorithm           | Throughput      | Max rel error vs Direct |
|---------------------|-----------------|--------------------------|
| DoubleTrip          | ~1.6 M ops/sec  | 0                        |
| DirectEvaluation    | ~1.1 M ops/sec  | 0 (oracle)               |
| Lookup              | ~1.1 M ops/sec  | ~9.5e-5                  |
| Polynomial          | ~1.1 M ops/sec  | ~1.1e-5                  |
| ArnoldBailey        | ~1.1 M ops/sec  | ~5e-2                    |

### Reading the data

- **Throughput** is dominated by the `lns` encode/decode path, not the
  `sb_add` cost. At the lns class API level, all algorithms run in the same
  ballpark (~1 M ops/sec on this host). The throughput differential between
  algorithms is what matters when sb_add is invoked in a tight loop without
  going through a full encode/decode each time -- i.e., a hardware
  co-processor pipelined at the log-domain level. To measure that
  differential meaningfully you would need to benchmark `sb_add(d)` directly
  on `double` operands rather than through `add_assign` on `Lns` operands.
- **Accuracy** matches the analytical bounds:
  - LookupAddSub at default `IndexBits = min(rbits + 2, 10)` gives ~5e-5
    rel error on rbits=16, dropping to 0 (encoding-bound) on smaller rbits
    where the table is dense enough.
  - PolynomialAddSub gives ~1e-5 rel error consistently, matching the
    degree-7 truncation envelope amplified through `exp2(Lresult)`.
  - ArnoldBaileyAddSub caps at ~5e-2 rel error across all configs, matching
    the piecewise-linear secant bound.

---

## References

- Mitchell, J. N. (1962). "Computer multiplication and division using binary
  logarithms." *IRE Transactions on Electronic Computers*, EC-11(4), 512-517.
- Arnold, M. G., Bailey, T. A., Cowles, J. R., Cuthbertson, K. (1990). "An
  Improved Logarithmic Number System Architecture." *Journal of VLSI Signal
  Processing*, 1(1), 13-20.
- Arnold, M. G. and Walter, J. (2000). "Unrestricted faithful rounding is
  good enough for some LNS applications." *Proc. 15th IEEE Symposium on
  Computer Arithmetic*, 237-246.
- Coleman, J. N., Chester, E. I., Softley, C. I., Kadlec, J. (2000).
  "Arithmetic on the European Logarithmic Microprocessor." *IEEE Transactions
  on Computers*, 49(7), 702-715.

---

## Files

- `include/sw/universal/number/lns/lns_addsub_algorithms.hpp` -- all six
  algorithm policies + traits class + `detail::gauss_log_add` dispatcher
- `include/sw/universal/number/lns/lns_impl.hpp` -- `operator+=` / `operator-=`
  delegate to `lns_addsub_algorithm_t<lns>::add_assign` / `sub_assign`
- `static/logarithmic/lns/arithmetic/log_add_algorithms.cpp` -- cross-validation
  regression suite (algorithm vs algorithm, corner cases, Tier 1 cancellation)
- `benchmark/performance/arithmetic/lns/log_add_algorithms.cpp` -- per-algorithm
  throughput + accuracy benchmark
- `benchmark/accuracy/lns/cordic_precision_assessment.cpp` -- per-iteration
  convergence study, ULP histograms, worst-case witness table for
  `CORDICAddSub` across the rbits sweep
- `docs/design/cordic-precision-assessment.md` -- generated artifact from
  the precision/accuracy assessment tool
