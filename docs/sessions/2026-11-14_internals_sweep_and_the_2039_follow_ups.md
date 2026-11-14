# Development Session: The Internals Sweep, v5.16.0, and the #2039 Follow-ups

**Date:** 2026-11-13 to 2026-11-14
**Status:** Complete -- 7 PRs merged, the #2029 sweep closed, v5.16.0 released, two of three #2039
tracking issues closed
**Focus:** Triage QA sweep #2029 (internal building blocks) into three tracking issues and resolve
them; recover the v5.16.0 release whose tag was pushed without a prepare-release PR; triage QA sweep
#2039 (quire, native helpers, recent commits) into three tracking issues and resolve them.
**Issues closed:** #2029, #2052, #2053, #2054, #2058, #2060
**PRs merged:** #2055, #2056, #2057, #2061, #2062, #2063, #2064
**Left open:** #2059 (the cross-type `promote_t` decision; everything else in it merged with #2063),
#2039 (closes when #2059 does)

## Session Overview

Both sweeps were split under the rule recorded at the start of the session: a QA sweep produces many
findings, one CI cycle costs 1-2 hours, so a sweep becomes at most three tracking issues, each one PR.

| sweep | tracking issue | PR |
|---|---|---|
| #2029 internals | #2052 limb blocks: blocktriple, blockbinary, blockfraction, blocksignificand | #2055 |
| | #2054 multi-component and elastic: ereal remainder, elreal range, sorn saturation, floatcascade division, expansion_ops, native quire assignment | #2056 |
| | #2053 legacy and digit internals: value, bitblock, abstract triple, blockdecimal, blockdigit; gfp and f2s archived | #2057 |
| #2039 | #2060 follow-ups: fixpnt CORDIC hypot, sorn division, complex atanh, dd/qd zero signs | #2062 |
| | #2058 quire: asymmetric range checks, convert_to rounding, numeric_limits | #2064 |
| | #2059 native and utility helpers (all but the promotion rule) | #2063 |

