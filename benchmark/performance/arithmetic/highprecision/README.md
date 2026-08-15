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
date           : 2026-08-15
```

### Scalar operators (nsec/op, lower is better)

```text
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
sink only (floor)              0.21         0.37         0.37         0.41         0.49         0.49
construct                      0.20         0.27         0.29         0.41         0.41         0.41
copy construct                 0.21         0.37         0.37         0.41         0.49         0.49
assign                         0.20         0.37         0.37         0.41         0.49         0.49
add                            0.41        23.86        16.05        35.05        62.37        48.44
subtract                       0.41        23.87        15.30        32.62        61.57        45.59
multiply                       0.82        22.21        29.77       216.22        72.99       585.36
divide                         2.86       216.42        81.32       179.88       594.57       373.23
compare (<)                    0.48         1.69         0.61         0.55         0.65         0.57
compare (==)                   0.41         0.55         0.54         0.41         0.55         0.54
convert to double              0.41         0.41         0.41         0.41         0.41         0.41
convert from double            0.21         0.29         0.27         0.37         0.41         0.41
convert to string            511.09      2713.63      3792.76      4417.47      7728.11      6199.56
convert from string          119.20    456606.59    458821.04    473846.43    489229.24    490162.74
```

The construct/copy/assign/convert rows all sit at the sink floor: a copy is 2 to 4 `movsd`
instructions and there is nothing else to measure. They are reported to establish that floor, not
because they distinguish the implementations.

### Kernels (nsec per elementary multiply-add)

```text
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
dot N=16                       0.10        40.38        52.68        82.08       145.66       114.42
dot N=256                      0.31        33.30        46.32        87.71       147.02       122.69
dot N=4096                     0.40        33.39        46.72        90.83       145.74       121.54
axpy N=4096                    0.16        39.11        41.79        69.90       143.22        84.62
matmul 32x32                   0.24        24.33        50.81        92.00       145.27       114.97
horner deg 20                  0.32        47.66        59.38        89.26       154.24       124.43
```

The `double` column is auto-vectorized and the multi-component columns are not, which is most of the
70x to 1300x gap in the `dd/double` and `qd/double` ratios. That gap is a property of the format, not
of these implementations.

### Mathlib (nsec/op, includes one accumulate; subtract the `accumulate only` row)

```text
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
accumulate only                0.41        23.93        32.43        50.37        71.89        62.76
sqrt                           1.23        42.77       326.05       793.15      1201.47      2484.26
exp                            3.49       932.47      1335.72     13326.21      4436.20     25338.94
log                            3.47      1992.89      2850.57     25703.05     14029.56     72903.92
sin                            3.80      1000.84      1475.24      9648.44      3800.25     24301.56
cos                            3.62      1008.58      1485.98      9769.45      3799.47     24583.19
```

### Normalized cost, cascade relative to classic (gcc)

Ratio of nsec/op. 1.00 is parity, above 1.00 means the cascade implementation is slower.

```text
operation                dd_cascade/dd   qd_cascade/qd   td_cascade/dd
----------------------------------------------------------------------
add                               0.67            0.78            1.47
subtract                          0.64            0.74            1.37
multiply                          1.34            8.02            9.73
divide                            0.38            0.63            0.83
dot N=256                         1.39            0.83            2.63
dot N=4096                        1.40            0.83            2.72
axpy N=4096                       1.07            0.59            1.79
matmul 32x32                      2.09            0.79            3.78
horner deg 20                     1.25            0.81            1.87
sqrt                              7.62            2.07           18.55
exp                               1.43            5.71           14.29
log                               1.43            5.20           12.90
sin                               1.47            6.39            9.64
cos                               1.47            6.47            9.69
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
within about 20% between gcc and clang, and `qd_cascade` multiply is 5x to 8x `qd` in both.

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
qd_cascade               2.091e+15         4.707e+13             3.852e+31
```

Reading the large residuals as correct decimal digits:

- `qd_cascade` `sqrt` is accurate to about 48 digits and `exp`/`log` to about 50, where `qd` delivers
  the full 63. So `qd_cascade` is both 2x to 6x slower on those functions *and* 13 to 15 digits
  short: there is no trade being made, it is worse on both axes.
- `sin`/`cos` above double-double precision lose digits on every type tested, over the sampled
  domain `[0.5, 2.0)`: `qd` holds about 41 digits, `qd_cascade` and `td_cascade` about 32, which is
  `dd` precision. The extra limbs are not carrying information through the trigonometric evaluation.
  Arguments outside `[0.5, 2.0)`, where a real argument reduction is needed, are not sampled here
  and could well be worse.
- `dd` and `dd_cascade` are both sound; `dd_cascade`'s `sqrt` is the more accurate of the two
  (2.9 vs 10.8 ulps).

The mathlib rows in the tables above are therefore **not** like-for-like for the `qd` pair, and the
`sqrt`/`exp`/`log`/`sin`/`cos` comparisons should be read with that caveat until the accuracy
defects are fixed.

**Decimal string parsing is pathologically slow** for all five types: 45 usec to parse `"1.5"` into a
`dd` and ~460 usec for a 62-digit string, versus 119 nsec for `std::stod`. The cost scales with digit
count and is the same for every multi-component type, so it lives in shared conversion code rather
than in any one number system.

These three findings are tracked separately; they are defects surfaced by the benchmark, not
benchmark results.

## What the numbers say

1. **What does the generic `floatcascade<N>` renormalization cost?** For addition and division,
   nothing: the cascade implementations match or beat the classic ones (and the arithmetic is
   bit-identical, so this is a real win). For multiplication it is expensive and it grows with N:
   1.3x at N=2 under gcc, 8.0x at N=4. `qd_cascade` multiply at 585 nsec/op against `qd`'s 73 is the
   single worst row in the suite.
2. **Does the `volatile` hardening cost throughput?** Not measurably at the level this benchmark can
   isolate. The add/subtract rows, where the hardened error-free transformations dominate, are the
   rows where the cascade types *win*.
3. **Is `td_cascade` a useful middle point?** Not on speed. Against `dd` it ranges from 0.83x
   (divide) to 9.7x (multiply), with add/subtract at 1.4x and the kernels at 1.8x to 3.8x, and its
   trigonometry is no more accurate than `dd`'s. Its value has to come from the 159-bit significand
   in code paths that need it (universal#1300 wants it for takum64), not from filling a performance
   gap.
4. **Is there a crossover length where the cascade dot product wins?** No for `dd_cascade`: it is
   1.3x to 1.6x `dd` at every length from 16 to 4096, under both compilers. Yes, trivially, for
   `qd_cascade`: it is faster than `qd` at every length (0.79x at N=16, 0.83x at N=256 and N=4096),
   because `qd`'s scalar multiply is the bottleneck in both.
5. **Can we retire classic `dd` and `qd`?** Not yet, on this evidence. `dd_cascade` is a defensible
   replacement for `dd` once the multiply is addressed. `qd_cascade` cannot replace `qd` today: its
   multiply is 8x slower and its `sqrt`/`exp`/`log` lose 13 to 15 digits.

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
