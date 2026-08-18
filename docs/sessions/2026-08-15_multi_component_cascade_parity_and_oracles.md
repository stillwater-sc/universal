# Development Session: Multi-component Cascade Parity -- closing what the #1315 benchmark exposed

**Date:** 2026-08-15 (into 2026-08-18)
**Branches:** per-issue branches off `main`, all merged: PRs #1316, #1321, #1323, #1325, #1328, #1330, #1333, #1335, #1336, #1338, #1339, #1341, #1343, #1344.
**Focus:** Benchmark the multi-component families against each other (`dd`/`qd` vs `dd_cascade`/`td_cascade`/`qd_cascade`), then fix everything the comparison exposed.
**Status:** Complete -- issues #1315, #1317, #1318, #1319, #1322, #1324, #1326, #1327, #1330, #1331, #1332, #1340 closed. Releases v4.8.4 through v4.8.8 cut. One follow-up open (#1342).

## Session Overview

The session began with a single question -- *is the cascade implementation as fast and as accurate as
the direct one?* -- and a benchmark to answer it. The benchmark found that it was neither, and the
rest of the session was spent closing the gap. Nine defects: six the benchmark reported directly
(#1317, #1319, #1322, #1326, #1327, #1318), and three found by the work it prompted -- #1324 while
porting the multiplication schedule to the third width, #1332 while reformulating `sqrt` for #1331,
and #1330 while writing the assessment.

The arc in one line each:

| # | What was wrong | Result |
|---|---|---|
| #1317 | `add_cascades<4>` emitted an overlapping expansion; renormalization dropped the fourth limb | 15 digits recovered |
| #1322 / #1324 | `qd_cascade`/`td_cascade` multiplication improvised a sorted schedule | 8x -> 1.3x of `qd`, accuracy matched |
| #1326 | division computed N quotient digits where it needs N+1 | all three widths |
| #1330 | the generic `add_cascades<N>`/`multiply_cascades<N>` still carried the old design | now a compile error naming the assessment |
| #1331 | `sqrt` was a single formulation with no measured alternative | three-way selector, most accurate by default |
| #1319 | decimal parsing cost 45-460 usec per value | 12-58x faster |
| #1318 | trigonometry capped at double-double accuracy in every type above `dd` | full format precision, six functions |
| #1332 | `sqrt` unbounded at both ends of the range | `qd` worst case 60 -> 214 bits of its 212 |
| #1340 | `add_cascades<4>` bubble-sorted operands that arrive sorted | gcc -27%, bit-identical |

`qd_cascade` came into the session 6-8x slower than `qd` and 13-15 decimal digits less accurate. It
leaves at 1.1-1.9x with matching or better accuracy, and its addition is now faster than `qd`'s under
gcc.

## The move that kept working

Five of the nine fixes were the same move: **where the cascade improvises, adopt the direct family's
proven schedule and express it with the framework's hardened primitives.**

- #1322/#1324: the cascade sorted its partial products by magnitude. It does not need to -- in
  `a[i]*b[j]` the term contributes at order `eps^(i+j)`, so the products emerge in decreasing
  significance *by construction*. Adopting `qd_mul`'s schedule removed the sort and matched `qd`'s
  accuracy.
- #1326: `qd`'s long division computes N+1 quotient digits and closes with an (N+1)-term
  renormalization. The cascade computed N.
- #1318: the cascade trigonometry *was* the double-double implementation with zero-padded constants.
  Porting `qd`'s pi/1024 schedule fixed the reduction, which was one of three causes -- the other two
  (a `qd_eps` holding the double-double unit roundoff, and an `atan2` taking one Newton step) are the
  same mistake in different places: values sized for a double-double left in a wider type.

Issue #1332 is the same rule pointing the other way: the cascades had the correct `sqrt` argument
scaling from #1331 and the *direct* types did not, so the port went cascade -> direct.

Issue #1340 is where the rule stopped applying, and the assessment's prediction was wrong (below).

## Testing: three suites that could see what the old ones could not

The most durable finding of the session is not any single fix. It is that **three of these defects
survived for years behind test suites that were structurally incapable of detecting them**:

- trigonometry was compared against `std::sin`/`std::cos` with a `1e-10` tolerance -- that cannot see
  past double precision, so a `qd` delivering 43 of its 63 digits passed;
- `sqrt` was tested by chaining it from 2.0 and 0.5, never approaching either end of the exponent
  range, so `sqrt(maxpos)` returning NaN passed;
- decimal parsing had no accuracy oracle at all.

Each fix now carries a suite whose verdict is decided in exact integer arithmetic -- against a
reference **wider than the format under test**, or, where no exact reference exists, against a
residual that is itself exact:

| suite | reference | verdict decided by |
|---|---|---|
| `static/highprecision/qd_cascade/math/trigonometry_oracle.cpp` | six doubles (~318 bits) from mpmath at 200 digits | exact dyadic comparison |
| `static/highprecision/qd_cascade/math/sqrt_range_oracle.cpp` | none needed -- residual `\|r*r - a\|` | exact dyadic (dyadics are closed under multiplication) |
| `static/utility/test_decimal_to_binary_oracle.cpp` | exact integer scan of the decimal literal | exact dyadic comparison |

Two techniques worth reusing:

**Charge each argument its own condition number.** `sin(3.141592653589793)` is 1.2e-16 while the
argument is not, so one ulp of input moves the result by half of it -- no implementation holds
relative accuracy there. Rather than exempting the hard arguments by hand, the generator computes
`log2|x f'(x)/f(x)|` per argument and the suite subtracts it from the budget. That lets the test
demand *full* precision everywhere else, and it happens to cover the argument reduction too, whose
error grows with `|x|` at the same rate.

**When there is no exact reference, check the residual.** `sqrt` is irrational, but `r*r` is exact in
dyadic arithmetic, so `|r*r - a| <= 2^-budget * a` is decided in exact integers.

**Mutation-test every new oracle.** Each of the three was verified to *fail* against the
implementation it was written for: 90/97/101 cases for trigonometry, 14/29 (49/107 at level 4) for
sqrt, 77/714 for the parse width dispatch.

## Where measurement overturned the hypothesis

Four times, and worth recording because in each case the wrong answer was *plausible*:

1. **My own over-broad fix (#1317).** I "found" `td_cascade` broken by scoring a 159-bit format
   against a threshold calibrated for a 212-bit one. Compression is applied at N=4 only; N=2 and N=3
   were fine all along. **Score each width against its own ulp.**
2. **Fixing multiply made division 23% slower (#1322).** Division's initial quotient estimate is a
   one-component cascade, and the old sorted multiply skipped zero products. `multiply_cascade_by_double`
   had to land in the same change.
3. **The assessment's item 2 was wrong (#1340).** It said compression is applied *after the fact* to
   repair an expansion built without the invariant, so a better addition would not need it. Half
   right: the bubble sort was waste, but the compression **had to stay** -- the 2N -> N step needs a
   *nonadjacent* expansion, not merely a non-overlapping one, and feeding it the raw chain output
   costs a factor of three on the composite identities. The 46% was never the compression's to give
   back.
4. **My precondition defense was wrong (#1340, caught in review).** I documented "operands must be
   canonical" on the grounds that `qd::accurate_addition` merges identically. Classic `qd` and
   `dd_cascade` both handle a non-canonical operand correctly, so the cascade would have been the only
   type that did not. The guard is now a six-comparison test plus a five-comparator network.

## Performance summary

i7-12700K, `-O3`, gcc 13.3 unless noted.

| operation | before | after |
|---|---|---|
| `qd_cascade` multiply | 8x `qd` | 1.3x |
| `td_cascade` add | 9-15x `dd` | ~3x |
| `qd_cascade` add | 69.6 nsec/op | 51.0 (faster than `qd`'s 61.2) |
| parse `"1.5"` into `dd` | 40.4 usec | 2.7 |
| parse `1.7976931348623157e308` | 759.7 usec | 13.0 |
| `qd` sin/cos | 3381 nsec/op | 3904 (the terms it should have been summing) |
| `qd_cascade` atan | 13160 nsec/op | 31210 (three Newton steps instead of one) |

The two regressions are the price of correct answers, not lost ground: the old `qd` sin stopped its
series five limbs early, and the old cascade `atan2` returned a double-double result in a quad-double.

**The parse work (#1319)** was four independent costs, none inherent: a full-width bigint multiply per
digit (now nine digits per multiply), `5^|E|` by repeated multiplication (now binary exponentiation),
quotient and remainder from two separate long divisions (now one `idiv`), and every parse running at
2048 bits regardless of need (now the narrowest of 256/512/1024/2048, chosen from the digit count and
exponent, with the result copied back into the caller's width so no call site changed).

## Gotchas and process notes

**Verification**

- `ctest` reported 113/113 passing while a target *failed to build*. The build log is the real signal;
  a green ctest run over stale binaries proves nothing.
- A benchmark measured 163 nsec for an operation whose source had just been changed back -- the binary
  was stale. Caught only because the number matched the discarded experiment.
- Full-width operands matter. Comparing `dd` against `td_cascade` on single-double values proves
  nothing: the products are exactly representable. The equivalence benchmark now generates full-width
  limbs, which immediately corrected a claim of mine (addition is *not* bit-identical between `qd` and
  `qd_cascade` -- the cascade is exact where `qd` rounds, 3907/4096).
- For a pure-performance change, **bit-identity is the cleanest possible verification**: #1340 matched
  the previous implementation across 40,000 random full-width operations, which retires the accuracy
  question entirely.

**Compilers**

- gcc and clang disagree on this code, repeatedly and in both directions. #1340 is gcc -27% and clang
  +7%; four formulations of the merge landed within 2.6 nsec of each other under clang, so the shape
  is not the variable. Tracked as #1342.
- Local `node_modules` for `docs-site` had drifted (Starlight 0.34.8 vs the lockfile's 0.41.7), which
  made every sidebar section fail to build locally while CI was green. `npm ci` fixed it. Nearly
  "fixed" twelve sidebar sections for a stale toolchain.

**CI**

- Regression-level constants defined in `main()` but used only under `REGRESSION_LEVEL_2`/`_3` warn as
  unused at level 1 -- gcc as `-Wunused-variable`, clang as `-Wunused-const-variable` if you move them
  to namespace scope. Define them inside the block that uses them.
- Codacy's Prospector profile enables **both** pydocstyle D212 and D213, which are mutually exclusive;
  no multi-line docstring can satisfy it. One-line summaries with the prose as comments sidestep both.
- cppcheck's `unusedStructMember` does not see members read inside a function template. Restructuring
  the reference tables to be indexed by function removed six findings *and* shortened the verifier.
- The PR-title scope allow-list (`.github/workflows/conventional-commits.yml`) did not include
  `floatcascade`, though its sibling internals (`blockbinary`, `blocktriple`, ...) are all listed.
  Added.
- `gh pr checks --json` is not supported by the installed `gh`; a monitor built on it silently watched
  nothing. `gh pr checks --watch` is the reliable form.

## Follow-on work

- **#1342** -- the clang regression in `add_cascades<4>`. Full measurement and the four formulations
  already tried are on the issue. Start with what clang emits for the bubble sort it replaces.
- **#1318 left the shared items untouched**: `square` is 4.0-4.2 ulps in both `dd` and `qd` (where
  multiplication is 0.11-0.46), and `log` is 15.3 ulps in both `qd` and `qd_cascade`. These are now the
  largest accuracy gaps in either family and they cap `exp`.
- The assessment's framework items are all closed. What remains is number-system work.

## References

- Assessment: `docs/assessments/multi-component-cascade-vs-direct.md`, published at
  [Assessments / Multi-component Systems](https://stillwater-sc.github.io/universal/assessments/multi-component-cascade-vs-direct/)
- Benchmarks: `benchmark/performance/arithmetic/highprecision/` (`scalar`, `kernels`, `mathlib`,
  `equivalence`) and its `README.md`
- Shewchuk, *Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates* --
  `fast_expansion_sum` (Fig. 7) and COMPRESS (Fig. 22)
- Hida, Li and Bailey, *Library for Double-Double and Quad-Double Arithmetic* -- the multiplication
  schedule, the pi/1024 trigonometric reduction, and the `renorm` chains
- Generator: `tools/generators/cascade_trig_gen.py` (mpmath at 200 digits)

## Appendix: reproducing the measurements

The timings above are gcc 13.3, `-O3`, on an i7-12700K. Use a fresh build directory: an existing
cache keeps whatever `CMAKE_BUILD_TYPE` and compiler it was configured with.

```bash
mkdir build_bench && cd build_bench
CXX=g++-13 cmake -DCMAKE_BUILD_TYPE=Release \
      -DUNIVERSAL_BUILD_BENCHMARK_PERFORMANCE=ON -DUNIVERSAL_BUILD_NUMBER_STATICS=ON ..
make -j4 benchmark_hp_scalar benchmark_hp_kernels benchmark_hp_mathlib benchmark_hp_equivalence
./benchmark/performance/arithmetic/benchmark_hp_equivalence   # the identity table
./benchmark/performance/arithmetic/benchmark_hp_scalar        # per-operation timings
```

The oracle suites build with the number systems they cover:

```bash
CXX=g++-13 cmake -DCMAKE_BUILD_TYPE=Release -DUNIVERSAL_BUILD_NUMBER_DOUBLE_DOUBLE=ON -DUNIVERSAL_BUILD_NUMBER_QUAD_DOUBLE=ON \
      -DUNIVERSAL_BUILD_NUMBER_DD_CASCADE=ON -DUNIVERSAL_BUILD_NUMBER_TD_CASCADE=ON \
      -DUNIVERSAL_BUILD_NUMBER_QD_CASCADE=ON -DUNIVERSAL_BUILD_UTILITY=ON ..
make -j4 && ctest -j4
```
