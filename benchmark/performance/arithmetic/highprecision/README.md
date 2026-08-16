# High-precision multi-component performance: classic vs floatcascade

Tracking issue: [universal#1315](https://github.com/stillwater-sc/universal/issues/1315)

`dd_cascade`, `td_cascade` and `qd_cascade` are the modernized multi-component types built on the
`floatcascade<N>` framework, and the stated intent is that they replace the hand-specialized classic
`dd` and `qd` (see `include/sw/universal/number/dd_cascade/README.md`, *Unify the codebase*).

That decision needs numbers. The cascade implementations use volatile-hardened error-free
transformations and a generic N-component Priest renormalization; the classic implementations use
hand-unrolled Bailey/Hida sequences specialized to 2 and 4 components. This directory measures what
the generalization costs, per operation, with `double` as the reference floor.

## The four programs

| program | what it measures |
|---|---|
| `benchmark_hp_scalar` | scalar operator latency: construct/copy/assign, add, subtract, multiply, divide, compare, conversions to and from `double` and decimal strings |
| `benchmark_hp_kernels` | composite kernels, all normalized to one elementary multiply-add: dot (N=16/256/4096), axpy, 32x32 matmul, degree-20 Horner |
| `benchmark_hp_mathlib` | `sqrt`, `exp`, `log`, `sin`, `cos` |
| `benchmark_hp_equivalence` | the guardrail: do the classic and cascade implementations compute the same answer, and when they do not, which one is losing digits |

## Building and running

```bash
mkdir build && cd build
cmake -DUNIVERSAL_BUILD_BENCHMARK_PERFORMANCE=ON ..
make -j4 benchmark_hp_scalar benchmark_hp_kernels benchmark_hp_mathlib benchmark_hp_equivalence

./benchmark/performance/arithmetic/benchmark_hp_scalar          # default 0.05 sec measurement window
./benchmark/performance/arithmetic/benchmark_hp_scalar 0.25     # longer window, less noise
```

The optional argument is the target wall-clock window per measurement. Every measurement calibrates
its own operation count to that window, so a `qd` division and a `double` addition both get a
statistically useful sample without either taking minutes.

On a hybrid CPU, pin the run: `taskset -c 0 ./benchmark/performance/arithmetic/benchmark_hp_scalar`.
The numbers recorded below were taken that way.

## Methodology

**Latency, not peak throughput.** Every scalar workload is a dependent chain: the result of one
operation is the input to the next. That is the number a renormalization-heavy implementation has to
answer for. The kernels cover the other side, where independent work can overlap.

**The optimizer barrier.** Multi-component arithmetic is exactly the code a compiler likes to
delete. Every workload hands its result to `hpbench::consume()`, which stores *every component*
through a `volatile` sink. Consuming only the leading limb is not enough: converting a `qd` to a
`double` hands the compiler limb 0, and it is then free to prove the other three limbs dead. The
generated code was checked:

```asm
sinkLoop(size_t, dd const*):        # the empty-loop floor
        movsd   (%rax), %xmm0
        movsd   %xmm0, hpbench::g_sink(%rip)      # limb 0 stored
        movsd   8(%rax), %xmm0
        movsd   %xmm0, hpbench::g_sink(%rip)      # limb 1 stored
```

The `sink only (floor)` row in the scalar tables is that loop, measured. Any row at the floor is
reporting the cost of a store, not of the operation.

**No loop-invariant kernels.** A kernel over constant operands computes the same answer on every
call, and clang hoists the whole thing out of the timing loop (it did exactly that to the `double`
Horner evaluation before this was fixed). Each kernel invocation is seeded with a different starting
value, which makes every call distinct without adding work to the inner loop.

**Sampling.** Best of 3 runs at the calibrated operation count. Operands are drawn from a fixed-seed
LCG in `[0.5, 2.0)`, so runs are reproducible and the multiplicative and additive chains stay near
1.0 instead of drifting to infinity.

## Recorded results

```text
processor      : 12th Gen Intel(R) Core(TM) i7-12700K, pinned to core 0
compiler       : gcc 13.3.0, -O3 -DNDEBUG, C++20, no ISA extensions beyond baseline
measurement    : best of 3, 0.050 sec calibration window
date           : 2026-08-16, re-measured after universal#1317 and #1322 (both widths)
  window         : 0.25 sec (the recorded run uses a longer window than the default)
```

### Scalar operators (nsec/op, lower is better)

```text
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
sink only (floor)              0.20         0.37         0.37         0.41         0.50         0.49
construct                      0.20         0.29         0.27         0.37         0.41         0.41
copy construct                 0.21         0.37         0.37         0.41         0.49         0.49
assign                         0.20         0.37         0.37         0.41         0.49         0.49
add                            0.41        23.88        16.05        35.09        62.51        70.27
subtract                       0.41        23.89        15.31        32.61        61.52        66.40
multiply                       0.82        22.30        24.55        62.98        73.09        83.13
divide                         2.86       216.87        78.15       209.24       587.77       442.61
compare (<)                    0.62         1.69         0.61         0.75         0.65         0.74
compare (==)                   0.41         0.55         0.55         0.41         0.55         0.55
convert to double              0.41         0.41         0.41         0.41         0.41         0.41
convert from double            0.21         0.27         0.29         0.41         0.41         0.41
convert to string            513.61      2708.69      3582.20      5973.99      7708.84      8783.55
convert from string          120.75    387242.40    386866.48    404230.50    418011.99    417810.97
```

The construct/copy/assign/convert rows all sit at the sink floor: a copy is 2 to 4 `movsd`
instructions and there is nothing else to measure. They are reported to establish that floor, not
because they distinguish the implementations.

### Kernels (nsec per elementary multiply-add)

```text
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
dot N=16                       0.10        40.56        52.91        80.46       145.44       150.11
dot N=256                      0.29        33.44        46.64        86.25       146.91       158.19
dot N=4096                     0.40        33.52        46.70        90.45       145.80       163.54
axpy N=4096                    0.17        39.02        42.02        77.49       143.15       144.15
matmul 32x32                   0.24        24.34        51.06        85.29       145.11       164.03
horner deg 20                  0.33        47.72        58.79       104.52       154.35       171.88
```

The `double` column is auto-vectorized and the multi-component columns are not, which is most of the
70x to 1300x gap in the `dd/double` and `qd/double` ratios. That gap is a property of the format, not
of these implementations.

### Mathlib (nsec/op, includes one accumulate; subtract the `accumulate only` row)

```text
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
accumulate only                0.42        23.96        29.25        47.28        71.86        95.95
sqrt                           1.25        42.77       299.04       726.04      1123.38      2221.94
exp                            3.56       918.07      1174.77      3280.47      4166.86      6761.49
log                            3.45      1963.75      2507.83      7009.07     13186.61     22049.89
sin                            3.83       952.96      1280.67      2846.36      3530.69      7106.00
cos                            3.66       960.09      1284.42      2867.30      3538.01      7058.37
```

### Normalized cost, cascade relative to classic (gcc)

Ratio of nsec/op. 1.00 is parity, above 1.00 means the cascade implementation is slower.

```text
operation                dd_cascade/dd   qd_cascade/qd   td_cascade/dd
----------------------------------------------------------------------
add                            0.67            1.12            1.47
subtract                       0.64            1.08            1.37
multiply                       1.10            1.14            2.82
divide                         0.36            0.75            0.96
dot N=256                      1.39            1.08            2.58
dot N=4096                     1.39            1.12            2.70
axpy N=4096                    1.08            1.01            1.99
matmul 32x32                   2.10            1.13            3.50
horner deg 20                  1.23            1.11            2.19
sqrt                           6.99            1.98           16.97
exp                            1.28            1.62            3.57
log                            1.28            1.67            3.57
sin                            1.34            2.01            2.99
cos                            1.34            2.00            2.99
```

## Code generation sensitivity

The scalar `dd_cascade` add and multiply rows are not a property of the source alone.

**They flip direction between compilers.** Same source, same machine, `dd_cascade/dd`:

| operation | gcc 13.3 | clang 18.1 |
|---|---|---|
| add | **0.67** (cascade faster) | **1.50** (cascade slower) |
| subtract | **0.64** | **1.45** |
| multiply | **1.34** (cascade slower) | **0.81** (cascade faster) |
| divide | 0.38 | 0.40 |

The absolutes behind the flip: gcc compiles the `dd_cascade` addition to 16.1 nsec and clang to
33.8; gcc compiles the `dd_cascade` multiplication to 29.8 nsec and clang to 24.6. Classic `dd` is
stable across both (23.9 / 22.4 add, 22.2 / 30.5 multiply).

**They move under changes that do not touch them.** During review, the harness's `consume()` was
refactored from an `if constexpr` on a trait constant to a trait member function - a change with no
effect on the multiply workload, which calls `consume()` exactly once after the timed loop. gcc's
`dd_cascade` multiply went from 45.2 to 29.8 nsec/op, a 1.5x shift, reproducibly. Classic `dd` did
not move (22.2 both times).

The conclusion is not that the measurements are unreliable - they are stable to within 1% run to
run, and the kernel and mathlib rows barely moved - but that `dd_cascade`'s scalar add and multiply
sit close enough to an inlining cliff that gcc falls off either side of it. Any decision that
depends on those two rows should be re-measured on the exact build in question, and should not be
made from a single compiler.

Everything else is stable in direction across compilers: the kernel and mathlib ratios agree to
within about 20% between gcc and clang. After universal#1322, `qd_cascade` multiply is 1.13x `qd`
under gcc and 0.89x under clang - the only remaining row where the two compilers disagree about
which implementation is ahead, and by a margin small enough that neither answer is interesting.
The post-universal#1317 `qd_cascade` addition is 1.13x `qd` under gcc and 1.33x under clang: same
direction, slower than `qd` under both.

### AVX2

`-mavx2 -mfma` (`cmake -DUNIVERSAL_USE_AVX2=ON`) changes little, with one exception: gcc's
`dd_cascade` addition regresses from 16.1 to 29.1 nsec/op, which erases the only large win the
cascade addition had. clang is within noise on every row. There is no measurable AVX2 benefit for
any of these types, which is expected: the error-free transformations are scalar dependent chains,
not vectorizable work.

## The equivalence guardrail

A performance difference is only meaningful if both implementations compute the same thing.
`benchmark_hp_equivalence` runs both members of each pair over the same operands, and separately
evaluates three identities inside each type (`sqrt(x)^2 == x`, `exp(log(x)) == x`,
`sin^2 + cos^2 == 1`) so that a disagreement can be attributed rather than merely noted.

**Arithmetic is bit-identical.** `add`, `subtract`, `multiply`, `divide`, the dot product and the
Horner evaluation agree bit-for-bit on all 4096 samples, for both `dd` vs `dd_cascade` and `qd` vs
`qd_cascade`. The arithmetic and kernel rows of those two pairs are like-for-like comparisons.

`td_cascade` has no classic counterpart, so its column is a datapoint about a 3-component type
rather than a comparison: the guardrail runs it against `dd` only to establish that its third
component carries information. It does. Comparing component by component with the `dd` value
zero-padded to three components, `add`, `subtract` and `multiply` agree exactly (the third component
is genuinely zero, because both operands came from a single `double`), while `divide`, the dot
product and every mathlib function differ from `dd` by up to a few ulps of `dd` - which is the
third component doing its job.

**The mathlib is not.** Self-consistency residuals, in ulps of each type's own significand:

```text
type                 sqrt(x)^2 - x   exp(log(x)) - x     sin^2 + cos^2 - 1
--------------------------------------------------------------------------
double                       1.999                 0                     2
dd                           10.83             1.479                 10.88
dd_cascade                   2.867             1.479                 2.389
td_cascade                   7.198            0.8534             4.277e+15
qd                           1.046             0.215             1.494e+23
qd_cascade                   8.572            0.2392             3.852e+31
```

Reading the large residuals as correct decimal digits:

- `qd_cascade` `sqrt`, `exp` and `log` were the original finding of this benchmark: 48 to 50 correct
  digits where `qd` delivers 63. That was **fixed** in universal#1317 (an overlapping expansion
  returned by addition, whose fourth component the renormalization then dropped) and improved again
  in universal#1322 (multiplication rewritten to the classic qd_mul schedule).

  What the numbers above show, and what they do not: the residuals are *self-consistency* checks,
  so they bound the error from below and cannot certify a result as correctly rounded. On that
  evidence `qd_cascade` went from 2.1e+15 ulps to 8.6 on `sqrt` and from 4.7e+13 to 0.24 on
  `exp(log(x))`, against 1.0 and 0.22 for `qd` - the same order of magnitude, no longer 13 to 15
  digits apart. The stronger claim, that `qd_cascade` multiplication is now bit-for-bit identical
  to `qd`, rests on an external exact oracle rather than on this program; the in-repo artifact for
  it is `static/highprecision/qd_cascade/arithmetic/addition_oracle.cpp`, which pins addition and
  multiplication to within 2^-205 and 2^-210 of exact dyadic arithmetic.

  The cost of the first fix is on the other axis: quad-double addition is 46% more expensive, which
  is what turned `qd_cascade`'s dot product from 0.83x `qd` into 1.10x.
