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

On a hybrid CPU, pin the run: `taskset -c 0 ./benchmark_hp_scalar`. The numbers recorded below were
taken that way.

## Methodology

**Latency, not peak throughput.** Every scalar workload is a dependent chain: the result of one
operation is the input to the next. That is the number a renormalization-heavy implementation has to
answer for. The kernels cover the other side, where independent work can overlap.

**The optimizer barrier.** Multi-component arithmetic is exactly the code a compiler likes to
delete. Every workload hands its result to `hpbench::consume()`, which stores *every component*
through a `volatile` sink. Consuming only the leading limb is not enough: converting a `qd` to a
`double` hands the compiler limb 0, and it is then free to prove the other three limbs dead. The
generated code was checked:

```
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

```
processor      : 12th Gen Intel(R) Core(TM) i7-12700K, pinned to core 0
compiler       : gcc 13.3.0, -O3 -DNDEBUG, C++20, no ISA extensions beyond baseline
measurement    : best of 3, 0.050 sec calibration window
date           : 2026-08-15
```

### Scalar operators (nsec/op, lower is better)

```
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
sink only (floor)              0.21         0.37         0.37         0.41         0.49         0.49
construct                      0.20         0.27         0.29         0.41         0.41         0.41
copy construct                 0.20         0.37         0.37         0.41         0.49         0.49
assign                         0.20         0.37         0.37         0.41         0.49         0.49
add                            0.41        23.87        15.99        35.01        62.39        48.39
subtract                       0.41        23.88        15.27        32.58        61.55        45.55
multiply                       0.82        22.19        45.24       226.27        73.11       574.25
divide                         2.86       216.63        81.00       178.85       594.33       373.60
compare (<)                    0.48         1.69         0.62         0.56         0.64         0.56
compare (==)                   0.44         0.55         0.54         0.41         0.54         0.54
convert to double              0.41         0.41         0.41         0.41         0.41         0.41
convert from double            0.21         0.29         0.27         0.37         0.41         0.41
convert to string            511.10      2665.35      3829.43      4418.12      7765.55      6228.08
convert from string          121.09    398683.47    396995.23    415779.92    428933.54    428459.30
```

The construct/copy/assign/convert rows all sit at the sink floor: a copy is 2 to 4 `movsd`
instructions and there is nothing else to measure. They are reported to establish that floor, not
because they distinguish the implementations.

### Kernels (nsec per elementary multiply-add)

```
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
dot N=16                       0.10        39.35        50.00        76.95       136.60       108.51
dot N=256                      0.31        29.34        46.18        83.85       137.99       117.23
dot N=4096                     0.40        29.76        46.36        86.75       136.65       115.53
axpy N=4096                    0.13        37.59        41.32        65.91       132.94        77.01
matmul 32x32                   0.24        21.07        49.67        83.41       135.57       108.52
horner deg 20                  0.32        47.49        59.36        75.37       143.72       119.40
```

The `double` column is auto-vectorized and the multi-component columns are not, which is most of the
70x to 1300x gap in the `dd/double` and `qd/double` ratios. That gap is a property of the format, not
of these implementations.

### Mathlib (nsec/op, includes one accumulate; subtract the `accumulate only` row)

```
operation                    double           dd   dd_cascade   td_cascade           qd   qd_cascade
----------------------------------------------------------------------------------------------------
accumulate only                0.41        23.95        30.10        47.08        71.89        60.89
sqrt                           1.23        42.74       317.06       771.51      1123.68      2365.06
exp                            3.48       917.13      1309.93     13096.72      4178.94     24748.28
log                            3.43      1956.90      2794.83     25604.30     13183.75     70841.90
sin                            3.80       951.12      1458.71      9530.45      3533.13     23534.36
cos                            3.63       959.50      1465.53      9631.98      3532.00     23810.52
```

### Normalized cost, cascade relative to classic (gcc)

Ratio of nsec/op. 1.00 is parity, above 1.00 means the cascade implementation is slower.

```
operation                dd_cascade/dd   qd_cascade/qd   td_cascade/dd
----------------------------------------------------------------------
add                               0.67            0.78            1.47
subtract                          0.64            0.74            1.36
multiply                          2.04            7.85           10.20
divide                            0.37            0.63            0.83
dot N=256                         1.57            0.85            2.86
dot N=4096                        1.56            0.85            2.91
axpy N=4096                       1.10            0.58            1.75
matmul 32x32                      2.36            0.80            3.96
horner deg 20                     1.25            0.83            1.59
sqrt                              7.42            2.10           18.05
exp                               1.43            5.92           14.28
log                               1.43            5.37           13.08
sin                               1.53            6.66           10.02
cos                               1.53            6.74           10.04
```

## Compiler sensitivity

The scalar `add` and `multiply` comparisons are not a property of the code alone: they **flip
direction** between gcc and clang. Both compilers, same source, same machine, `dd_cascade/dd`:

| operation | gcc 13.3 | clang 18.1 |
|---|---|---|
| add | **0.67** (cascade faster) | **1.51** (cascade slower) |
| subtract | **0.64** | **1.46** |
| multiply | **2.04** (cascade slower) | **0.81** (cascade faster) |
| divide | 0.37 | 0.40 |

The absolute numbers behind the flip: gcc compiles the `dd_cascade` addition to 16.0 nsec and clang
to 33.8; gcc compiles the `dd_cascade` multiplication to 45.2 nsec and clang to 23.2. The classic
`dd` is stable across both (23.9 / 22.6 add, 22.2 / 31.2 multiply). Nothing that reads only one
compiler's numbers should be trusted on these two rows.

Everything else is stable in direction across compilers: the kernel and mathlib ratios agree to
within about 20% between gcc and clang, and `qd_cascade` multiply is 6x to 8x `qd` in both.

### AVX2

`-mavx2 -mfma` (`cmake -DUNIVERSAL_USE_AVX2=ON`) changes little, with one exception: gcc's
`dd_cascade` addition regresses from 16.0 to 29.3 nsec/op, which erases the only large win the
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
`qd_cascade`. Every arithmetic and kernel row above is a like-for-like comparison.

**The mathlib is not.** Self-consistency residuals, in ulps of each type's own significand:

```
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
- `sin`/`cos` above double-double precision are inaccurate everywhere: `qd` holds about 41 digits,
  `qd_cascade` and `td_cascade` about 32, which is `dd` precision. The extra limbs are not carrying
  information through the trigonometric argument reduction.
