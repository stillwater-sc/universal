# Development Session: elreal Class Facade (Epic #1079) + Constant Performance (#1061 Phase 3b)

**Date:** 2026-06-18 .. 2026-06-21
**Branch:** per-phase feature branches off `main` (all merged)
**Focus:** Give `elreal` (McCleeary LFPERA lazy exact-real) the standard Universal
plug-in facade, and make the last two eager math constants fast.
**Status:** Complete -- Epic #1079 closed; #1061 Phase 3b constant work merged.

## Session Overview

`elreal` is the library's lazy exact-real type: a real is a (possibly infinite)
co-list of `block<FpType>` (the `ZBCL`), and arithmetic is online -- each consumer
pull drives exactly as much work as the requested precision needs. Before this
session it existed only as free functions over `ZBCL`. This session built the
`class elreal<FpType>` facade so it plugs into templated kernels like every other
number system, then closed out the remaining constant-generator performance gap.

Two threads:

1. **Epic #1079 -- the class facade** (Phases 1-5; Phases 1-2 landed at the very
   start, Phases 3-5 are the bulk of this session). Five PRs, all merged.
2. **#1061 Phase 3b -- eager-constant conversion** (online `e`, Abel-summed
   `euler_gamma`). Two PRs, both merged.

All work validated on gcc + clang (Release), gcc Debug-with-assertions for the new
suites, cppcheck (Codacy gate), ASCII guard, and -- for the constants -- the
320-digit mpmath oracle at `REGRESSION_LEVEL_4`.

### Goals Achieved

