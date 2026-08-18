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

### universal#1331 - three square roots, and a default that picks accuracy

All three cascade types iterated `x' = (x + a/x)/2`, one **division** per step. That was reasonable
when written, because cascade division was the cheap operation - it computed one quotient digit too
few. #1326 made division correct and therefore expensive, and `sqrt` paid the fix once per
iteration: 13.8x `dd` at N=2, 2.7x `qd` at N=4.

The obvious move was to switch to a multiplication-only formulation. The measurements said that is a
trade, not an upgrade, so all three formulations are now kept side by side behind
`UNIVERSAL_{DD,TD,QD}_CASCADE_SQRT_ALGORITHM`, **defaulting to the most accurate at each width**:

| width | formulation | residual | nsec/op | |
|---|---|---|---|---|
| `dd_cascade` | `NEWTON_DIVISION` | **1.4 ulps** | 607 | default |
| | `NEWTON_RECIPROCAL` | 1.9 | 358 | |
| | `KARP` | 5.5 | 78 | classic `dd`'s algorithm and accuracy |
| `td_cascade` | `NEWTON_RECIPROCAL` | **0.51 ulps** | 619 | default - faster *and* more accurate here |
| | `NEWTON_DIVISION` | 0.55 | 883 | |
| `qd_cascade` | `NEWTON_DIVISION` | **0.26 ulps** | 2843 | default |
| | `NEWTON_RECIPROCAL` | 0.66 | 1412 | classic `qd`'s algorithm |

Read the consequence honestly: **by default this changes `sqrt` performance very little.**
`dd_cascade` and `qd_cascade` keep the division iteration, so they stay at 14.4x `dd` and 2.8x `qd`,
and the speed is opt-in. What the change actually delivers is three things:

- `td_cascade` improved outright, 957 -> 691 nsec/op and 1.10 -> 0.72 ulps, because at that width
  two multiplication-only iterations beat two divisions on both axes.
- The faster formulations exist, are measured, and are one define away, each landing on its direct
  counterpart's accuracy - `KARP` *is* `dd`'s algorithm, `NEWTON_RECIPROCAL` at N=4 *is* `qd`'s.
- Every formulation now scales the argument into `[0.5, 2)` by an exact power of two first. Each
  squares a value of magnitude ~sqrt(a) or ~1/sqrt(a), which leaves the representable range at the
  extremes: `sqrt(maxpos)` used to return inf or NaN in `dd`, `dd_cascade` and `qd`. `dd` and `qd`
  received the same prologue in universal#1332, which turned out to fix more than the overflow --
  scored end to end over the exponent range, `dd`'s worst case went from 52 to 105 bits of its 106
  and `qd`'s from 60 to 214 of its 212, and `qd`'s 28 non-finite results went to none. `qd::sqrt(0)`
  also returned NaN, having never guarded zero at all.

A third reciprocal iteration was measured at N=2 and rejected: 3.3 ulps for 532 nsec/op, dominated
by the division iteration at 1.4 ulps for 607. What limits that path is the rounding inside the
iteration and the closing multiply, not the iteration count.

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
| `sqrt` | 10.8 (*) | 1.4 (*) | 0.72 (*) | 0.37 | 0.42 (*) |
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
| `sqrt` | 42.0 | 606.7 | 14.44 | 690.9 | 1088.7 | 3017.1 | 2.77 |
| `exp` | 917 | 1164 | 1.27 | 3290 | 3997 | 6691 | 1.67 |
| `log` | 1959 | 2493 | 1.27 | 6997 | 12610 | 22075 | 1.75 |
| `sin` | 916 | 1204 | 1.31 | 2987 | 3411 | 6359 | 1.86 |

The generic framework now costs a uniform 1.1x-1.9x, wins on double-double add and subtract, and
sits at parity on division. There is no outlier left: `sqrt` was 13.8x `dd` after #1326 and is 3.4x
after #1331. What remains is the framework's floor - a cascade operation carries its components
through a renormalization the hand-specialized code folds into the arithmetic.

## Remaining discrepancies, in the order a second pass should take them

Item 1 of the original list - division accuracy - was closed by universal#1326. What follows is the
list as it stands after that, reordered by what the measurements now say.

### 1. `sqrt` costs a division per iteration - CLOSED

Closed by universal#1331, though not the way the item assumed. Switching to a multiplication-only
iteration turned out to be a trade rather than an upgrade, so all three formulations are kept behind
`UNIVERSAL_{DD,TD,QD}_CASCADE_SQRT_ALGORITHM` and the default is the most accurate at each width.

That means `sqrt` is still 14.4x `dd` and 2.8x `qd` **by default** - the speed is opt-in, one define
away, each option landing on its direct counterpart's accuracy. `td_cascade` is the exception and
improved outright. `sqrt(maxpos)`, broken in `dd`, `dd_cascade` and `qd`, is fixed at all three
cascade widths by the argument scaling every formulation now shares. See the fix section above.

### 2. The 46% that compression costs N=4 addition - CLOSED, and the diagnosis was wrong

