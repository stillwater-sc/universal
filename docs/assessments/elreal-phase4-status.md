# Exact Lazy Real

We have a working, cross-platform, property-checked baseline for the additive substrate of the McCleeary lazy real — but not yet a validated baseline
for the exact lazy real type. Two distinct reasons: the type doesn't exist yet (scope), and the validation rests on a non-exact reference (rigor).
The methodology is right; the oracle strength and the surface are the gaps.

## What Phases 1-4 actually delivered

The McCleeary rewrite (after the older Phases A-L were deliberately backed out in 85cb5e50) built the substrate, not the type:

| Phase |                    Artifact                    |     Status      |
|-------|------------------------------------------------|-----------------|
| 1     | block<FpType> (value + per-unit exp)           | present, tested |
| 2     | ZBCL<FpType> lazy co-list                      | present, tested |
| 3     | block-level EFTs: two_sum / two_mult / two_div | present, tested |
| 4     | bit-perfect threeAdd + add() combinator        | present, tested |

Decisive fact: there is no elreal class and no arithmetic operators. elreal.hpp includes only block, zbcl, block_eft, threeAdd. elreal_fwd.hpp
forward-declares only block and ZBCL, and its own comment says "Phase 3+: arithmetic, math suite, real floating-point conversion (#927-#933)" — i.e.
multiply, divide, compare, math functions, and float conversion are not built. They existed in the backed-out A-L line; they have not been re-derived
on the McCleeary foundation. So as a usable number type, the exact lazy real is not yet a baseline at all — it's a validated adder plus its
primitives.

## Is the substrate validated — in the sense your conclusion demands?

Half the answer is genuinely good, and it's the half your ereal lesson is about:

What's right (the "use mathematics on the building blocks" part): threeAdd is checked against McCleeary's Definition 4.2.1 as theorems — value
preservation, dominance (dominate o1 o2 ∧ dominate o2 o3), the exponent bound, and no-shrink. add() is checked for ZBCL 0-overlap and
prefix-stability (take(3) is a prefix of take(6)). These are spec-derived mathematical invariants, not self-referential or structural-coincidence
checks. That is exactly the scientific-method-without-an-oracle approach the retrospective prescribed.

The gap that matters — there is no independent exact oracle. I grepped: no dyadic, no MPFR, no __float128, no multiprecision anywhere in
elastic/elreal/. Every value-preservation check uses long double. That fails the retrospective's own test ("is the oracle more precise than the thing
it validates?"):
- On x86_64, long double is a 64-bit mantissa — ~11 bits over double. Enough to catch gross errors, not enough to certify bit-exactness (the exact
sum/product of two doubles can need ~107 bits).
- On MSVC / most ARM / RISC-V, long double == double — zero headroom over the type under test. The threeAdd test's own comment admits this. On those
platforms the value check is nearly vacuous; it can only catch errors larger than a double ulp. This is precisely the weakness ereal's epic #987
built dyadic_exact.hpp to eliminate — and elreal has not adopted it.

Two smaller rigor gaps, both familiar from ereal:
- Loose tolerance on an exact type. add()'s value test allows ~4 FpType-ulp. For a type whose entire purpose is exactness, the representable cases 
  (1 + 0.25 → exactly 1.25) should assert diff == 0. A 4-ulp band would not fail on a sub-ulp regression — "would this test fail if the code were wrong?"
   → for small errors, no.
- Thin inputs. add() uses ~9 hand-picked, all-exactly-representable operands over only double/float; no catastrophic-cancellation, many-limb, or
  subnormal stress. (threeAdd is better — it does randomize.)

## Recommendation (directly from the ereal conclusion)

Before Phase 3+ stacks multiply/divide/transcendentals on this, port dyadic_exact.hpp as elreal's oracle and re-assert Phases 1-4 bit-exactly — 
diff == 0 where the exact result is representable, exact dyadic comparison otherwise — with randomized and adversarial sweeps, and on a platform where the
oracle has real headroom (not long-double-aliased). That converts "working substrate, property-checked" into a validated baseline in the strong
sense, and avoids repeating the ereal trap one layer up: composing new operations on top of building blocks whose "exactness" is currently asserted
by a reference that isn't exact.

Want me to file that as the Phase-5-precondition issue (port the exact oracle + tighten the Phase 1-4 assertions), or scope it as part of whatever
issue covers the next arithmetic phase?

Filed #1022 (https://github.com/stillwater-sc/universal/issues/1022) — "elreal Phase 5 precondition: port the exact dyadic oracle and re-assert
Phases 1-4 bit-exactly" (label enhancement), and cross-linked it as a precondition comment on #929 (Phase 5).

The issue frames it exactly as we discussed:
- Why — applies the ereal/#987 rule (validate exactness against an independent exact oracle, not a merely-more-precise one) to the elreal substrate,
citing the retrospective.
- The gap, with evidence — no dyadic/MPFR/__float128 anywhere in elastic/elreal/; everything rests on long double (~11 bits headroom on x86_64, zero
on MSVC/ARM/RISC-V where it aliases double); the 4-ulp tolerance on an exact type; thin hand-picked inputs.
- Deliverable — a concrete, portable mapping (exact_value(block) = dyadic::from_double(double(b.v)) << b.exp; ZBCL prefix = exact sum of block
dyadics), re-assert the three EFTs + threeAdd + add() with bit-exact dyadic equality, randomized/adversarial sweeps, and an oracle-headroom guard. It
also respects the #942 cfloat<24,5> subnormal-residual floor as a representability limit rather than an error.
- Acceptance criteria + references to epic #923, phases #925-928, EFT fix #942, the ereal oracle precedent, and the retrospective.