- Phase 1: facade scaffold -- `class elreal`, native ctors/conversions, lazy
  operators, depth-bounded comparison (#1080)
- Phase 2: lazy API hardening -- memoisation regression, precision-honest approx (#1081)
- Phase 3: `numeric_limits` / `attributes` / `manipulators` (#1082)
- Phase 4: `mathlib.hpp` math facade -- functions + constants (#1083)
- Phase 5: dedicated IEEE non-finite state + conversion/logic/arithmetic suites (#1084)
- #1061 Phase 3b: online `e_zbcl` (~195x); Abel-summed `euler_gamma` (~2.3-3.1x) (#1087, #1088)
- Epic #1079 closed; `docs/design/elreal-euler-gamma.md` written

## Architecture Decisions

### The facade is elastic, not trivial

`elreal` holds a `ZBCL` (a `shared_ptr`-backed memoised co-list), so it is **not**
trivially copyable. The #925 hardware-shareable triviality rule applies to the
static `block`, not to this elastic facade -- same posture as `ereal`/`einteger`.
The facade is modelled on `ereal_impl.hpp` (the eager Priest/Shewchuk sibling).

### Three settled facade design decisions (Phase 1)

| Concern | Decision |
|---------|----------|
| Precision | runtime `_depth` member + `.precision()` + thread-local default with an RAII `elreal_precision_guard` scoped override |
| Operators | **fully lazy** -- `+ - * /` store unforced `add`/`mul_online`/`div_online` streams; evaluation happens only at a boundary (conversion / compare / I/O / explicit `approx`) |
| Comparison | **depth-bounded** -- compare `a-b` to the deeper of the two depths; exact ordering when the difference's leading limb is nonzero (exact equality of distinct irrationals is undecidable) |

### Non-finite policy (Phase 5) -- a deliberately reversed decision

Phase 3 shipped a **finite-only** `numeric_limits` (the LFPERA model is exact over
the finite reals). For Phase 5 the user chose the larger option: a **dedicated
IEEE-style non-finite state** (`elreal_class {finite, pinf, ninf, qnan}` + a `_cls`
member), which *revised* the Phase 3 contract. Rationale: plug-in kernels can
produce `NaN`/`+-Inf` (`x/0`, `log` of a negative, conversion overflow), and a
predictable IEEE classification beats silently mapping them to 0. The tag
propagates by IEEE-754 rules through arithmetic, comparison (`elreal_order_of`:
NaN unordered, `-inf < finite < +inf`), conversion, unary minus, and `abs`.

### euler_gamma: Brent-McMillan + Abel, binary splitting deferred

`euler_gamma` has no elementary series, so it uses Brent-McMillan B1
(`gamma = A/B - ln(n)`). The dominant cost was the `A = sum_k w_k H_k`
accumulation. Abel summation rewrites it as `A = sum_{k>=0} tail_k/(k+1)`
(`tail_k = B - B_k`), replacing the per-term full multiply with a single-block
scalar division and removing the harmonic numbers entirely. The asymptotically
better approach (binary splitting with `einteger` P/Q/B/T) was **investigated and
deliberately deferred**: it needs an arbitrary-precision-integer dependency the
elreal math layer does not have, a new `einteger -> ZBCL` bridge, and a
harmonic-aware splitting tuple, with no repo precedent. Full reasoning and the
algorithm are in `docs/design/elreal-euler-gamma.md`.

## Notable Bugs / Gotchas Surfaced

- **CodeRabbit (Critical), Phase 5** -- `to_triple(-inf)` printed `"(-, -inf)"`:
  the sign was emitted twice (the `(sign, ...)` prefix plus a signed tag). Fixed
  with an unsigned-tag option on `nonfinite_tag`; regression checks added.
- **`a + (-a)` is value-zero but not `iszero()`** -- lazy exact cancellation yields
  a zero-*valued* stream, not the structurally-empty canonical zero. `iszero()` is
  a cheap structural test (the right semantics for a semi-decidable lazy real), so
  it is correctly `false` there. A test assertion was relaxed and documented.
- **cppcheck `duplicateCondition` folds mirror operators** -- `a<b` and `b>a`
  normalise to the same condition, so the logic suite's `if (...) ++n;` checks read
  as duplicates. Rewrote as `n += !(...)` accumulation statements on opaque
  (volatile-seeded) operands -- exercises every operator with no duplicate
  if-conditions.
- **Codacy `noExplicitConstructor` counts per-instantiation/per-ctor-edit** -- the
  intentional implicit plug-in ctors (same as ereal/cfloat/posit) get re-counted
  when a new instantiation appears (first `elreal<float>` test) or the ctor region
  is edited. Accepted as non-blocking (Codacy is not a required check); making them
  `explicit` would break plug-in semantics.
- **The 320-digit constant oracle is `REGRESSION_LEVEL_4`-gated** -- a default
  (`LEVEL_1`) build compiles the high-precision check out and the test "PASSes" as
  a no-op. The euler_gamma cancellation analysis was only truly validated after
  rebuilding with `-DREGRESSION_LEVEL_4=1` (305 digits, gcc + clang). Recorded so
  the next person is not fooled by the no-op pass.
- **Eager constants were pathologically slow, not the functions** -- profiling the
  Phase 4 facade showed every transcendental *function* < 100 ms, but `e_zbcl(32)`
  ~16 s and `euler_gamma_zbcl` ~11 s, because those two *constants* had not been
  moved onto the online series path. (`exp(1)` via the online path is 1 ms.)

## Performance Results

| Item | Before | After | Speedup |
|------|-------:|------:|--------:|
| `e_zbcl(16)` | 16165 ms | 83 ms | ~195x |
| `euler_gamma(8)` | 4408 ms | 1890 ms | ~2.3x |
| `euler_gamma(16)` | 10986 ms | 3578 ms | ~3.1x |

All value-identical to the 320-digit reference (e: 307 digits, euler_gamma: 305).

## Pull Requests

| PR | Title | Merge |
|----|-------|-------|
| [#1080](https://github.com/stillwater-sc/universal/pull/1080) | facade scaffold (#1079 Phase 1) | 22857fb7 |
| [#1081](https://github.com/stillwater-sc/universal/pull/1081) | lazy API hardening (#1079 Phase 2) | d679ebe8 |
| [#1082](https://github.com/stillwater-sc/universal/pull/1082) | numeric_limits/attributes/manipulators (#1079 Phase 3) | 1ccdf224 |
| [#1083](https://github.com/stillwater-sc/universal/pull/1083) | math facade (#1079 Phase 4) | 6444f6b2 |
| [#1084](https://github.com/stillwater-sc/universal/pull/1084) | non-finite state + suites (#1079 Phase 5) | 823dff27 |
| [#1087](https://github.com/stillwater-sc/universal/pull/1087) | online e_zbcl + drop gamma peak pass (#1061 Ph3b) | ace597bd |
| [#1088](https://github.com/stillwater-sc/universal/pull/1088) | Abel-summed euler_gamma (#1061 Ph3b) | 79272eb5 |

## Process Notes

- Per-phase workflow: branch off `main`, draft PR -> fast tier (gcc+clang CI_LITE)
  -> resolve CodeRabbit/Codacy -> `gh pr ready` -> full tier (11 platforms +
  ASan/UBSan + Coverage + Clang-Tidy) -> admin-squash-merge `--delete-branch` ->
  sync main -> check off the phase on #1079.
- When a phase started before the prior merged, it branched off the prior tip and
  was rebased onto `main` (`git rebase --onto main <prior-tip> <branch>`) after the
  prior squash-merge.

## Follow-on Work (tracked, not blocking)

- **Binary-splitting `euler_gamma`** -- the asymptotic `O(M(D) log^2 D)` win;
  needs einteger integration + an einteger->ZBCL bridge (see the design doc).
- **`to_hex` + high-precision decimal printer** for elreal manipulators (deferred
  in Phase 3, as in `ereal` where both are `tbd` stubs).
