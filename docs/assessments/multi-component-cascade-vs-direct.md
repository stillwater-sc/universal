# Multi-component arithmetic: cascade vs direct

Universal carries two implementations of multi-component floating-point. The **direct** types, `dd`
and `qd`, are hand-specialized transliterations of the Hida/Li/Bailey QD library: one algorithm per
operation per width, written out in full. The **cascade** types, `dd_cascade`, `td_cascade` and
`qd_cascade`, are built on `floatcascade<N>`, a generic N-component expansion framework with
volatile-hardened error-free transformations, and the stated intent is that they replace the direct
ones.

This document records what the two families actually cost against each other, what was fixed in
August 2026 (universal#1317, #1322, #1324), and what is left for a second optimization pass. It is
written to be the starting point for that pass.

## Method, and why it matters here

Every accuracy number below comes from an oracle that shares no code with the implementations under
test:

- an external reference (Python `decimal` at 140-200 digits) for transcendentals and for scoring
  worst/median error over random operand sweeps;
- the in-repo exact dyadic oracle (`verification/dyadic_exact.hpp`, backed by `einteger`) for `+`,
  `-` and `*`, where the true result is a dyadic rational and the comparison is exact.

This is not a formality. The defect that started this effort (#1317) survived every existing test
because those tests are **self-consistent**: they check that a result is normalized, that it
round-trips, and that the two implementations agree with each other. A result that is well-formed
and wrong passes all three. Two further traps are worth stating outright, because both hid real
defects:

1. **Operands built from a single `double` prove nothing.** Their sum and product are exactly
   representable, so every implementation returns them bit-for-bit. Defects only appear with
   *full-width* operands - each component ~2^-53 below the previous - which is what Taylor series,
   Newton iterations and Horner evaluations actually produce internally.
2. **Score each width against its own ulp.** 2^-106 for `dd`, 2^-159 for `td`, 2^-212 for `qd`.
   Applying a `qd`-calibrated threshold to a triple-double once led this effort to "find" a defect
   in `td_cascade` that did not exist.

Self-consistency residuals (`sqrt(x)^2 - x`, `sin^2 + cos^2 - 1`) still have a place - they are
cheap and they *attribute* a disagreement to one side - but they bound error from below and cannot
certify correct rounding. They are used below only where labelled.

Measurements: i7-12700K pinned to one P-core, gcc 13.3.0 and clang 18.1.3, `-O3 -DNDEBUG`, via
`benchmark/performance/arithmetic/highprecision/`. Accuracy sweeps are 400 random operand pairs in
`[0.5, 2.0)` unless noted.

## Where the two designs differ by construction

| Operation | Direct (`dd`/`qd`) | Cascade (`floatcascade<N>`) |
|---|---|---|
| Addition | magnitude-ordered merge with a **two-term running carry** (`quick_three_accumulation`), emitting a component only once it is settled | merge, accumulate with a **single running sum**, collect the errors, then renormalize the collected expansion |
| Multiplication | fixed schedule ordered by `eps^(i+j)`, no sorting | *was* a sorted 18/32-term expansion; **now the same fixed schedule** at N=3 and N=4 |
| Division | long division with exact residuals | Newton refinement, residual via `add_cascades` + compress |
| `sqrt` | Newton on the **reciprocal** square root - multiplication only | Newton on `(x + a/x)/2` - **one division per iteration** |
| Special values | `renorm` bails out on infinity | explicit non-finite guard on the leading product |
| Widths | 2 and 4, written out | 2, 3, 4 specialized; a generic template for any other N |

The first row is the structural heart of the matter. Keeping a two-term carry is what makes the
direct addition emit a *non-overlapping* expansion; collecting errors into a flat array does not,
and that is what #1317 had to repair after the fact.

## What was fixed

### universal#1317 - compress the expansion before renormalizing (N=4 addition)

`add_cascades` returned a sum whose *value* was exact and whose *components overlapped* -
consecutive terms within a factor of two of each other rather than 2^53. `compress_8to4` is QD's
`renorm`, built entirely from `fast_two_sum` chains, and `fast_two_sum(a, b)` is error-free only
when `|a| >= |b|`. Handed an overlapping expansion it ran out of places for the tail and returned a
fourth component of **exactly zero**, on roughly one addition in eight.

The fix is Shewchuk's COMPRESS (*Geometric Predicates*, Fig. 22), applied to the `floatcascade<4>`
overload. It is value-preserving: it changes only the shape of the expansion.

| | before | after | `qd` |
|---|---|---|---|
| addition, worst rel err | 1.7e-49 | **exact** | 6.2e-65 |
| `sqrt` | 1.3e-49 | 6.1e-64 | 5.5e-65 |
| `exp` | 4.8e-51 | 4.0e-65 | 4.0e-65 |
| `log` | 1.3e-50 | 2.3e-63 | 2.3e-63 |
| qd_cascade addition | 48.4 ns | 70.4 ns | 62.5 ns |

**Cost: +46% on quad-double addition**, and it reversed a benchmark conclusion - `qd_cascade`'s dot
product had measured 0.83x `qd` only because the addition was dropping a component.

Deliberately **not** applied at N=2 and N=3: both stay within 0.49 ulp of their own format with and
without it, measured over 5000 random full-width additions. Fewer merged terms compressed into
fewer output components leaves enough slack that the overlap never reaches a surviving component.

### universal#1322 - the qd_mul schedule at N=4

The old multiplication computed 16 partial products, **bubble sorted** the 32-term expansion (up to
496 compare-and-swap steps), and folded it into 4 components with a carry loop that discarded the
error term of every sub-ulp carry. Sorting throws away the structure the reference algorithm relies
on; the fold then throws away the terms that structure would have placed.

Replaced by a transliteration of `qd::accurate_multiplication`, which needs no sorting at all:
`a[i]*b[j]` contributes at order `eps^(i+j)`, so the products emerge in decreasing significance by
construction.

| | before | after | `qd` |
|---|---|---|---|
| multiply, worst rel err | 1.7e-63 | **1.6e-65** | 1.6e-65 |
| multiply | 581 ns | **82 ns** | 73 ns |
| `exp` | 26036 ns | **6642 ns** | 4165 ns |
| `log` | 75321 ns | **21814 ns** | 13183 ns |
| `inf * 3` | **NaN** | inf | inf |

Two pieces came with it and are worth remembering as a pattern:

- `multiply_cascade_by_double`, the equivalent of qd's separate `operator*=(double)`. Without it,
  fixing multiplication would have made **division 23% slower**: division's initial quotient
  estimate is a one-component cascade, and where the sorted multiply skipped zero products, a fixed
  schedule spends ten of its sixteen partial products multiplying by zero.
- A non-finite guard. `two_prod(inf, x)` is `fma(inf, x, -inf)` = NaN, which the downstream sums
  smear across the expansion. The old multiply turned `inf * 3` into a NaN - a real bug no test
  caught. A faithful qd_mul returns `inf` with `-nan` trailing components, which is what classic
  `qd` still does; the cascade now clears them.

### universal#1324 - the same schedule at N=3

| | before | after |
|---|---|---|
| multiply, worst | 5.53 ulps of 2^-159 | **0.23 ulps** |
| multiply, past 1 ulp | 102 / 400 | **0 / 400** |
| square, worst | 9.72 ulps | **0.22 ulps** |
| multiply | 214 ns | **63 ns** |
| `exp` | 13406 ns | **3280 ns** |
| `log` | 26224 ns | **7009 ns** |
| `sin` | 9763 ns | **2846 ns** |

Multiplication is now correctly rounded at this width, and `td_cascade` costs roughly what carrying
a third component should rather than 9x-15x `dd`.

## Where the two families stand today

### Accuracy, worst case in ulps of each format's own significand

Lower is better; a correctly rounded operation is at or below 0.5. The `add`/`multiply`/`square`/
`divide` rows are 400 random **full-width** operand pairs against the exact oracle - every component
populated, the shape that arises inside iterative kernels. The `sqrt`/`exp`/`log` rows are 200
single-argument evaluations, where the operand shape is not the variable of interest.

| | `dd` | `dd_cascade` | `td_cascade` | `qd` | `qd_cascade` |
|---|---|---|---|---|---|
| add | 0.46 | 0.46 | 0.48 | 0.41 | **0.00** |
| multiply | 0.46 | 0.46 | 0.23 | 0.11 | 0.11 |
| square | 4.23 | 4.23 | 0.22 | 4.01 | 4.01 |
| divide | 0.47 | **2.21** | **3.16** | 0.14 | **1.84** |
| `sqrt` | 10.8 (*) | 2.9 (*) | 7.2 (*) | 0.37 | 4.03 |
| `exp` | - | - | - | 0.26 | 0.26 |
| `log` | - | - | - | 15.3 | 15.3 |

(*) self-consistency residual, a lower bound, not an oracle comparison.

Read across the rows: **add and multiply are settled** - the cascade matches or beats the direct
implementation at every width. **Divide is the outlier at all three widths**, 4x to 13x the direct
error. **Square and `log` are weak in both families equally**, which makes them upstream problems
rather than cascade problems.

### Performance, nsec/op (gcc 13.3, -O3, pinned)

| | `dd` | `dd_cascade` | ratio | `td_cascade` | `qd` | `qd_cascade` | ratio |
|---|---|---|---|---|---|---|---|
| add | 23.9 | 16.1 | **0.67** | 35.1 | 62.5 | 70.3 | 1.12 |
| subtract | 23.9 | 15.3 | **0.64** | 32.6 | 61.5 | 66.4 | 1.08 |
| multiply | 22.3 | 24.6 | 1.10 | 63.0 | 73.1 | 83.1 | 1.14 |
| divide | 216.9 | 78.2 | **0.36** | 209.2 | 587.8 | 442.6 | **0.75** |
| `sqrt` | 42.8 | 299.0 | **6.99** | 726.0 | 1123.4 | 2221.9 | **1.98** |
| `exp` | 918 | 1175 | 1.28 | 3280 | 4167 | 6761 | 1.62 |
| `log` | 1964 | 2508 | 1.28 | 7009 | 13187 | 22050 | 1.67 |
| `sin` | 953 | 1281 | 1.34 | 2846 | 3531 | 7106 | 2.01 |

The generic framework now costs 1.1x-1.7x on most operations, wins on division and on
double-double add/subtract, and loses badly on exactly one: `sqrt`.

## Remaining discrepancies, in the order a second pass should take them

### 1. Division accuracy - all three widths (highest value)

The only operation where the cascade is *consistently* behind the direct implementation, and the
gap is structural rather than incidental.

```text
                worst      >1 ulp     direct counterpart
dd_cascade      2.21 ulps   30/400    dd 0.47
td_cascade      3.16 ulps   24/400    (none)
qd_cascade      1.84 ulps    -        qd 0.14
```

The cascade divides by Newton refinement: estimate `q0 = a[0]/b[0]`, form the residual
`a - q0*b` through `add_cascades` + compress, then refine. Two suspects, in order: the residual is
formed through the compression path added by #1317 (which is exact but may not be the tightest
formulation available), and the refinement takes a fixed number of steps rather than iterating to
the format's precision. Classic `qd` uses long division with exact residuals instead.

Note that cascade division is simultaneously *faster* than direct (0.36x at N=2, 0.75x at N=4), so
there is budget to spend. This is the one place where a more accurate algorithm can plausibly be
adopted without going backwards on speed.

### 2. `sqrt` algorithm choice - all widths

`qd_cascade` is 2.0x `qd` and `dd_cascade` is 7.0x `dd`, the worst ratios in the suite, and
`qd_cascade`'s result is 4.0 ulps against `qd`'s 0.37.

The cause is a design choice, not an implementation flaw: the cascade iterates `x = (x + a/x)/2`,
one **division** per step, while classic `qd` iterates on the reciprocal square root using
**multiplication only**, then multiplies through once at the end. Now that cascade multiplication
is at parity (1.10x-1.14x) and division is the expensive operation (443 ns at N=4 against 83 ns for
a multiply), the reciprocal formulation is clearly the right one and was not when the cascade
`sqrt` was written.

Interesting counter-example worth preserving: `dd_cascade`'s `sqrt` is *more accurate* than `dd`'s
(2.9 vs 10.8 ulps residual). The direct family could take something from the cascade here.

### 3. The cost of #1317's compression at N=4

Quad-double addition pays 46% for correctness, and that is the single largest performance
regression this effort introduced. The compression is applied *after* the fact, to repair an
expansion that was built without the non-overlapping invariant. The direct implementation never
needs it because `accurate_addition` maintains a two-term running carry and emits components
already settled.

Porting that formulation to `add_cascades<4>` would plausibly recover most of the 46% *and* keep
the exactness, since it is the algorithm the compression is emulating. This is the same move that
#1322 made for multiplication, and it is the obvious next application of the pattern.

### 4. The generic templates are still the old design (latent)

`add_cascades<N>` and `multiply_cascades<N>` - the templates reached for any N outside {2, 3, 4} -
still use `std::sort`/`std::vector` and the uncompressed, sorted-expansion formulation that #1317
and #1322 replaced. Nothing instantiates them today, so this is not a live defect, but it is a trap:
the first person to instantiate `floatcascade<5>` inherits every bug this effort fixed, silently.

Either specialize them the same way, or make the primary template `static_assert` that N is one of
the supported widths.

### 5. Shared weaknesses - not cascade problems, but they cap both families

- **Square is 4.0-4.2 ulps in both `dd` and `qd`** (and in their cascade counterparts, identically),
  where multiplication is 0.11-0.46. `sqr(a)` is not simply `a*a` in these implementations, and
  whatever it does is worse. `exp` squares 16 times.
- **`log` is 15.3 ulps in both `qd` and `qd_cascade`**, an order of magnitude worse than `exp` at
  0.26.
- **`sin`/`cos` never reach format precision above `dd`** - universal#1318. `qd` holds ~41 digits of
  63, `qd_cascade` and `td_cascade` ~32, i.e. the third and fourth components carry nothing through
  the trigonometric evaluation.
- **Decimal string parsing costs 45-460 usec per value** across every multi-component type -
  universal#1319 - which puts it in shared conversion machinery, not in any one number system.

Fixing these benefits both families at once, and #1318 in particular means the extra components of
`td_cascade` and `qd_cascade` are currently wasted in trigonometric code.

## Suggested sequencing

1. **Port `accurate_addition` to `add_cascades<4>`** (item 3). Highest confidence: the algorithm is
   known-good, sitting next door, and the pattern is proven twice over. Recovers most of a 46%
   regression.
2. **Reformulate `sqrt` on the reciprocal iteration** (item 2). Removes the worst performance ratio
   in the suite and should improve accuracy at the same time.
3. **Attack division accuracy** (item 1). Highest value, least certain shape; there is speed budget
   to trade.
4. **Close the generic templates** (item 4). Cheap, prevents silent regression.
5. **Then the shared items** (item 5), which are number-system work rather than framework work.

Items 1-3 are all the same move: where the cascade improvises, adopt the direct family's proven
schedule and express it with the framework's hardened primitives. That move has now paid off three
times.

## Reproducing any of this

```bash
cmake -DUNIVERSAL_BUILD_BENCHMARK_PERFORMANCE=ON -DUNIVERSAL_BUILD_NUMBER_STATICS=ON ..
make -j4 benchmark_hp_scalar benchmark_hp_mathlib benchmark_hp_kernels benchmark_hp_equivalence
taskset -c 0 ./benchmark/performance/arithmetic/benchmark_hp_scalar 0.25
```

- `benchmark/performance/arithmetic/highprecision/` - the performance and equivalence suite, with
  its own README carrying the full recorded tables and the gcc/clang/AVX2 sensitivity analysis.
- `static/highprecision/{qd,td}_cascade/arithmetic/addition_oracle.cpp` - exact dyadic validation of
  addition and multiplication. These fail loudly on the pre-fix implementations (825/4096 and
  235/4096 at `REGRESSION_LEVEL_4`), which is the property that makes them worth keeping.
- `include/sw/universal/number/qd_cascade/compress_8_4_bug.md` - the mechanism-level write-up of
  #1317 and #1322, including the expansion shapes that break `fast_two_sum`.
