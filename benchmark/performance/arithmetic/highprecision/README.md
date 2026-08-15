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
date           : 2026-08-15, re-measured after the universal#1317 fix
```

### Scalar operators (nsec/op, lower is better)

```text
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
sink only (floor)              0.20         0.37         0.37         0.41         0.49         0.49
construct                      0.20         0.29         0.27         0.37         0.41         0.41
copy construct                 0.20         0.37         0.37         0.41         0.49         0.49
assign                         0.20         0.37         0.37         0.41         0.49         0.51
add                            0.41        23.86        16.06        35.46        62.44        70.36
subtract                       0.41        23.86        15.53        32.54        61.51        66.16
multiply                       0.82        22.26        24.50       215.81        73.01       581.26
divide                         2.86       216.47        81.30       180.49       594.88       429.73
compare (<)                    0.60         1.69         0.60         0.78         0.67         0.76
compare (==)                   0.41         0.54         0.54         0.41         0.55         0.55
convert to double              0.41         0.41         0.41         0.41         0.41         0.41
convert from double            0.20         0.27         0.29         0.41         0.41         0.41
convert to string            510.09      2694.48      3804.09      4403.04      7722.31      7499.15
convert from string          120.73    454849.97    453252.03    476934.70    486030.26    489235.67
```

The construct/copy/assign/convert rows all sit at the sink floor: a copy is 2 to 4 `movsd`
instructions and there is nothing else to measure. They are reported to establish that floor, not
because they distinguish the implementations.

### Kernels (nsec per elementary multiply-add)

```text
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
dot N=16                       0.10        39.37        49.95        77.57       137.13       157.74
dot N=256                      0.29        29.49        46.51        83.99       141.93       165.08
dot N=4096                     0.40        29.68        46.40        87.10       143.08       163.62
axpy N=4096                    0.13        37.59        41.27        65.89       139.08       133.94
matmul 32x32                   0.24        21.09        49.74        82.43       142.00       148.37
horner deg 20                  0.32        47.49        59.42        75.35       150.29       153.61
```

The `double` column is auto-vectorized and the multi-component columns are not, which is most of the
70x to 1300x gap in the `dd/double` and `qd/double` ratios. That gap is a property of the format, not
of these implementations.

### Mathlib (nsec/op, includes one accumulate; subtract the `accumulate only` row)

```text
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
accumulate only                0.41        23.94        28.81        47.16        71.90        96.60
sqrt                           1.23        42.74       313.70       771.98      1120.91      2879.21
exp                            3.48       916.48      1230.00     13338.90      4169.76     26035.96
log                            3.45      1960.27      2636.61     26134.51     13181.51     75320.89
sin                            3.80       951.43      1446.23      9674.26      3531.18     24653.02
cos                            3.63       960.57      1454.11      9832.77      3525.93     24816.82
```

### Normalized cost, cascade relative to classic (gcc)

Ratio of nsec/op. 1.00 is parity, above 1.00 means the cascade implementation is slower.

```text
operation                dd_cascade/dd   qd_cascade/qd   td_cascade/dd
----------------------------------------------------------------------
add                               0.67            1.13            1.49
subtract                          0.65            1.08            1.36
multiply                          1.10            7.96            9.69
divide                            0.38            0.72            0.83
dot N=256                         1.58            1.16            2.85
dot N=4096                        1.56            1.14            2.93
axpy N=4096                       1.10            0.96            1.75
matmul 32x32                      2.36            1.04            3.91
horner deg 20                     1.25            1.02            1.59
sqrt                              7.34            2.57           18.06
exp                               1.34            6.24           14.55
log                               1.35            5.71           13.33
sin                               1.52            6.98           10.17
cos                               1.51            7.04           10.24
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
within about 20% between gcc and clang, and `qd_cascade` multiply is 6x to 8x `qd` in both. The
post-universal#1317 `qd_cascade` addition is 1.13x `qd` under gcc and 1.25x under clang - same
direction, and slower than `qd` under both.

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
qd_cascade                      15            0.2215             3.852e+31
```

Reading the large residuals as correct decimal digits:

- `qd_cascade` `sqrt`, `exp` and `log` were the original finding of this benchmark: 48 to 50 correct
  digits where `qd` delivers 63. That was **fixed** in universal#1317 - the cause was not in those
  functions but in addition, which returned an exact-valued but overlapping expansion whose fourth
  component the renormalization then dropped. The rows above are post-fix; `exp` and `log` now agree
  with `qd` to the last bit against an exact oracle, and `sqrt` sits at 15 ulps of 212 bits (63.2 of
  63.6 digits). The cost is on the other axis: quad-double addition is now 46% more expensive, which
  is what turned `qd_cascade`'s dot product from 0.83x `qd` into 1.15x.
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

1. **What does the generic `floatcascade<N>` renormalization cost?** For division, nothing: the
   cascade implementations beat the classic ones (0.38x and 0.72x). For addition it is free at N=2
   (0.67x) but costs 13% at N=4, which is the price of the universal#1317 correctness fix - the
   expansion has to be compressed before it can be renormalized without dropping a component. For
   multiplication it is expensive and grows with N: 1.1x at N=2 under gcc, 8.0x at N=4.
   `qd_cascade` multiply at 581 nsec/op against `qd`'s 73 is the single worst row in the suite.
2. **Does the `volatile` hardening cost throughput?** Not measurably at the level this benchmark can
   isolate. The add/subtract rows, where the hardened error-free transformations dominate, are the
   rows where the cascade types *win*.
3. **Is `td_cascade` a useful middle point?** Not on speed. Against `dd` it ranges from 0.83x
   (divide) to 9.7x (multiply), with add/subtract at 1.4x and the kernels at 1.8x to 3.8x, and its
   trigonometry is no more accurate than `dd`'s. Its value has to come from the 159-bit significand
   in code paths that need it (universal#1300 wants it for takum64), not from filling a performance
   gap.
4. **Is there a crossover length where the cascade dot product wins?** No, at either width, and
   this is the one conclusion universal#1317 reversed. `dd_cascade` is 1.3x to 1.6x `dd` at every
   length from 16 to 4096 under both compilers, unchanged. `qd_cascade` used to be *faster* than
   `qd` at every length (0.83x), but that margin came from an addition that was dropping a
   component; with the correct addition it is 1.14x to 1.16x, i.e. slightly slower. The earlier
   win was not real.
5. **Can we retire classic `dd` and `qd`?** Closer than before, but not yet. `dd_cascade` is a
   defensible replacement for `dd` once the multiply is addressed. `qd_cascade`'s accuracy blocker
   is gone (universal#1317), so what remains is purely a speed question: its multiply is 8x `qd`'s,
   and its addition is now 13% more expensive. Both are in the generic expansion code rather than
   in the number system, which is where a follow-up should look.

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
