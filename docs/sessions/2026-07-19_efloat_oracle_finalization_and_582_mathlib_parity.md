# Development Session: efloat Oracle Finalization + #582 ereal/efloat Mathlib Parity

**Date:** 2026-07-19 (into 2026-07-20)
**Branches:** per-issue branches off `main` (all merged): #1155, #1156, #1157, #1158, #1159, #1161, #1162; then #1168, #1169, #1170, #1171, #1172.
**Focus:** Finish the follow-ups that make `efloat` a complete arbitrary-precision oracle (close Epic #1101), then audit and close the gap between the two Oracle number systems' mathematical libraries (close Epic #582).
**Status:** Complete -- Epic #1101 and Epic #582 both closed; issues #1110, #1114, #1120, #1121, #1150, #1160, #1163-#1167 closed.

## Session Overview

Two threads, run back to back:

1. **efloat Oracle-grade finalization** -- the loose ends from the prior session's oracle-completion work (#1150 operator<<, #1114 hyperbolic, #1110 complex, #1121 hypot, #1120 next), plus a real bug found along the way (#1160 long double conversion) and its cleanup (#1162).
2. **Oracle mathlib parity (#582)** -- an audit of both `ereal` and `efloat` against the standard `<cmath>` surface, five tracking issues (#1163-#1167), and their resolution.

All work validated on gcc 13 + clang 18; PRs taken through CI. CodeRabbit gave real reviews on several PRs and was rate-limited on others (a recurring fair-usage limit); rate-limited PRs merged on fast-tier + Codacy + local dual-compiler validation.

## Thread 1: efloat Oracle-grade finalization (Epic #1101)

Coming in, `efloat` had trigonometry, 1000-digit constants, and a demo suite, but several `<cmath>`-surface gaps and a `"TBD"` string-output stub.

- **decimal `operator<<` / `to_string` (#1150 / #1155)** -- ported `ereal`'s formatter; extracts digits via efloat's own arithmetic. Review caught two porting bugs (4-digit exponents like `1e1000`; fixed-format never rounded) plus a Codacy cppcheck finding (`unreadVariable`/`variableScope`).
- **hyperbolic (#1114 / #1156)** -- `sinh/cosh/tanh/asinh/acosh/atanh` on efloat's own `exp`/`expm1`/`log1p`/`sqrt`, cancellation-free near 0. The recurring `isneg()`-is-false-for-`-inf` gotcha applied again (use `sign() == -1` on the infinity branches). Review added assertions on the `InvalidOperation`/`DivisionByZero` side-effect flags.
- **`complex<efloat>` (#1110 / #1157)** -- surprise finding: `sw::universal::complex<efloat>` already worked through the generic library (concept-gated), so the binding is thin -- register `is_universal_number` + `real`/`imag`/`conj`, no `std::complex<efloat>` shims (UB). Documented that the shared library's complex *transcendentals* pivot through `double` for every UDT.
- **`hypot` (#1121 / #1158)** and **`nextafter`/`nexttoward` (#1120 / #1159)** -- relocated existing implementations into their own headers (repo one-function-per-header convention) with dedicated tests. `#1159` also fixed a real ULP bug (below).

### Bug: efloat `long double` conversion yielded 0 (#1160 / #1161)

Discovered while implementing `nexttoward` (which routed its target through `double` as a workaround). `convert_ieee754` only built limbs for `sizeof(Real)` 4 or 8; the `else` was a no-op `static_assert(true)`, so a 16-byte `long double` produced 0 (and output capped at double precision). Fixed both directions **portably** via `frexp` + sum-of-doubles decomposition (layout-agnostic -- handles both 80-bit x87 and IEEE binary128), round-trip-exact across precision and the extended exponent range. The `nexttoward` workaround was then removed (#1162).

## Thread 2: Oracle mathlib parity (Epic #582)

The audit's key finding: **both** mathlibs were already genuinely high-precision (Taylor/Newton in native arithmetic), reproducing `exp`/`sqrt`/`log`/`sin`/`erf`/`gamma`/`pi` to ~293 digits at `ereal<19>` (its architectural ceiling; Shewchuk expansion arithmetic caps `maxlimbs<=19`). The `ereal/mathlib.hpp` "Phase-0 double stub" header was simply stale. Five sub-issues filed and closed:

| # | PR | What |
|---|-----|------|
| #1163 | #1168 | ereal mathlib docs de-staled |
| #1164 | #1169 | efloat `frexp`/`ldexp` (new `numerics.hpp`) |
| #1165 | #1170 | ereal `fdim`/`modf`/`rint`/`nearbyint` |
| #1166 | #1171 | both: `fma`/`scalbn`/`logb`/`ilogb` |
| #1167 | #1172 | `complex<ereal>` binding |

`rint`/`nearbyint` use IEEE round-half-to-even (distinct from ereal's half-away `round`); `fma` is exact `x*y+z`; ereal `logb`/`ilogb` correct the leading-component exponent by +/-1 across power-of-two boundaries.

## Notable Bugs / Gotchas Surfaced

- **`efloat_impl.hpp:105-119` "compound arithmetic ... stubs" comment** is scoped to the C++20 *constant-evaluation* surface (#747) only; runtime arithmetic is fully implemented (efloat `exp(1)` matches the mpmath `e` literal to 60+ digits). It reads as misleading out of context -- flagged for a future cleanup.
- **`nextafter` power-of-two down-step**: efloat's own subtraction rounds `1 - 2^-256` back at working precision, so the boundary neighbor must be built at one extra limb of precision, gated to the power-of-two case (the trick corrupts non-power-of-two values).
- **fma tests must not be tautological**: comparing `fma(a,b,c)` to `a*b+c` can't detect rounding -- build the expected value independently (`(2^30+1)(2^30-1) = 2^60-1`, exact in the oracle, rounds to `2^60` in double).
- **C99 compound literals** `(const double[]){...}` are a GCC/Clang extension, ill-formed ISO C++ -- CodeRabbit flagged; replaced.

## Process Notes

- **PR-title lint**: the scope must be a single token from `.github/workflows/conventional-commits.yml`; `feat(efloat+ereal):` fails -- use `feat(math):`. `gh pr edit --title` fails on the projects-classic GraphQL path; `gh api -X PATCH repos/.../pulls/N -f title=...` works.
- **Never run `clang-format -i` on `efloat_impl.hpp`** -- it reformats the whole 2000+ line file (caught and reverted on the #1160 branch; hand-edit the target region instead).
- CodeRabbit rate-limiting recurred; rate-limited PRs merged on fast-tier + Codacy + local dual-compiler + cppcheck.

## Follow-on Work (tracked / suggested)

- Clean up the misleading `efloat_impl.hpp:105-119` constant-evaluation "stub" comment.
- A per-call precision-request API (e.g. `sqrt(x, 200)`) and a full-precision complex transcendental layer (the latter affects all user-defined types via the shared `complex<T>` library) -- both noted on #582 as non-blocking future enhancements.
- The flaky `Linux x64 (Clang)` full-tier CI job and the draft-then-ready full-tier skip remain open annoyances.
