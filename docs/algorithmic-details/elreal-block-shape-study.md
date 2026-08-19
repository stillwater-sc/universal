# elreal block-shape design study (Phase 9)

Phase 9 of the McCleeary LFPERA `elreal` epic (#923, dissertation section 5.1) is
a two-part evaluation:

1. a correctness/performance comparison of `elreal` against `qd` (quad-double),
   the dissertation's reference type; and
2. a **hardware block-shape design study** -- how the choice of storage block
   shape (the host `FpType` that backs each `block<FpType>`) trades off
   convergence rate, latency, and memory footprint.

`elreal` stores an exact real as a lazily-materialised **ZBCL** -- a
zero-overlap co-list of `block<FpType>` limbs, each carrying `k =
numeric_limits<FpType>::digits` significand bits. The block shape is the host
`FpType`. This document reports the measured trade-offs across the four
candidate shapes and gives a first recommendation.

> This is the **MVP** deliverable for Phase 9 (#933). It covers the block-shape
> convergence/footprint study, dot-product throughput, and the `qd` precision
> ceiling. Geometric predicates (orient2d/incircle), cancellation-stressed
> sums, and the full recommendation matrix are tracked as Phase 9 follow-ups.

## How to reproduce

```sh
cmake -DUNIVERSAL_BUILD_NUMBER_ELREALS=ON -DUNIVERSAL_BUILD_BENCHMARKS=ON -DUNIVERSAL_BUILD_BENCHMARK_PERFORMANCE=ON ..
make benchmark_elreal_performance
./benchmark/performance/arithmetic/benchmark_elreal_performance
```

The sweep oracle (random `elreal` arithmetic vs the exact dyadic-rational
oracle, since Universal is dependency-free and does not link mpfr) lives at
`elastic/elreal/oracle/sweep.cpp` and runs under the standard regression tiers:

```sh
make el_oracle_sweep && ./elastic/elreal/el_oracle_sweep
```

## Method

- **Hosts (block shapes):** `half` (`cfloat<16,5>`, k=11), `bfloat16` (k=7),
  `float` (k=24), `double` (k=53).
- **Oracle:** the exact 320-digit reference constants
  (`include/sw/math/constants/reference_constants.hpp`) via
  `agreed_decimal_digits(zbcl_to_dyadic(stream), ref)`; and the exact
  dyadic-rational type for arbitrary arithmetic.
- **Convergence metric:** decimal digits of the materialised ZBCL that agree
  with the reference, and the number of blocks that carried them.

The numbers below are indicative (single dev host, `-O2 -DNDEBUG`); absolute
times and timing ratios are machine-specific (CPU, compiler, optimization, load),
while the digits/blocks columns are reproducible for this configuration.

## A. Memory footprint per block shape

| Host      | k (significand bits) | sizeof(block) | payload bits/block |
|-----------|----------------------|---------------|--------------------|
| half      | 11                   | 36 B          | 11                 |
| bfloat16  | 7                    | 36 B          | 7                  |
| float     | 24                   | 36 B          | 24                 |
| double    | 53                   | 40 B          | 53                 |

`block<FpType>` carries a wide (`integer<256>`) exponent field for unbounded
scale, so the struct is dominated by that exponent, not by the host mantissa --
`half`/`bfloat16`/`float` blocks are all 36 B, `double` is 40 B. **The narrow
hosts do not save block memory; they only reduce payload bits per block**, which
means they need *more* blocks for the same precision. This is the central
finding for silicon: a narrow block shape is only attractive if the per-block
datapath (an EFT twoSum/twoProd at width k) is correspondingly cheaper.

## B. Convergence: blocks to reach a target precision

Blocks needed to first agree with the reference to 50/100/200/320 decimal
digits, and the saturation point (max digits reached and at how many blocks).

| Host     | const | b@50 | b@100 | b@200 | b@320 | saturation            |
|----------|-------|------|-------|-------|-------|-----------------------|
| half     | pi    |  -   |  -    |  -    |  -    | does not converge     |
| bfloat16 | pi    |  -   |  -    |  -    |  -    | max 33 digits @ 13    |
| float    | pi    |  -   |  -    |  -    |  -    | max 37 digits @ 5     |
| double   | pi    |  5   |  9    | 17    |  -    | max 306 digits @ 19   |
| half     | e     |  -   |  -    |  -    |  -    | does not converge     |
| bfloat16 | e     |  -   |  -    |  -    |  -    | max 33 digits @ 14    |
| float    | e     |  -   |  -    |  -    |  -    | max 37 digits @ 7     |
| double   | e     |  7   |  9    | 18    |  -    | max 307 digits @ 19   |
| half     | sqrt2 |  -   |  -    |  -    |  -    | does not converge     |
| bfloat16 | sqrt2 |  -   |  -    |  -    |  -    | max 36 digits @ 13    |
| float    | sqrt2 |  -   |  -    |  -    |  -    | max 38 digits @ 5     |
| double   | sqrt2 |  4   |  8    | 16    |  -    | max 312 digits @ 19   |

Reading the table:

- **`double` is the only host that reaches the high-precision regime** the type
  exists for -- ~300+ digits in 19 blocks. It clears 200 digits in 16-18
  blocks across all three constants.
- **`float` and `bfloat16` saturate at ~35 digits and stop improving with
  depth.** Strikingly, `bfloat16` uses *more* blocks than `float` (13 vs 5) to
  reach *fewer* digits. Precision is capped not by block storage but by the
  **series arithmetic degrading in the narrow host** -- the transcendental
  generators compute their terms in `FpType`, and below ~`float` those terms
  lose too much accuracy to refine the tail. This matches the Phase 7 descope of
  bfloat16 transcendentals (#1051).
- **`half` does not converge at all** for these division-based series: the
  Machin/Newton/Taylor generators drive the host past `fp16`'s 5-bit exponent
  range (the online-division floor). This is the known `fp16`-host blocker; a
  full `half` characterisation waits on the division floor-lift.

## C. Time-to-first-block (latency)

Wall time to produce the first block of each transcendental generator at depth
16 (the latency that matters when an algorithm needs only a few digits fast).

| Host   | pi       | e        | sqrt2    |
|--------|----------|----------|----------|
| float  | ~5.4 ms  | ~1.8 ms  | ~0.33 ms |
| double | ~790 ms  | ~56 ms   | ~4.7 ms  |

The `double` host's first-block latency is dominated by the cost of the
division-heavy generators at width 53 (`pi` via Machin is ~170x more expensive
than `sqrt2` via Newton: ~790 ms vs ~4.7 ms). If time-to-first-digit matters
more than ultimate
precision, a narrower host reaches its (lower) ceiling far faster.

## D. Stream-wise ZBCL dot-product throughput

Dot product of two length-N ZBCL vectors at multiply depth 32.

| Host   | N=16       | N=64       | N=256      |
|--------|------------|------------|------------|
| float  | ~20 us/dot | ~126 us/dot| ~548 us/dot|
| double | ~8.7 us/dot| ~36 us/dot | ~140 us/dot|

`double` is ~2-4x faster per dot than `float` here despite the wider datapath,
because the `float` host needs more blocks to represent each product, so the
online multiply/add churn more limbs.

## E. Precision ceiling: elreal vs qd

`pi`, digits agreeing with the 320-digit reference:

| Type            | digits | note                                   |
|-----------------|--------|----------------------------------------|
| `qd`            | 64     | fixed ceiling (4x double ~ 63 digits)  |
| `elreal<double>`| 306    | unbounded; depth 96 shown              |

`qd` is a fixed 4-limb type: it tops out near 63-64 digits by construction.
`elreal<double>` matches `qd` in `qd`'s range and then keeps going to hundreds
of digits with more depth. That unbounded refinement -- at the cost of lazy,
per-block latency -- is the whole point of the type.

## Recommendation (first cut)

- **Want unbounded / very-high precision (100+ digits):** the only viable block
  shape today is **`double`**. It is also the fastest for multi-block work.
- **Want a bounded ~60-digit fixed-precision floating-point type with a simpler
  API:** use **`qd`** directly; `elreal<double>` only wins past `qd`'s ceiling.
- **Narrow block shapes (`float`/`bfloat16`/`half`) are not yet a good precision
  trade:** they save no block memory (wide exponent dominates), cap out at low
  precision because the series arithmetic degrades, and `half` does not converge
  at all. They remain interesting *only* as a silicon study of cheap per-block
  EFT datapaths -- and that case needs (a) the fp16 division floor-lift and (b)
  extended-precision intermediate series evaluation (#1051) before the narrow
  hosts can be fairly characterised.

## F. Cancellation-stressed accumulation

Sections A-E measure how fast `elreal` converges and how much it costs. This one
measures the property the type exists for: accumulating sums whose individual
terms dwarf their own total. Both workloads are scored against the **exact
dyadic value of the same terms**, so the only thing being compared is the
accumulator.

### F1. Naive Taylor `exp(-40)`

The textbook catastrophic-cancellation series. Terms come from the naive
recurrence `t_k = t_{k-1} * (-40/k)` evaluated in `double` -- that is what
"naive" means here: each term arrives already rounded. 180 terms, peak
`|term|` ~ `1.48e16` against a true value of `4.25e-18`, so the condition
number is ~`3.5e33`.

| accumulator | digits agreeing with the exact sum of those terms |
|-------------|--------------------------------------------------|
| `double`    | 0                                                |
| `qd`        | 320 (exact)                                      |
| `elreal`    | 320 (exact)                                      |

**`qd` is not beaten here, and the honest reading is more interesting than a
win.** The terms span `1e16` down to `1e-40`, about 186 bits, which fits inside
`qd`'s 212-bit budget -- so `qd` carries this sum exactly too. Naive `double` is
the only casualty.

The finding that matters is the next line: **the exact sum of those terms agrees
with `exp(-40)` to 0 digits.** The terms were rounded before any accumulator saw
them, and no amount of exactness downstream can rebuild information destroyed
upstream. For this workload exact accumulation is necessary and nowhere near
sufficient. This is worth stating plainly because the obvious reading of "elreal
accumulates exactly, therefore elreal fixes naive Taylor" is wrong.

What actually computes `exp(-40)` is `elreal`'s own `exp()`, which reduces the
argument (`exp(x) = exp(x/2^r)^(2^r)`) instead of summing a wildly alternating
series, and so never enters the cancellation regime at all. It agrees with
`std::exp(-40)` to all 16 digits the `double` reference carries.

### F2. Ill-conditioned dot product

Here the answer itself is made to exceed what a fixed-limb type can represent.
The exact dot is spread over `chunks` pieces of 52 bits each, separated by 100
bits, and large `+P`/`-P` pairs well above the answer supply the cancellation.
The vectors are shuffled -- adjacent cancelling pairs would cancel with no
rounding at all, which is the opposite of the intended stress. Both factors carry
26 bits, so every product is exact in `double` and product rounding is not part
of what is measured.

The answer is centred on `2^0` rather than anchored at the top, because
anchoring puts the bottom chunk at `2^(-100*(chunks-1))`, which at 12 chunks is
`2^-1100` -- below the smallest subnormal. That chunk rounds to zero, and the row
then reports a span it does not have. Both generators now check every term for
survival in binary64 and the oracle fails outright if one does not, so the
"answer spans" column cannot drift away from the answer actually being measured.

| chunks | answer spans | `double` | `qd` | `elreal` |
|--------|--------------|----------|------|----------|
| 2      | 152 bits     | 0        | 320  | 320      |
| 4      | 352 bits     | 0        | 90   | 320      |
| 6      | 552 bits     | 0        | 90   | 320      |
| 8      | 752 bits     | 0        | 89   | 320      |
| 12     | 1152 bits    | 0        | 90   | 320      |

(320 is the reference cap -- exact as far as the oracle can see.)

`double` returns nothing at any size. `qd` is exact while the answer fits its
four limbs and then plateaus near **90 digits** -- not its usual ~64. The
difference is instructive: `qd` is a non-overlapping expansion, so its four
limbs can sit at arbitrary scales rather than packing 212 contiguous bits. Four
limbs at 100-bit spacing reach down to about `2^-300`, hence ~90 decimal digits.
The ceiling is the *limb count*, not the bit count, and the workload is built to
find it. `elreal` carries every chunk, so it stays exact as the answer grows
past a thousand bits.

This is the clean statement of what the type buys: not "more precision" but
**precision that is not budgeted in advance**.

Both workloads are also pass/fail regressions in `elastic/elreal/oracle/sweep.cpp`
(`elreal<double> cancellation-stressed accumulation is exact`), so the exactness
claim is guarded rather than merely reported.

## G. Exact geometric predicates

`orient2d` and `incircle` are the canonical robust-predicates workload. Both are
sign queries on a determinant, and the sign is what a mesh generator or convex
hull consumes: a predicate that returns the wrong sign near a degeneracy does
not produce a slightly wrong mesh, it produces an inconsistent one, and the
algorithm above it can loop or crash.

Each predicate is written once over a generic `Real` and instantiated for
`double`, `qd` and `elreal<double>`; the reference is the same expression in
exact dyadic rationals.

| workload | `double` | `qd` | `elreal` | cases | exactly degenerate |
|----------|----------|------|----------|-------|--------------------|
| orient2d, collinear + ulp grid | **5474** | 0 | 0 | 16384 | 128 |
| incircle, exactly cocircular +/- ulps | **105** | 0 | 0 | 288 | 60 |
| incircle, near-cocircular | **486** | 0 | 0 | 4096 | 0 |

The degenerate column is worth its own row rather than a footnote. Exact
cocircularity of four independently rounded points is a measure-zero event, so
the random near-cocircular sampling never produces one -- it cannot exercise the
case where a predicate has to answer *zero*. The middle workload is built to:
four points at the cardinal positions of a circle whose centre and radius are
exactly representable are exactly cocircular by construction.

`double` fails on a third of the orient2d grid and a sixth of the incircle
cases. These are not exotic inputs -- they are ordinary coordinates near a
degeneracy, which is where a mesh algorithm spends its time.

**`qd` gets both exactly right, and that is the honest result.** Shewchuk's
analysis puts `orient2d` at ~2x and `incircle` at ~4x working precision; `qd`
supplies 4x, and on `double` inputs at ordinary scales that is enough. Attempts
to break it here did not succeed: wide dynamic range alone does not help,
because spreading the coordinates makes the determinant *large* relative to the
error rather than small, so the inputs stop being degenerate at all.

`elreal` is also exact, for a different reason. `qd` is exact because somebody
did the error analysis and the answer happened to fit; `elreal` is exact because
it has no budget to exceed. The guarantee survives a change of predicate, of
scaling, or of input distribution without anyone re-deriving a bound -- which is
the property a predicate library actually wants.

### Three traps, for anyone using elreal this way

Both cost real debugging time while this section was written, and both are now
guarded in `elastic/elreal/oracle/sweep.cpp`.

**`operator*` is depth-bounded.** At the default precision the determinant is
truncated and near-degenerate signs come out wrong. Raise the depth with
`elreal_precision_guard` for exact work.

**Do not step ulps from a coordinate that is exactly zero.** `from_native`
refuses subnormal inputs by contract, and `nextafter(0.0, ...)` lands squarely in
the subnormal range. In a release build this silently produced non-normalised
blocks and 12 wrong signs; in a debug build it aborted on the assertion. The
generator now checks every coordinate and reports what it skipped, because a
workload that quietly drops cases is worse than one that fails loudly. The
failure was in the test data, not in elreal -- the contract is documented at
`from_native`.

**Take the sign from the comparison operators, not from `sign()` or the raw
block stream.** `sign()` answers +1 for zero, and a cancelling sum stays lazily
unnormalised, so the leading raw block carries a phantom sign. Reading the
stream directly gave the wrong answer on 61 of the 128 exactly-collinear grid
points -- precisely the inputs a predicate exists to detect -- while being
correct on all 16256 non-degenerate ones. `elreal_cmp`, which the comparison
operators route through, skips zero blocks and is correct everywhere.

## Re-validation

The study is a measurement, so it goes stale when the code under it moves. It
was re-run end to end on `1e868e42` (2026-08-19), gcc 13.3 and clang 18.1,
`-O2 -DNDEBUG`, and the tables above carry those numbers. Both compilers agree
cell for cell on the digits/blocks columns; the timing columns are indicative
and reproduce to within a few percent between runs on this host.

Every qualitative finding held. Three cells moved, all in the same direction:

| row              | at the MVP (2026-07-23) | now         |
|------------------|-------------------------|-------------|
| `double` / pi    | 306 digits @ 20 blocks  | 306 @ **19** |
| `double` / e     | **308** digits @ 20 blocks | **307** @ **19** |
| `float` / pi     | 37 digits @ 6 blocks    | 37 @ **5**  |

These trace to #1292, which stopped streaming addition from emitting subnormal
ZBCL blocks. The block that vanished was the subnormal one at the tail, so the
same precision now arrives one block sooner -- and `e` loses the sliver of a
digit that partial block had been carrying. A smaller number in the block
column is the improvement here, not a regression.

One correction rather than a change: table A gave `bfloat16` **8** significand
bits. `numeric_limits<bfloat16>::digits` is 7, and was already 7 at the commit
that published this study, so that cell was wrong when written -- the benchmark
itself printed 7 all along. The 7 vs 8 does not affect any conclusion: the
narrow hosts are capped by the series arithmetic, not by payload bits per
block, which is the point table A exists to make.

## Follow-ups (remaining Phase 9 scope)

- ~~#1186 -- geometric predicate suite~~ -- **done**, section G above.
- ~~#1187 -- cancellation-stressed sums~~ -- **done**, section F above.
- **#1188** -- full recommendation matrix ("X precision at Y latency -> pick
  Z"). Still blocked, and blocked on the physics rather than on effort: the
  matrix cannot be filled in while `half` does not converge and `float` /
  `bfloat16` saturate near 35 digits. It needs the fp16 division floor-lift and
  extended-precision intermediate series evaluation (#1051), plus the
  characterisation tooling in #1176.
- SIMD/FMA acceleration is explicitly a possible Phase 10, out of scope here.