It's scoped as a blocker, so Phase 5 (#929) and everything downstream (#930-933) should build on a baseline that's validated in the strong sense, not
just working.

## Issue #1022 resolved

Branch: feat/issue-1022-elreal-exact-oracle · PR: #1023 (https://github.com/stillwater-sc/universal/pull/1023) (draft) · All local tests pass (gcc +
clang)

### What was done

Ported the ereal/#987 independent exact oracle (dyadic_exact.hpp) to the elreal McCleeary substrate, replacing the Phase 1-4 reliance on long double
(which has zero headroom on MSVC/ARM/RISC-V). New exact_value_oracle.cpp asserts the building blocks bit-exactly against einteger-backed dyadic
rationals; addition.cpp tightened from a 4-ulp band to the same exact check.

The headline result — the oracle did exactly what the ereal retrospective said it should

It immediately exposed a real fact the loose tolerances were hiding, on the very first run:

- 5 round-to-nearest hosts now certified bit-exact: float, double, half, cfloat<24,5> (modulo the #942 subnormal floor), cfloat<32,8>. This is the
validated baseline #1022 asked for.
- bfloat16 is not an exact-EFT host — it rounds toward zero (round_toward_zero; cast is bits >> 16, no rounding), so Knuth/Dekker EFTs leak ~1 ulp on
it. I traced this carefully (step-by-step instrumentation + a rounding-mode probe across all six types) to confirm it's intentional bfloat16
behaviour, not an elreal bug — and I didn't paper over it: bfloat16 is pinned with a truncation bound, documented in the test header, and noted on
epic #923 (https://github.com/stillwater-sc/universal/issues/923#issuecomment-4559221249).

I also caught and fixed a latent reference bug the old 4-ulp band hid in addition.cpp (the float host was being compared against the original double
literals, not the represented float-cast operands).

### Verification

|            Target            | gcc  | clang |
|------------------------------|------|-------|
| el_arith_exact_value_oracle  | PASS | PASS  |
| el_arith_addition            | PASS | PASS  |
| full elreal suite (16 tests) | PASS | PASS  |

### Next steps

- Fast CI (gcc + clang CI_LITE builds elreal); gh pr ready 1023 when you're satisfied.
- Worth your attention: the bfloat16 finding means elreal's exactness guarantee is conditional on a round-to-nearest host — Phases 5+ that assert
exactness should run conformance on the RN hosts, and the docs should state that bfloat16 is approximate by construction. That's captured on #923.

One note: I followed the issue's own design (which I'd scoped earlier), so I synthesized the analysis directly rather than spawning a fresh
issue-analyzer pass — I had the full code context from writing #1022 minutes ago.

## EFT Validation

Issue #1025 merged, #1024 closed, main synced, branch deleted, tree clean.

### Where elreal stands now

The McCleeary lazy-real additive and multiplicative core is certified exact at quad precision, validated by a trustworthy bit-based dyadic oracle (no
longer dependent on the buggy cfloat frexp/floor paths). That's the validated baseline your "leverage elreal with quad precision" goal needed.

Merged this session (elreal verification arc):
- #1023 — independent exact dyadic oracle for the Phase 1-4 building blocks
- #1025 — quad-capable two_prod_host (fused-fma) + quad-robust oracle

Open follow-ups, all tracked:

|   Issue   |                                                  What                                                   |
|-----------|---------------------------------------------------------------------------------------------------------|
| #1026     | cfloat floor/trunc/round wrong for wide large integers (routes through double)                          |
| #1027     | cfloat frexp returns [1,2) instead of std's [0.5,1)                                                     |
| #929–#933 | elreal Phases 5–9 (infinite summation, negation/mul/div, transcendentals, fp conversion, qd evaluation) |
| #923      | the McCleeary epic                                                                                      |

The two cfloat bugs (#1026/#1027) are genuine core defects the oracle surfaced — worth fixing on their own merits, and they'd let the oracle later
drop its bit-based workaround. The elreal phase work (#929+) can now build on a baseline that's exact-validated for both add and multiply across all
round-to-nearest hosts including quad.

A meta-note worth keeping: across #1023 and #1025, the "validate the validator" discipline caught two fixes that looked correct — the bfloat16 "EFT
is exact" assumption (#923) and the cfloat-fma "sign bug" false alarm (which was my own broken oracle, not cfloat). Both times an independent exact
reference, not a passing test, told the truth.

## cfloat bugs fixed

Issue #1031 merged (2026-05-28T18:16:57Z), #1030 closed, main synced, branch deleted, tree clean.

The cfloat IEEE-conformance cluster is fully closed

| Issue |                       Fix                       |  PR   | Status |
|-------|-------------------------------------------------|-------|--------|
| #1026 | floor/ceil/trunc/round native (correct >53-bit) | #1028 | merged |
| #1027 | frexp → std [0.5,1) semantics                   | #1029 | merged |
| #1030 | isnormal(±0) → false                            | #1031 | merged |

All three were surfaced by the elreal exact-oracle work (#1024), all fixed with careful caller sweeps confirming no adaptation was needed, all
validated bit-exactly on gcc + clang through the full-tier gate, and the learnings are captured in reference_cfloat_ieee_conformance.md.

Net effect: cfloat's math/classification surface is now IEEE-conformant for the cases that matter to high-precision work — which also means the
elreal oracle's bit-based extraction is no longer forced (it remains as a clean, self-contained reference and a standing regression check).

Local main is clean with no stray branches. Nothing else in flight. The broader arc this session — validated quad-precision elreal (add + multiply)
plus the cfloat conformance fixes its oracle exposed — is in a coherent, merged state.

