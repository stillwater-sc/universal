# Development Session: The Elastic Reals Sweep, and a CORDIC Family

**Date:** 2026-11-08 to 2026-11-11
**Status:** Complete -- 21 PRs merged, the #1999 sweep closed, five CORDIC issues closed
**Focus:** Close out the range and elastic-integer sweep leftovers; triage the elastic reals QA
sweep #1999 into four tracking issues and resolve them; fix the follow-ups it surfaced (complex
in T, dd/qd signed zero, quire infinities, sorn outward rounding); assess elreal transcendental
performance without changing code; build CORDIC engines for fixpnt, dfixpnt and lns.
**Issues closed:** #1342, #1630, #1986, #1999, #2003-#2012, #2015, #2016, #2017, #2019, #2021,
#2022, #2023, #2026, #2032, #2033, #2040, #2041, #2042, #2046
**PRs merged:** #1998, #2000, #2002, #2013, #2018, #2020, #2024, #2025, #2027, #2030, #2031,
#2034, #2035, #2036, #2037, #2038, #2043, #2044, #2045, #2048 (plus dependabot #2028)
**Filed and left open:** #2001 (valid projective-circle model and the Gustafson design
questions), #2014 (elreal transcendentals take 7-20 s at 32 blocks; assessment posted)

## Session Overview

The session had two halves.

