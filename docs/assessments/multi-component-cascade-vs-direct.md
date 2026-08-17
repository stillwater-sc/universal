# Multi-component Systems

**A design assessment of the cascade and direct implementations: what they cost against each other,
what the August 2026 optimizations bought, and where a second pass should start.**

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

### universal#1326 - one more quotient digit, and the error term the scalar multiply dropped

Division computes its quotient one digit at a time. **Every cascade width computed exactly N digits
for an N-component result; the correct algorithm needs N+1.** The extra digit carries no weight of
its own - it is discarded - but the renormalization needs it to round the last component. Classic
`dd` computes three digits for two components, classic `qd` five for four.

Fixing the digit count got `qd_cascade` to 0.88 ulps, not to `qd`'s 0.14. The rest was a second,
independent cause: `multiply_cascade_by_double`, added by #1322, computed the last partial product
with a **plain multiply**, discarding an error term that lands exactly at the last component's ulp.
That schedule is a transliteration of qd's own `operator*=(double)`, which has the same limitation -
and which is why classic `qd` routes `a * double` through its full 4x4 multiply instead. Division
inherits it, because every residual step is exactly that operation.

| | before | after | direct |
|---|---|---|---|
| divide, N=2 | 2.21 ulps | **0.47** | `dd` 0.47 |
| divide, N=3 | 3.16 ulps | **0.33** | - |
| divide, N=4 | 1.84 ulps | **0.14** | `qd` 0.14 |
| cascade x double, N=4 | 0.79 ulps | **0.11** | `qd` 0.11 |
| `sqrt` residual, N=4 | 8.57 ulps | **0.42** | `qd` 1.05 |

Two of the three divisions are now bit-identical to their direct counterparts - over 4096
full-width operand pairs `dd_cascade` reproduces `dd` exactly on add, multiply and divide, and
`qd_cascade` reproduces `qd` exactly on multiply and divide (`benchmark_hp_equivalence`). Where
`qd_cascade`'s addition differs from `qd`'s, in 189 of those 4096 pairs, it is because `qd_cascade`
is *exact* and `qd` is not, which is why the difference is bounded by half an ulp.

`sqrt` improved without being touched - its Newton iteration divides, so it inherited the fix - and
is now the most accurate square root in the library, better than `qd`'s.

The cost is real and was anticipated: the cascade divisions were *faster* than the direct ones
because they did less work. Doing the correct amount lands them at parity - `dd_cascade` divide goes
78 -> 219 nsec/op against `dd`'s 217, `qd_cascade` 443 -> 640 against `qd`'s 588 - and `sqrt`, which
divides, roughly doubles. A cheaper option exists at N=2: dropping the fused residual (classic `dd`
uses `fma`) gives 0.67 ulps instead of 0.47 at 163 nsec/op instead of 219. Matching the reference
exactly was chosen over keeping a speed margin, on the grounds that a type meant to replace `dd`
should not be measurably worse than it.

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
| divide | 0.47 | 0.47 | 0.33 | 0.14 | 0.14 |
| `sqrt` | 10.8 (*) | 1.4 (*) | 1.1 (*) | 0.37 | **0.14** |
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
| add | 23.9 | 16.0 | **0.67** | 35.7 | 62.5 | 70.2 | 1.12 |
| subtract | 23.9 | 15.3 | **0.64** | 32.5 | 61.5 | 66.1 | 1.07 |
| multiply | 22.3 | 24.6 | 1.10 | 63.4 | 73.1 | 89.5 | 1.22 |
| divide | 216.7 | 219.2 | 1.01 | 319.4 | 587.5 | 639.8 | 1.09 |
| `sqrt` | 42.1 | 580.7 | **13.80** | 957.5 | 1089.1 | 2981.7 | **2.74** |
| `exp` | 918 | 1164 | 1.27 | 3276 | 4009 | 6743 | 1.68 |
| `log` | 1959 | 2494 | 1.27 | 7011 | 12669 | 22211 | 1.75 |
| `sin` | 953 | 1678 | 1.76 | 3196 | 3422 | 8331 | 2.43 |

The generic framework now costs 1.1x-1.8x on most operations, wins on double-double add/subtract,
sits at parity on division, and loses badly on exactly one: `sqrt`. That last number moved the wrong
way in #1326 and is no accident - `sqrt` iterates on division, so it paid the correctness fix twice
over. It is now the clear next target.

