# Development Session: efloat Oracle-Grade Completion + Adaptive-Precision Demonstrations

**Date:** 2026-07-18
**Branches:** per-issue branches off `main` (all merged): #1137, #1138, #1142, #1143, #1144, #1145-#1149.
**Focus:** Finish `efloat` as an oracle-grade arbitrary-precision type (trigonometry, 1000-digit constants, precision/parse fixes) and build the five demonstration programs that show why it matters.
**Status:** Complete -- issues #1115, #1138, #1139, #1140, #1141, #1096-#1100 closed; Epic #1092 (efloat mathlib) complete.

## Session Overview

`efloat` is the library's single-component arbitrary-precision binary float. Coming into this session its precision scaled with `nlimbs` but the *library around it* had gaps: no trigonometry, no high-precision constants, and two latent precision bugs. This session closed those gaps and then exercised the finished type with a demonstration suite.

Two threads, run back to back:

1. **Oracle-grade completion** -- trigonometry (#1137), 1000-digit pi/e/phi constants (#1142), and three bug fixes surfaced while building the constants: `abs`/`fabs` stub (#1138), `sqrt`/`cbrt` precision cap (#1140/#1143), and `parse()` overflow (#1141/#1144).
2. **Demonstration suite** -- five `applications/precision/adaptive/` programs (#1096-#1100), each the same templated kernel on `double` vs `efloat`.

All work validated on gcc 13.3.0 + clang 18.1.3; PRs taken through CI (and CodeRabbit review where it wasn't rate-limited).

## Thread 1: Oracle-grade completion

### Trigonometry (#1115 / #1137)
Native `sin/cos/tan/asin/acos/atan/atan2` -- argument reduction + Taylor series, not double-delegating shims. Adapted the algorithm from `ereal` but avoided its `atan2` double-literal bug (used the efloat pi constants). A review pass caught a real infinity bug: `isneg()` is false for `-inf` (state is Infinite, not Normal), so `atan(-inf)` and several `atan2` branches were mis-signed -- switched to `sign() == -1` and added the `atan2(inf, inf)` diagonal cases.

### 1000-digit constants (#1139 / #1142)
`efloat_pi/e/phi<nlimbs>()` from ~1000-digit mpmath literals. The investigation was the hard part and reshaped the plan:
- efloat `+ - * /` are solid to 3400+ bits, but `parse()` broke above 2040 bits and `sqrt()` was capped at ~97 digits.
- The d2b `convert` needs `BigBits >> target + 3*ndigits` of headroom -- a 1000-digit literal at a ~3300-bit target needs `BigBits=16384`, not the 4096 first tried.
- Validation uses **independent** oracles (phi identity, native-series e, Machin pi), not the literal validating itself.

### Three bug fixes
- **#1138 abs/fabs**: the free `abs(efloat)` was `return a;` (a no-op). Fixed + added `fabs`; also un-broke `log1p`'s magnitude guard.
- **#1140/#1143 sqrt/cbrt**: fixed 7 Newton iterations from a ~1-bit seed froze accuracy at ~323 bits. Made the count precision-adaptive with a convergence break, keeping the unbounded-exponent seed.
- **#1141/#1144 parse**: capped at 2040 bits and returned garbage above it (convert overflow). Templated on `BigBits` + overflow-safe target sizing that budgets both negative and positive exponents.

## Thread 2: Adaptive-precision demonstrations (#1096-#1100)

Five programs in the new `applications/precision/adaptive/` directory, one plug-in kernel each:

| Demo | double | efloat |
|------|--------|--------|
| catastrophic_cancellation (#1096) | `(1-cos x)/x^2` -> 0 | -> 1/2 (correct) |
| ill_conditioned_systems (#1097) | Hilbert n=12: ~17% error | exact ~1e-139 |
| high_precision_fractals (#1098) | ~75% pixels wrong (vertical bands) | correct render |
| mathematical_identities (#1099) | BBP pi: ~15 digits | ~152 digits |
| polynomial_roots (#1100) | Wilkinson roots drift ~1e-2 | exact integers |

`mathematical_identities` uses the `efloat_pi()` constant from #1142 as its oracle -- the constants work paying off in a later demo.

## Notable Bugs / Gotchas Surfaced

- **`isneg()` is false for `-inf`** (state-based), so sign tests on possibly-infinite operands must use `sign() == -1` (as `exp()` already did). Bit both trig (atan/atan2) and would bite any efloat code handling infinities.
- **d2b `convert` overflow is target+digit-count driven**, not a fixed cap: `num <<= headroom + 3*neg_E`. Raising `BigBits` alone is not enough if you don't also account for the literal length.
- **efloat `operator<<` is a TBD stub** -- the fractal/demo values print via `double()` casts and scale metrics; a real high-precision `operator<<` is a worthwhile future enhancement.
- **Fractal deep-zoom is axis-specific**: `dx` was 0.34 ulp at `CENTER_X` but 1.35 ulp at `CENTER_Y` (smaller binade), so only the x coordinates collapse -> vertical (not horizontal) bands. Verified empirically before writing the README.
- **CodeRabbit's clang-format suggestion contradicted `.clang-format`** (it wanted to de-align assignments, but `AlignConsecutiveAssignments` is enabled) -- ran the project formatter instead of the suggested diff.

## Process Notes

- Speculative-merge pattern: fast-tier + core CI green + local dual-compiler validation was the bar for merging demo PRs; the long sanitizers finished post-merge.
- Two recurring CI annoyances observed: a flaky `Linux x64 (Clang)` job, and the full tier skipping when a fix is pushed while draft and then promoted to ready (the ready flip doesn't re-trigger those workflows).
- CodeRabbit hit fair-usage rate limits repeatedly given the PR volume; waited it out rather than fabricating findings.

## Follow-on Work (tracked / suggested)

- A full-precision `efloat operator<<` (prints many digits) -- would let the demos show their extra digits directly.
- `parse<16384>` is now a general high-precision path, so #1139's bespoke `make_constant` helper could be simplified to use it.
- The already-merged demos were written in manual style before I started clang-formatting; a small sweep to `clang-format` all `adaptive/` files would tidy the set.
- Investigate the flaky `Linux x64 (Clang)` CI job and the draft-then-ready full-tier skip.