The first half finished the QA work. #1986 gave sorn a `numeric_limits` derived from its
lattice (#2000). #1630 was closed and its open design questions moved to #2001. #1342 turned out
to be resolved by clang 21, which no longer loses on the `add_cascades<4>` merge; that was
recorded in the docs (#2002). The elastic reals sweep #1999 was triaged into ten tracking
issues. Because one CI cycle costs about an hour, the user asked for them to be consolidated
into four:

| tracking issue | absorbed | PR |
|---|---|---|
| #2003 elreal zero and sign | #2004 elreal math | #2013 |
| #2005 ereal and efloat core | #2007 limits/parse/print, #2009 efloat extremes | #2020 |
| #2006 ereal math | #2008 tests through double | #2018 |
| #2010 follow-ups | #2011 einteger, #2012 areal/faithful/valid | #2024 |

The sweep's fixes exposed deeper problems, each filed and fixed in turn:

- ereal large-argument trig and the special functions were double-precision (#2016, #2017 ->
  #2027).
- `complex<T>` computed abs, arg and the transcendentals in double for every T (#2026 -> #2030,
  with a branch-cut and signed-zero document in #2031).
- dd and qd dropped the sign of zero, atan2 squared its arguments, and stale `BUILD_COMPLEX`
  gates hid a suite and a broken application (#2032, #2033 -> #2034).
- The generalized quire collapsed infinities into NaN (#2019 -> #2025).
- sorn-with-sorn operators rounded their bounds to nearest, not outward (#2021 -> #2037).
- areal's level 2-4 tests used inexact references (#2022 -> #2035), and the efloat allocation
  counter could be folded away by gcc 15 (#2023 -> #2036).

#2014 asked why elreal's transcendentals take seconds. The user framed it as thesis-level
algorithm work: investigate, measure and report, no code. The assessment is a comment on #2014.

The second half was CORDIC. It started with a question: does Universal have CORDIC algorithms
for trigonometry? Only lns had one (`CORDICAddSub`, #783), and it iterates in double. That
became five issues, each resolved in one PR:

| issue | engine | PR |
|---|---|---|
| #2015 | fixpnt circular: sin, cos, atan, atan2, hypot | #2038 |
| #2040 | fixpnt linear: multiply and divide, on a unified core | #2043 |
| #2041 | fixpnt hyperbolic: exp, log, sqrt, sinh, cosh, tanh, atanh, and tan | #2044 |
| #2042 | dfixpnt decimal: sin, cos, atan, atan2, hypot | #2045 |
| #2046 | lns add/sub on the fixpnt hyperbolic datapath | #2048 |

`docs/algorithmic-details/cordic.md` records the why, what, how, when and history of the
algorithm, and is published on the docs site.

## Key Decisions

- **CORDIC is a separate engine, not the default** (user, #2015, option a). fixpnt `sin`/`cos`
  keep the double shim for software experiments. Hardware experiments call `cordic::` functions
  with an explicit guard width and iteration count.
- **Unbounded hyperbolic results saturate in both arithmetic modes** (user, #2041). An exact
  Modulo wrap of exp or sinh would need a datapath as wide as the result.
- **Decimal CORDIC is fixed-count, constant-gain** (user, #2042): one +-pi/4 rotation, then nine
  +-atan(10^-j) rotations per digit position.
- **The lns datapath is a new sibling policy** (user, #2046). `CORDICAddSub` stays the double
  model, unchanged, so there is no behaviour change. The new `CORDICDatapathAddSub` is opt-in.
- **The cancellation band runs on a wider bus** (user, #2046). For d in (-1, 0), sb_sub uses
  2 rbits + g + 2 fraction bits, so every path stays CORDIC: no direct-evaluation fallback.
- **elreal has one, unsigned zero** (user, #2003). **fmod(x, 0) is NaN** (user, #2006).
- **Performance assessment only** for #2014 (user). The proposed layers (engineering, argument
  reduction, a big-integer substrate) and the default-precision policy are the user's decision.

## Implementation Details

- **CORDIC core** (`fixpnt/math/cordic.hpp`): `detail::step<m>` serves circular, linear and
  hyperbolic coordinates. The datapath is Modulo fixpnt with `g = clog2(rbits + 8) + 2` guard
  bits by default, and constants come from fixpnt series 16 bits wider than the datapath, with no
  double. The circular engine reaches a worst 0.62 ulp up to `fixpnt<128,120>`.
- **Vectoring inputs** are widened to nbits + 2 bits. `atan` in `fixpnt<8,8>` needs 1 = 2^nbits
  (CodeRabbit caught the nbits + 1 version).
- **Decimal CORDIC** (`dfixpnt/math/cordic.hpp`) runs on a byte-per-digit `decimal_register`,
  bit-identical to a BCD dfixpnt datapath and 12x faster, because blockdecimal reads BCD digits
  bit by bit.
- **lns** (`lns/lns_addsub_cordic_datapath.hpp`): t = d ln2, v = cordic::exp(t), w = 1 +- v,
  L = cordic::log(w), sb = L / ln2, on a fixpnt bus of rbits + g fraction bits. The cutoff is
  dmax = rbits + g + 1. A saturating subtractor keeps truncated units finite, and the error-bound
  trait covers full and truncated iteration counts. The worst error is 0.044 lns units from
  lns<8,4> to lns<32,24>, against 3-4 for the double model, and the results are bit-identical to
  DirectEvaluation from lns<16,8> up.
- **ereal math** (#2018, #2027): C99 Annex F special values, an exact chunked fmod, Karp sqrt
  with exact perfect squares, Payne-Hanek reduction, and erf/erfc/tgamma/lgamma in ereal
  arithmetic. About 105 tests that compared through double now compare to the type's precision.

## Challenges & Solutions

- **A CI outage that looked like a failure** (#2048). After a fully green commit, every fast job
  and the required Documentation `build` failed for about three hours. The cause was the runners
  failing to resolve `azure.archive.ubuntu.com` in apt steps; nothing compiled. A loop re-ran the
  failed jobs every 30 minutes until the mirror came back, and the PR merged without changing a
  workflow or bypassing a red required check.
- **An error bound that missed the discarded tail** (#2048, CodeRabbit). The first cutoff,
  rbits + 2, dropped corrections larger than the bound. It moved to rbits + g + 1, and the sweeps
  were extended past it.
- **A test that dropped NaN** (#2048). `max` with a NaN keeps the other operand, so a NaN
  correction could pass. `worse()` now counts NaN as infinite error.
- **Mutation checks.** Each lns guard (the old cutoff, no saturation clamp, a narrow-bus band, an
  injected NaN) was reverted in a copy of `include/` and the test confirmed to fail. One mutation
  survived on purpose: the engine's default iteration counts overshoot (30 and 41 at lns<16,8>
  against 24 needed), so removing three iterations changes nothing.
- **Through-double tests hid 53-bit results** (#2008). An ereal function that computed in double
  passed every test, because the tests compared in double.

## Testing & Validation

- **CORDIC:** each engine is measured against a high-precision reference (qd, exact integer<>,
  or long double), with error tables over guard width and iteration count. The lns policy is
  checked exhaustively against DirectEvaluation at lns<8,4>, and
  `benchmark/accuracy/lns/cordic_precision_assessment.cpp` regenerates
  `docs/design/cordic-precision-assessment.md` byte for byte under gcc and clang.
- **Each fix:** a test that fails against main's header, run on gcc and clang before the PR.
- **CI:** every PR iterated on CodeRabbit as a draft, then merged after the full tier passed.
  Codacy and coveralls are advisory.

## Next Steps

- #2014: the user's decision on the elreal performance layers and the default precision.
- #2001: valid's projective-circle model and the Gustafson design questions.
- #2047: QA sweep of posit, filed 2026-11-11, to be triaged.
- **Release:** `[Unreleased]` holds 31 behaviour changes across 41 commits since v5.15.0.

## Appendix

- CORDIC headers: `number/fixpnt/math/cordic.hpp`, `number/dfixpnt/math/cordic.hpp`,
  `number/lns/lns_addsub_cordic_datapath.hpp`.
- CORDIC docs: `docs/algorithmic-details/cordic.md`, `docs/design/cordic-precision-assessment.md`,
  `docs/number-systems/lns-addsub-algorithms.md`, `docs/algorithmic-details/lns-log-add-sub.md`.
- Sweep issue and map comment: #1999.