- `sin`/`cos` above double-double precision lose digits on every type tested, over the sampled
  domain `[0.5, 2.0)`: `qd` holds about 41 digits, `qd_cascade` and `td_cascade` about 32, which is
  `dd` precision. The extra limbs are not carrying information through the trigonometric evaluation.
  Arguments outside `[0.5, 2.0)`, where a real argument reduction is needed, are not sampled here
  and could well be worse.
- `dd` and `dd_cascade` are both sound; `dd_cascade`'s `sqrt` is the more accurate of the two
  (2.9 vs 10.8 ulps).

After universal#1317, the `sqrt`/`exp`/`log` rows for the `qd` pair **are** like-for-like: both
implementations now deliver the format's precision, so the timings compare equal work. The
`sin`/`cos` rows are still not - every type above `dd` loses digits there (universal#1318) - and
should be read with that caveat.

**Decimal string parsing is pathologically slow** for all five types: 45 usec to parse `"1.5"` into a
`dd` and ~460 usec for a 62-digit string, versus 119 nsec for `std::stod`. The cost scales with digit
count and is the same for every multi-component type, so it lives in shared conversion code rather
than in any one number system.

These three findings are tracked separately; they are defects surfaced by the benchmark, not
benchmark results.

## What the numbers say

1. **What does the generic `floatcascade<N>` renormalization cost?** Much less than it did. For
   division, nothing: the cascade implementations beat the classic ones (0.36x and 0.75x). For
   addition it is free at N=2 (0.68x) and costs 13% at N=4, the price of the universal#1317
   correctness fix. Multiplication used to be the suite's worst row at 8.0x `qd`; universal#1322
   replaced the sort-based accumulation with a transliteration of the classic qd_mul schedule, at
   both the 3- and 4-component widths, and it is now 1.14x at N=4 and 2.8x `dd` at N=3. What remains is the generic machinery's floor: a cascade operation carries its
   components through a renormalization that the hand-specialized code folds into the arithmetic.
2. **Does the `volatile` hardening cost throughput?** Not measurably at the level this benchmark can
   isolate. On the add/subtract rows, where the hardened error-free transformations dominate,
   `dd_cascade` is the faster implementation (0.67x and 0.64x `dd`). `qd_cascade` is slightly
   slower there (1.12x and 1.08x `qd`), but that is the compression universal#1317 added, not the
   hardening: the same hardening is in both widths and only the wider one pays.
3. **Is `td_cascade` a useful middle point?** On speed it is now what its width suggests: against
   `dd` it ranges from 0.96x (divide) to 2.8x (multiply), with add/subtract at 1.4x, the kernels at
   2.0x to 3.5x, and the mathlib at 3.0x to 3.6x - roughly the cost of carrying a third component,
   where before the schedule port it paid 9.4x on multiply and 13x to 15x on exp/log. It also sits
   between `dd` and `qd` on the mathlib in absolute terms, which is the point of a middle width.
   The remaining caveat is accuracy, not speed: its trigonometry is no more accurate than `dd`'s
   (universal#1318), so the 159-bit significand only pays off outside `sin`/`cos`.
4. **Is there a crossover length where the cascade dot product wins?** No, at either width.
   `dd_cascade` is 1.4x to 1.5x `dd` at every length from 16 to 4096 under both compilers.
   `qd_cascade` measured *faster* than `qd` (0.83x) before universal#1317, but that margin came
   from an addition that was dropping a component; with the correct addition it is 1.10x to 1.15x.
   The earlier win was not real.
5. **Can we retire classic `dd` and `qd`?** For `qd`, this is now a real option. universal#1317
   removed the accuracy blocker and universal#1322 removed the speed one: `qd_cascade` multiplication
   is exact to within 2^-210 of dyadic arithmetic (the in-repo oracle test) and identical to `qd`
   against an external decimal oracle, the mathlib residuals are within an order of magnitude of
   `qd`'s, and the cost is 1.1x to 2.0x rather than 6x to 8x - with division actually faster
   (0.75x). Whether
   1.1x on add/multiply is an acceptable price for one implementation instead of two is a
   judgement call, not a measurement. `dd_cascade` is the better bargain: faster than `dd` on
   add/subtract/divide, 1.07x on multiply, and its `sqrt` is more accurate.

   The remaining outlier at every width is `sqrt`, 7.0x `dd` and 2.0x `qd`, because the cascade
   types reach it through Newton iteration on division while classic `qd` uses a multiplication-only
   reciprocal iteration. `td_cascade` division is also the one operation the schedule port left
   behind: it is still 3.2 ulps of the 159-bit format worst case against an exact oracle, where its
   multiplication is now 0.23.

## Caveats

- Single machine, single microarchitecture. The `dd_cascade` add and multiply rows are sensitive to
  code generation (they flip between compilers and moved 1.5x under an unrelated harness refactor);
  re-measure them on the build in question before generalizing.
- The `double` column is auto-vectorized in the kernels and the multi-component columns are not. The
  `dd/double` and `qd/double` ratios are therefore format-vs-format, not implementation-vs-
  implementation.
- The mathlib rows for the `qd` pair compare implementations of *different accuracy* (see above).
- Hybrid-core CPUs need pinning; unpinned runs on the i7-12700K varied by up to 15% on the cheaper
  rows.