## Remaining discrepancies, in the order a second pass should take them

Item 1 of the original list - division accuracy - was closed by universal#1326. What follows is the
list as it stands after that, reordered by what the measurements now say.

### 1. `sqrt` costs a division per iteration (highest value)

The worst ratio in the suite and now the only one moving the wrong way: `dd_cascade` is **13.8x**
`dd` and `qd_cascade` **2.74x** `qd`, up from 7.0x and 2.0x, because universal#1326 made division
correct and `sqrt` iterates on division.

The cause is a design choice: the cascade iterates `x = (x + a/x)/2`, one **division** per step,
while classic `qd` iterates on the reciprocal square root using **multiplication only** and
multiplies through once at the end. With division now at 219-640 nsec/op and multiplication at
25-90, the reciprocal formulation is clearly the right one.

The complication is that it is no longer a free win. The cascade `sqrt` is currently the *most
accurate* in the library - 0.42 ulps residual against `qd`'s 1.05, and 0.14 ulps against an oracle
where `qd` measures 0.37 - precisely because it divides. A reciprocal iteration would trade some of
that back for speed. Measure both before choosing; the accuracy headroom above `qd` is real budget
to spend, but it should be spent deliberately.

### 2. The 46% that compression costs N=4 addition

Unchanged from the original list. The compression added by universal#1317 is applied *after* the
fact, to repair an expansion built without the non-overlapping invariant. The direct implementation
never needs it, because `accurate_addition` maintains a two-term running carry and emits components
already settled.

Porting that formulation to `add_cascades<4>` would plausibly recover most of the 46% *and* keep the
exactness, since it is the algorithm the compression is emulating. Same move as #1322 and #1324 made
for multiplication.

### 3. The generic templates are still the old design

`add_cascades<N>` and `multiply_cascades<N>` - reached for any N outside {2, 3, 4} - still use
`std::sort`/`std::vector` and the uncompressed, sorted-expansion formulation that #1317 and #1322
replaced, and they know nothing of the quotient-digit fix. Nothing instantiates them today, so this
is not a live defect, but the first person to instantiate `floatcascade<5>` inherits every bug this
effort fixed, silently.

Either specialize them the same way, or make the primary template `static_assert` that N is one of
the supported widths.

### 4. Shared weaknesses that cap both families

These are now the largest accuracy gaps left anywhere in the multi-component types, and none of them
are cascade-specific:

- **Square is 4.0-4.2 ulps in both `dd` and `qd`** (and identically in their cascade counterparts),
  where multiplication is 0.11-0.46. `sqr(a)` is not simply `a*a` here, and whatever it does is
  worse. `exp` squares 16 times.
- **`log` is 15.3 ulps in both `qd` and `qd_cascade`**, an order of magnitude worse than `exp` at
  0.26.
- **`sin`/`cos` never reach format precision above `dd`** (universal#1318). `qd` holds ~41 digits of
  63, the two wider cascades ~32 - their extra components carry nothing through trigonometric
  evaluation.
- **`x / inf` returns NaN** in every multi-component type, direct and cascade alike, where IEEE says
  zero (universal#1327). A shared conversion/guard issue, not an algorithmic one.
- **Decimal string parsing costs 45-460 usec per value** (universal#1319) across every
  multi-component type, which puts it in shared conversion machinery.

Fixing these benefits both families at once. The first two are now the ceiling on `exp`, and
universal#1318 means the extra components of `td_cascade` and `qd_cascade` are wasted in
trigonometric code.

## Suggested sequencing

1. **Reformulate `sqrt` on the reciprocal iteration** (item 1), measuring the accuracy cost rather
   than assuming it. Removes the worst ratio in the suite.
2. **Port `accurate_addition` to `add_cascades<4>`** (item 2). Highest confidence: the algorithm is
   known-good, sitting next door, and the pattern is proven three times over.
3. **Close the generic templates** (item 3). Cheap, prevents silent regression.
4. **Then the shared items** (item 4), which are number-system work rather than framework work, and
   which now hold the largest remaining errors in either family.

The move that has paid off in #1317, #1322, #1324 and #1326 is the same one every time: where the
cascade improvises, adopt the direct family's proven schedule and express it with the framework's
hardened primitives. Item 1 is the first entry on this list where that rule does **not** simply
apply - the cascade's `sqrt` is already more accurate than the direct one, and the question is what
to trade, not what to copy.

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