Between the two sweeps, v5.16.0 was tagged on a9d6a659 with no prepare-release PR. release.yml stopped
at its CHANGELOG check (#1967), before the GitHub Release and the Docker image. PR #2061 renamed
`[Unreleased]` to `[5.16.0]`, added the three internals PRs and their behaviour changes, and the tag was
moved to its merge commit as an annotated tag. The rerun published the release and the image.

## Key Decisions

- **At most three tracking issues per QA sweep** (user). Recorded in the qa-triage skill and in memory.
- **gfp and f2s are archived, not fixed** (user, #2053). Their Grisu paths were stubs, the two headers
  could not be included together, and their tests asserted nothing.
- **The promotion rule stays open** (#2059). Mixed-type operators pick the result type by fraction bits
  alone and bridge through double. The options are recorded on the issue: promote by range and
  precision with a deterministic tie-break, a wider bridge than double past 53 bits, or a compile error
  for incomparable types.

## Implementation Details

- **blocktriple** (#2055): `round<srcbits>()` rewritten to round to nearest even; a MUL or DIV operand
  built from a native value now has the layout `mul()`/`div()` consume; comparisons work on the
  leading-one binade, so an unnormalized product compares correctly. The quire had dodged the rounding
  defect by using `blocktriple<22>`/`<51>`, which dropped the last bit of every float and double; #2056
  removed the workaround.
- **ereal** (#2056): fmod and remainder of multi-limb operands are exact, computed on the raw expansions
  (q * y needs twice y's limbs), with q backed off when q * b overflows.
- **value** (#2057): every sum was wrong (`abits` fbits + 5 against the module's fbits + 4 layout); IEEE
  special values, sticky division and nearest-even `round_to` throughout.
- **fixpnt CORDIC** (#2062): vectoring ran `rbits + g` iterations, but the magnitude's relative error after
  n iterations is about 2^-(2n-1). `fixpnt<32,0>` ran 5, and hypot(3e8, 4e8) was 499944976. Vectoring now
  sizes from `B = max(rbits, (nbits + 4) / 2)`; the worst result over every tested format is 0.62 ulp.
- **sorn division** (#2062): it multiplied by an outward-rounded reciprocal, rounding twice. It now takes
  the four corner quotients, each rounded outward once, as multiplication does.
- **complex** (#2062): `atanh` for x < 0 is `-atanh(-z)`, which removes a log1p cancellation near -1;
  `is_high_precision<dd|qd>` moved into the dd/qd mathlib, so `complex<dd>` no longer falls back to 53 bits
  when one optional header is missing.
- **quire** (#2064): operands are checked against the accumulator's real ends (bit 0 and bit qbits - 1),
  not +-half_range; `convert_to` rounds to nearest even from 64 leading bits and a sticky bit, with
  subnormals, overflow to inf and integral saturation; `numeric_limits<quire>` is included and corrected.
- **native** (#2063): `find_msb(long)` on LP64, NaN classification of every payload (and long double),
  `ulp` as a magnitude, `fastipow` without signed overflow, `setbit` bounds, `to_binary`/`to_hex` past
  64 bits, and `universal_cast` keeping -0.

## Challenges & Solutions

- **A workaround hid a building-block bug.** The quire's `blocktriple<22>`/`<51>` comment dodged an
  "edge case" in `round<>`, and disabled `MANUAL_TESTING` suites never ran the cases that would have shown
  it. Both patterns are now in memory as things to grep for.
- **A release with no CHANGELOG section.** Recovered by a prepare-release PR and a moved tag, as the
  #1967 check intends. git-cliff locally hit the GitHub API rate limit, so the entries were written by
  hand in the file's format.
- **`gh pr checks` here has no `--json`.** A wait loop that parsed it never broke and waited its full
  timeout. The loops now parse the plain tab-separated output.
- **Builds from other sessions.** The build-safety hook blocks a second build, so the session queued its
  compiles behind a wait-for-idle script and merged #2062 without switching the shared checkout's branch.
- **CI_LITE builds neither `static/quire` nor `static/native`.** Those suites only run in the full tier,
  so they were built locally with their own options; that caught a quire fixture asserting the old
  truncating `convert_to`.
- **Review findings worth keeping** (CodeRabbit): a closed infinite divisor bound attains `a / inf = 0`;
  NaN residuals must fail a tolerance check (`!(|e| <= tol)`); `make_unsigned` rejects `bool`; a quire
  `min()` of 2^-2148 cannot be checked through a double.

## Testing & Validation

- **Each fix:** a test that fails against main's headers, checked in a worktree of origin/main, then run
  under gcc and clang.
- **Mutation checks** for test helpers with no defect behind them: flipping the sign test of
  `ArealReference` gave 1384 failures in the new oracle test, and a sign-flipped `QuotientError` failed
  every case.
- **Full builds:** CI_LITE plus the suites each PR touches, gcc then clang, one at a time at -j4.
- **CI:** every PR iterated on CodeRabbit as a draft and merged after the full tier. Codacy and coveralls
  are advisory.

## Next Steps

- #2059: the user's decision on cross-type promotion; then the mixed-arithmetic tests it calls for, and
  #2039 closes with a map of its tracking issues to PRs.
- Deferred, not filed: limb-wise quire accumulation, the extra division in the complex `clog`/`abs`
  helpers, CORDIC's duplicate shifted copies, and the bit-serial internals noted in #2029.
- **Release:** `[Unreleased]` holds #2062-#2064.

## Appendix

- Sweep issues and map comments: #2029, #2039.
- New tests: `static/quire/api/range_and_conversion.cpp`, `range_exceptions.cpp`;
  `static/native/float/special_patterns.cpp`, `static/native/integer/edges.cpp`;
  `static/range/areal/arithmetic/reference_oracle.cpp`; `static/complex/math/high_precision_path.cpp`.