The hypothesis here was that compression is applied *after the fact* to repair an expansion built
without the non-overlapping invariant, and that a formulation which emits settled components would
not need it. Measurement (universal#1340) says otherwise, and it is worth recording which half was
wrong.

`add_cascades<4>` now merges its two operands instead of bubble-sorting them -- they arrive sorted,
so the 28 comparisons were pure waste -- and runs one `two_sum` chain over the merged sequence,
which is Shewchuk's `fast_expansion_sum`. Results are **bit-identical** to the previous formulation
over 40,000 random full-width additions and subtractions. Addition cost, i7-12700K -O3:

| | gcc 13.3 | clang 18.1 |
|---|---|---|
| before | 69.6 nsec/op | 80.1 nsec/op |
| after | **51.0** | 85.6 |

Operands that are not in magnitude order -- which nothing in the library produces, but which a
caller can build through the raw-limb constructor or the mutable component accessor -- are put in
order first, by a five-comparator network per operand behind a six-comparison test. Without that
guard the merge returns zero where the exact answer is 2^-100, and `qd_cascade` would have been the
one multi-component type that got such an operand wrong.

**But the compression pass had to stay.** It is not repairing sloppiness: the 2N -> N step needs a
*nonadjacent* expansion, not merely a non-overlapping one, and feeding it the raw chain output costs
a factor of three on the composite identities (`sqrt(x)^2 - x` goes 0.42 -> 1.61 ulps). Folding the
compression into that step rather than running it in the addition was measured too, and came out
both slower and less accurate. So the 46% was never the compression's to give back - the redundant
work was the sort.

The clang direction is the same compiler flip #1315 recorded for these benchmarks generally, and it
is tracked as universal#1342. Four formulations of the merge were measured there: branchy, branchless
and register-resident land within 2.6 nsec of each other (83.0, 83.6 and 85.6 nsec/op), and ordering
both operands unconditionally is the outlier at 94.3. So no shape of the merge recovers what clang
gets from the bubble sort it replaces.

### 3. The generic templates are still the old design - CLOSED

`add_cascades<N>` and `multiply_cascades<N>` - reached for any N outside {2, 3, 4} - still carry
`std::sort`/`std::vector` and the uncompressed, sorted-expansion formulation that #1317, #1322 and
#1324 replaced, and they know nothing of the quotient-digit fix. Nothing instantiated them, so this
was never a live defect, but the first person to write `floatcascade<5>` would have inherited every
bug this effort fixed, silently and with no indication anything was wrong.

Both templates now `static_assert` that N is 2, 3 or 4. Instantiating another width is a compile
error naming this document rather than an arithmetic result that is quietly 15 digits short.

Specializing them properly was considered and rejected: there is no generic form of the corrected
algorithms to fall back on. The multiplication schedule is derived from the eps^(i+j) ordering of the
partial products for one specific N; the division needs N+1 quotient digits closed by an N+1-term
renormalization; and whether the addition needs compression is a measured property, not a derived one
(N=2 and N=3 do not need it, N=4 does). A new width needs a new derivation, and the error message
says so.

### 4. Shared weaknesses that cap both families

These are now the largest accuracy gaps left anywhere in the multi-component types, and none of them
are cascade-specific:

- **Square is 4.0-4.2 ulps in both `dd` and `qd`** (and identically in their cascade counterparts),
  where multiplication is 0.11-0.46. `sqr(a)` is not simply `a*a` here, and whatever it does is
  worse. `exp` squares 16 times.
- **`log` is 15.3 ulps in both `qd` and `qd_cascade`**, an order of magnitude worse than `exp` at
  0.26.
- ~~**`sin`/`cos` never reach format precision above `dd`**~~ - fixed (universal#1318). Two
  independent causes, both inherited from the double-double code the files were copied from: the
  constant named `qd_eps` held the *double-double* unit roundoff (2^-104 rather than 2^-209), so the
  Taylor series stopped at 43 of the 63 digits `qd` carries; and the cascades reduced the argument
  modulo pi/16 with a four-entry double-double table, which a 15-entry inverse-factorial table
  cannot carry past double-double accuracy no matter how wide the type is. The same file's `atan2`
  took a single Newton step - right for a double-double, one third of what a quad-double needs - so
  `atan`, `asin` and `acos` were capped at 106 bits too. `sin^2 + cos^2 - 1` in ulps of each format:

  | type | before | after |
  |---|---|---|
  | `td_cascade` | 4.277e+15 | 1.141 |
  | `qd` | 1.494e+23 | 1.34 |
  | `qd_cascade` | 3.852e+31 | 0.3555 |

  All six functions now deliver the full significand of their format, scored against mpmath.
- **`x / inf` returns NaN** in every multi-component type, direct and cascade alike, where IEEE says
  zero (universal#1327). A shared conversion/guard issue, not an algorithmic one.
- ~~**Decimal string parsing costs 45-460 usec per value**~~ - fixed (universal#1319). Four
  algorithmic costs in shared conversion machinery, none of them inherent; parsing is now 12-58x
  faster across every multi-component type.

Fixing these benefits both families at once. The first two are now the ceiling on `exp`.

## Suggested sequencing

1. ~~Reformulate `sqrt`~~ - done (universal#1331); the accuracy cost was measured, not assumed, and
   it differed by width.
2. ~~Port `accurate_addition` to `add_cascades<4>`~~ - done (universal#1340), though not as
   predicted: the merge was the win, the compression pass had to stay, and the result is
   bit-identical to what it replaced. No framework items remain open.
3. ~~Close the generic templates~~ - done; they refuse to compile for unsupported widths.
4. **Then the shared items** (item 4), which are number-system work rather than framework work, and
   which now hold the largest remaining errors in either family - `square` at 4 ulps and `log` at 15
   are the ceiling on `exp`.

The move that has paid off in #1317, #1322, #1324, #1326 and #1318 is the same one every time: where
the cascade improvises, adopt the direct family's proven schedule and express it with the
framework's hardened primitives. #1318 is the purest instance of it: the cascade trigonometry was
the *double-double* schedule wearing a wider type, and porting the `qd` schedule - pi/1024 reduction,
256-entry tables, and enough Newton steps in `atan2` for the width (two for `td_cascade`, three for
`qd_cascade`) - was the whole fix. Item 1 is the first entry on this list where that rule does **not** simply
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
