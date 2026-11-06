# Development Session: faithful's Math Library, and the Stubs Behind It

**Date:** 2026-11-05 to 2026-11-06
**Status:** Complete -- 17 PRs merged, 18 issues closed
**Focus:** Build every number system without exceptions; fix the defects that work surfaced;
take `faithful<Real>` from stubbed comparisons and limits to a compensated elementary-function
library.
**Issues closed:** #1925, #1929, #1930, #1931, #1932, #1933, #1936, #1940, #1941, #1942, #1944,
#1949, #1950, #1952, #1954, #1955, #1956, #1957
**PRs merged:** #1928, #1934, #1935, #1937, #1938, #1939, #1943, #1945, #1946, #1947, #1948,
#1951, #1953, #1958, #1959, #1960, #1961

## Session Overview

The day began with `-fno-exceptions` (#1925): firmware targets could not include the umbrella
header. To verify it, `integration/noexceptions` had to build all 42 number systems and
exercise their arithmetic, comparisons, conversions and stream I/O. That harness turned up five
unrelated defects: rational, fixpnt and ereal stream input, faithful's unary minus, and dbns
conversion. Each was filed and fixed. Following faithful's stubs led to four more issues, then to
an epic of four sub-issues for its elementary functions. The user approved the epic's split and
left the last four sub-issues to run overnight.

The through-line is that **almost nothing here failed loudly**. Each defect compiled and passed
its test. Some returned a plausible constant: `abs()` returned 0, `==` was always false, and
`digits` was 3333333. Some tests checked nothing: dbns `conversion.cpp` printed a table and
returned 0, and `tests/issue_1327_divide_by_infinity.cpp` was never compiled. Others were
headers no build ever included: faithful's `attributes.hpp`, `manipulators.hpp` and
`mathlib.hpp`, and dbns's `NATIVE_DBNS_ARITHMETIC` path.

## Key Decisions

- **No exceptions is detected, not configured.** `UNIVERSAL_HAS_EXCEPTIONS` comes from
  `__cpp_exceptions`, `__EXCEPTIONS` or `_CPPUNWIND`. A throw switch set to 1 without exceptions
  is an `#error`, not a silent downgrade. `exceptions_check.hpp` has no include guard on
  purpose, so a switch defined after the first include is still caught.
- **faithful follows the multi-double conventions.** `faithful<Real>` is the same two-part
  expansion as dd. Its limits use #1882's choices with `Real` for double:
  - `min()` is the full-precision floor;
  - `max()` is `(MAX, MAX*2^-(digits+1))`;
  - `minpos` is `(denorm_min, 0)`.
  CodeRabbit suggested `maxpos = (max, 0)`. We kept the dd form and CodeRabbit withdrew.
- **Delivery of #1952.** The user chose sub-issues with one PR each (#1954 to #1957), on top of
  the minimum-plus-sqrt scope picked for #1950.
- **Native algorithms only.** As for the multi-double types (#1724), no function goes through
  `Real` and corrects afterwards. Every test includes a mutation check: replacing the function
  with its `Real` evaluation must fail it.

## Implementation Details

| PR | Functions | The non-obvious part |
|---|---|---|
| #1953 | classify, fmin/fmax, sqrt | A pair such as (1, -1) sums to +0. `signbit` follows the head only when the head is zero. |
| #1958 | floor, ceil, trunc, round, fmod | `fmod` reduces in an exact Shewchuk expansion. The quotient can be 2^230 for float, so the divisor is scaled by 2^k instead of growing q. |
| #1959 | exp, exp2, exp10, expm1, log, log2, log10, log1p | `x - m ln2` and `x ln10 - m ln2` are formed exactly. A pair product `x * ln10` was 78 units off at x = 30. |
| #1960 | sin, cos, tan, asin, acos, atan, atan2 | Reuses the multi-double Payne-Hanek reduction (#1723) through a small `reduction_real<Real>` adapter. |
| #1961 | sinh ... atanh, pow, cbrt, hypot | sinh switches form at `log(max) - 1`, not `log(max)`: float's rounded `log(max)` lies above the true value. `pow` reduces `w = y log b` exactly, so `pow(0.7f, 270)` stays finite. |

The constants (ln 2, ln 10, pi/2) live in `math/constants.hpp` as four doubles each. They are
split exactly into `Real` parts, so a float host gets them to about 100 bits.

## Challenges & Solutions

- **long double is not a reference.** The float-host tests first used `long double` as the
  reference. That passed on Linux and failed the full CI tier on MSVC and macOS ARM64, where
  `long double` is 53 bits and cannot hold a pair such as (1, 2^-40). All float references are
  now `dd`.
- **The same assumption almost reached the library.** The trig reduction carries double limbs. A
  `static_assert` excluding `faithful<long double>` would have broken where `long double` has
  double's format. The constraint `trig_host` therefore tests the property itself: every value
  converts to double exactly. That admits float, double and the 53-bit `long double`. The test
  asserts `has_sin<faithful<long double>> == ldFitsDouble`.
- **Error bounds from a budget, not a measurement.** `pow`'s first bounds were fitted to gcc's
  measurements. They failed on MSVC and then on macOS, whose random samples differ. The bound is
  now per sample and per path, derived from the error budget:
  - the exp/log path allows `5 + 4 g` with `g = |y log x|` (log(b) contributes 3 units, the
    product about 1);
  - binary powering allows `5 + |y|`.
  The measured slopes, 2.7 (libstdc++), 3.2 (libc++) and 1.0, sit inside it.
- **A NaN can hide in a worst-error tracker.** `max(worst, err)` with a NaN `err` keeps the old
  worst. CodeRabbit caught it in the trig test. The tracking now keeps a NaN once seen.
- **Infinity through renormalization.** `twoSum(inf, x)` has a NaN roundoff, which reached the
  error part and turned `faithful(inf) + 1` into NaN. A non-finite value now drops its error.
- **MinGW `inline thread_local`.** Linking efloat from two translation units failed on MinGW,
  because the TLS init was emitted as a strong symbol. Only `integration/noexceptions`, which
  builds on every platform, exposed it.

## Testing & Validation

Each faithful test measures over 20000 to 100000 arguments per range, against `dd` for
`faithful<float>` and `qd` for `faithful<double>`. The bound is 5 units of `epsilon()`, and the
worst measured error is 3.9. Evaluating through `Real` misses by 10^7 to 10^51 units. Every PR
was built and run with gcc and clang locally, iterated against CodeRabbit as a draft, and merged
after the full 11-platform tier passed.

## Next Steps

- `CHANGELOG.md` stops at v5.1.0, but the tree is tagged v5.14.0, and release notes come from
  git-cliff. This session's entry sits under `[Unreleased]`. The gap from v5.2.0 to v5.14.0 needs
  a decision: backfill it, or point to the GitHub releases.
- Codacy flagged every faithful PR, and its findings were never read: the Codacy CLI here has no
  token.
- The daily QA sweep opened #1962 (ranges: areal, interval, sorn).
- faithful's `pow` accuracy at large `|y log x|` is bounded by pair precision. A tighter `pow`
  needs a triple-length `log`, which dd lacks as well.

## Appendix

- Headers: `include/sw/universal/number/faithful/math/functions/{classify,minmax,sqrt,truncate,
  fractional,exponent,logarithm,trigonometry,hyperbolic,pow,cbrt,hypot}.hpp`,
  `math/constants.hpp`.
- Tests: `numeric/faithful/{compensated_arithmetic,logic_and_modifiers,limits_and_attributes,
  mathlib,truncation,exponential,math_exceptions,trigonometry,hyperbolic_and_power}.cpp`.
- No-exceptions: `utility/exceptions.hpp`, `utility/exceptions_check.hpp`,
  `integration/noexceptions/`, `tools/scripts/check-ungated-throws.py`.
