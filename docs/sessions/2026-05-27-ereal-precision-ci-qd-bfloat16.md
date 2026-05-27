# Session Log: 2026-05-27 - ereal Full-Precision Arc, CI Repair, qd/bfloat16 Fixes

**Session Date:** May 26-27, 2026
**Participants:** Claude Code (AI Assistant), User (Theodore Omtzigt / Ravenwater)
**Branch:** main (via topic branches per PR)
**Objective:** Drive the ereal precision/verification work to completion and merge, repair the CI regressions it surfaced, and clear the related backlog (qd constant bug, bfloat16 exponent functions, parse benchmark).

---

## Session Overview

This session resolved and merged a connected cluster of issues. It began by
driving the ereal extended-precision fix (#1002) through CI, which -- via an
honest, MPFR-referenced regression test -- exposed two deeper layers, and
those in turn surfaced two CI regressions. Nine PRs merged and one issue
(#913) was closed by verification.

**PRs merged (in order):**

1. **#1004** -- ereal full-precision transcendentals (#1002, #1005)
2. **#1012** -- inverted `is_nonoverlapping` predicate (#999)
3. **#1011** -- ereal `parse()` precision/NaN + `expansion_quotient` scaled division (#1006)
4. **#1008** -- ereal mathlib test fuzz tiering (#1007)
5. **#1010** -- ccache save-gating (#1009)
6. **#1014** -- qd_pi_3 constant correction (#914)
7. **#1015** -- bfloat16 ldexp/frexp/scalbn/logb/ilogb (#941)
8. **#1017** -- marshal those bfloat16 functions through float (PR #1015 review)
9. **#1016** -- ereal parse-cost benchmark (#1013)

**Issues filed this session:** #1005, #1006, #1007, #1009, #1013 (follow-ups,
all but #1013 resolved same session; #1013 -> #1016).
**Issue closed by verification:** #913 (parse O(N^3) blowup).

---

## The ereal precision investigation (the spine of the session)

#1002 ("transcendentals stuck at ~16 digits") looked like a one-layer bug
(constants from `double` literals). Rewriting the `progressive_precision`
regression test to measure *honestly* -- against precomputed limb-injected
MPFR references rather than `parse()`d strings -- revealed three stacked
limiters, each masking the next:

1. **Constants from `double` literals** truncated to ~16 digits. Replaced with
   precomputed 19-limb non-overlapping expansions summed exactly at runtime
   (the dd/qd approach), bypassing `parse()`.
2. **`expansion_reciprocal` hardcoded `iterations = 3`** -> Newton reciprocal
   reached only ~130 digits, capping *all* division (hence every
   transcendental) regardless of `maxlimbs`. Fixed to scale as
   `ceil(log2(maxlimbs)) + 1`. (#1005)
3. **`parse()` could not represent a 300-digit constant** -- it built a giant
   integer and multiplied by the *subnormal-range* `10^(-e)`, losing ~1 digit
   per power of ten and returning NaN past ~305 digits. (#1006, fixed in #1011)

Key insights that came out of the diagnosis:

- **`1/V` for large `V` is inherently ~1 limb.** `1/10^300 ~ 1e-300`; its
  second expansion component (~1e-316) is below DBL_MIN, which Shewchuk's
  arithmetic forbids. The reciprocal accuracy degraded as
  `~ 320 - log10(magnitude)`. The fix (in `expansion_quotient`) scales the
  divisor to near-unit magnitude with an exact `ldexp`, runs the Newton
  reciprocal on a normal operand, then applies the exact `2^-k` -- so a
  *normal-range quotient* (parse's `M/10^e ~ 0.14`) keeps full precision even
  though the tiny `1/V` cannot.
- **ereal operations do not cap at `maxlimbs`.** `sin(ereal<4>)` returns 20
  components; `maxlimbs` only bounds each function's convergence threshold.
  Clean per-limb scaling comes from that threshold, not storage truncation.
- **A phantom CI failure** on `er_api_progressive_precision` was a leftover
  `set_tests_properties(... WILL_FAIL TRUE)` -- ctest inverts the verdict, so
  the now-passing test reported as Failed. Removed.

Generator preserved at `tools/generators/ereal_reference_gen.py` (mpmath; emits
both the stored constant expansions and the test references, with the critical
exact-double-input handling, e.g. `cos(double(0.3))` not `cos(3/10)`).

---

## CI repair

Driving the above through CI surfaced two regressions, both diagnosed from
build/job telemetry rather than guessed:

- **#1009 ccache eviction** -- CI build time had gone from ~8 min to ~20 min.
  Root cause was *not* the code (CI_LITE does not even build ereal): the
  `ccache` action saved a fresh ~500 MB cache on every PR run, and under
  GitHub's 10 GB per-repo LRU limit a burst of CI evicted the warm caches.
  The `[fast]` Build step jumped 55 s -> 566 s with a ~1% ccache hit rate.
  Fixed by gating `save:` to `main` pushes + `max-size: 1G`.
- **#1007 mathlib test slowness** -- the Debug `-O0` instrumented jobs
  (ASan/UBSan/coverage) ran the heavy L1 property-fuzz at ~40x the `-O3` cost.
  The fuzz count now tiers with the regression level.

---

## The backlog cluster

- **#914 qd_pi_3** carried pi/2's value (2 of 4 components). Corrected to the
  4-component pi/3 expansion; the qd constants test (which only printed and
  always returned success) was made to actually assert.
- **#941 bfloat16** gained the `<cmath>` exponent family. PR #1015 review
  (Ravenwater) pointed out marshalling should go through `float` (shares
  bfloat16's exponent range, cheaper than double); applied in #1017 along with
  CodeRabbit's IEEE-special-case test coverage.
- **#1013** added the parse-cost benchmark closing #913's last acceptance item.

---

## PR review handling

`/resolve-pr-reviews` was run on #1015 and #1016:

- **#1015** (merged): 4 maintainer threads (marshal through float) addressed in
  follow-up #1017; threads resolved with a linking comment.
- **#1016**: 2 CodeRabbit threads (check `parse()` return value; "median" ->
  "average" wording) fixed in-branch, re-review came back clean, threads
  resolved.

---

## Process notes

- Every PR driven through the platform matrix gate and admin-squash-merged with
  the long ASan/UBSan/coverage jobs trailing post-merge (per the established
  pattern), after confirming the matrix + lint + clang-tidy were green.
- One conventional-commit slip caught and fixed: `ci(ccache)` -> `ci(cmake)`
  (`ccache` is not an allowed scope).
- Verified the behind-branch squash-merge of #1016 did not clobber main's
  newer content (float marshalling, IEEE special cases, parse review fix all
  intact).

## Status at session end

- 9 PRs merged; #913, #914, #941, #999, #1002, #1005, #1006, #1007, #1009,
  #1013 all CLOSED.
- ereal transcendentals deliver full ~304-digit precision at `ereal<19>`;
  division and parse correct across the magnitude range; the overlap predicate
  is sound; CI is back to ~8 min and the instrumented jobs are tractable.
- Local `main` synced; topic branches pruned.