- `dd` and `dd_cascade` are both sound; `dd_cascade`'s `sqrt` is the more accurate of the two
  (2.9 vs 10.8 ulps).

The mathlib rows in the tables above are therefore **not** like-for-like for the `qd` pair, and the
`sqrt`/`exp`/`log`/`sin`/`cos` comparisons should be read with that caveat until the accuracy
defects are fixed.

**Decimal string parsing is pathologically slow** for all five types: 45 usec to parse `"1.5"` into a
`dd` and 400 usec for a 62-digit string, versus 121 nsec for `std::stod`. The cost scales with digit
count and is the same for every multi-component type, so it lives in shared conversion code rather
than in any one number system.

These three findings are tracked separately; they are defects surfaced by the benchmark, not
benchmark results.

## What the numbers say

1. **What does the generic `floatcascade<N>` renormalization cost?** For addition and division,
   nothing: the cascade implementations match or beat the classic ones (and the arithmetic is
   bit-identical, so this is a real win). For multiplication it is expensive and it grows with N:
   2.0x at N=2 under gcc, 7.9x at N=4. `qd_cascade` multiply at 574 nsec/op against `qd`'s 73 is the
   single worst row in the suite.
2. **Does the `volatile` hardening cost throughput?** Not measurably at the level this benchmark can
   isolate. The add/subtract rows, where the hardened error-free transformations dominate, are the
   rows where the cascade types *win*.
3. **Is `td_cascade` a useful middle point?** Not on speed. It costs 1.4x to 4x `dd` on arithmetic
   and kernels, and its trigonometry is no more accurate than `dd`'s. Its value has to come from the
   159-bit significand in code paths that need it (universal#1300 wants it for takum64), not from
   filling a performance gap.
4. **Is there a crossover length where the cascade dot product wins?** No for `dd_cascade`: it is
   1.3x to 1.6x `dd` at every length from 16 to 4096. Yes, trivially, for `qd_cascade`: it is faster
   than `qd` at every length (0.85x), because `qd`'s scalar multiply is the bottleneck in both.
5. **Can we retire classic `dd` and `qd`?** Not yet, on this evidence. `dd_cascade` is a defensible
   replacement for `dd` once the multiply is addressed. `qd_cascade` cannot replace `qd` today: its
   multiply is 8x slower and its `sqrt`/`exp`/`log` lose 13 to 15 digits.

## Caveats

- Single machine, single microarchitecture. The `add`/`multiply` compiler flip is a warning that
  these rows are sensitive to code generation; re-measure before generalizing to another platform.
- The `double` column is auto-vectorized in the kernels and the multi-component columns are not. The
  `dd/double` and `qd/double` ratios are therefore format-vs-format, not implementation-vs-
  implementation.
- The mathlib rows for the `qd` pair compare implementations of *different accuracy* (see above).
- Hybrid-core CPUs need pinning; unpinned runs on the i7-12700K varied by up to 15% on the cheaper
  rows.
