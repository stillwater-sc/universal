# ereal Retrospective: Development, Testing, Bugs, and Architecture Mishaps

**Type under review:** `ereal<maxlimbs>` — adaptive multi-component (Priest/Shewchuk
expansion) floating-point over `std::vector<double>`.

**Scope:** the arc from the trust-destroying normal-form oversight through the
verification epic (#987) and the precision/parse/predicate fixes (#999, #1002,
#1005, #1006, #913). Written as both a *retrospective* (what happened) and an
*introspective* (why it happened, including the AI assistant's failure modes).

---

## 1. Executive summary

`ereal` was shipped as "a Douglas Priest multi-component floating-point" but for a
long time it did not *enforce* — and was not *verified* to satisfy — Priest normal
form, and its transcendentals silently delivered ~16 digits regardless of the
configured precision. None of this was caught by the existing tests, because those
tests were **self-referential, under-asserted, or compared against references no
more precise than the bug**. The headline lesson is not any single bug; it is that
**a multi-component number type accumulates a stack of plausible-looking defects
that only an independent, exact, end-to-end oracle can expose**, and the test
suite that should have been that oracle was instead a rubber stamp.

The work recovered correctness, but the cost (a multi-week verification epic plus a
cluster of follow-on fixes) was a direct consequence of building the type
"by association" — it *looked* like Priest arithmetic — rather than to a verified
specification.

---

## 2. What went wrong, by category

### 2.1 Architecture / design mishaps

| # | Mishap | Consequence |
|---|--------|-------------|
| A1 | **Priest normal form never enforced or verified.** The type was named and documented as Priest/Shewchuk multi-component, but nothing checked the non-overlapping / decreasing-magnitude / no-interior-zero invariants. | A normalized-but-wrong expansion is indistinguishable from a correct one without an external oracle. Trust collapse (the originating incident). |
| A2 | **Constants stored as `double` literals.** `Real pi(3.14159...100digits...)` truncates at the literal to ~16 digits. | Every transcendental that used a constant was capped at ~16 digits regardless of `maxlimbs` (#1002, layer 1). |
| A3 | **`expansion_reciprocal` hardcoded `iterations = 3`.** Newton reciprocal reaches ~53·2³ ≈ 130 digits, independent of `maxlimbs`. | Capped *all* division — hence every transcendental — at ~130 digits (#1005, layer 2). |
| A4 | **`parse()` architecture: giant-integer Horner × `10^(-e)`.** Builds the full mantissa as an integer, then multiplies by a subnormal-range `10^(-e)`. | O(N³) complexity (320-digit parse > 120 s, #913); precision decayed ~1 digit/power; NaN past ~305 digits (#1006). The constants couldn't even be parsed at full precision (#1002, layer 3). |
| A5 | **Division/reciprocal not magnitude-robust.** Forming `1/V` for large `V` produces a value near DBL_MIN whose components are subnormal. | Reciprocal accuracy degraded as `~320 − log10(magnitude)`. Inherent for a *tiny result*, but the quotient algorithm didn't scale operands to keep *normal-range quotients* exact. |
| A6 | **`maxlimbs` is an input ceiling, not a runtime cap.** Operations return more than `maxlimbs` components (`sin(ereal<4>)` returns 20 limbs). | Confused all precision reasoning: "ereal<4>" does not mean 4-limb results. Clean per-limb scaling actually comes from each function's convergence threshold, not storage truncation. `parse()` had to cap the result to `maxlimbs` explicitly. |
| A7 | **`is_nonoverlapping` predicate inverted.** Used a `fast_two_sum` error heuristic with the condition backwards. | A *public* predicate gave confidently wrong answers (non-overlapping → "overlapping" and vice-versa) for years (#999). |

### 2.2 Implementation bugs (the catalog)

- **#1002 / #1004** — transcendentals capped at ~16 then ~130 digits (A2 + A3).
- **#1005** — `expansion_reciprocal` iterations fixed at 3 (A3).
- **#1006 / #1011** — `parse()` precision cliff + NaN; the underlying
  `expansion_quotient` couldn't divide large/small magnitudes accurately (A4, A5).
- **#999 / #1012** — `is_nonoverlapping` / `is_strongly_nonoverlapping` inverted (A7).
- **#913** — `parse()` O(N³) complexity blowup (A4).
- Earlier (epic #987 era): `einteger` multiply carry and `erational` conversion
  bugs surfaced *only* because the new exact oracle exercised them at full scale —
  they had been latent under the previous self-referential tests.

### 2.3 Testing mishaps (the most important section)

This is where the real failure lived. Every architecture bug above had a test that
*should* have caught it and didn't:

1. **Self-referential oracles.** `progressive_precision` compared `ereal<N>` against
   `ereal<19>` — *the same code on both sides*. A normalized expansion encoding the
   wrong value passes trivially. (Generalised lesson in #987: structural oracles —
   "is it Priest-normal?" — and self-referential oracles cannot catch a
   *normalized-but-wrong* value.)
2. **References no more precise than the bug.** When `progressive_precision` was
   rewritten to use "high-precision" references, they were built by **`parse()`-ing
   300-digit strings** — but `parse()` itself caps at ~24 digits (A4). So the test
   measured the function against a reference that was *also* wrong, and reported
   PASS. The fix required **limb-injected MPFR references** that share no code with
   the implementation.
3. **Wrong reference inputs.** References were generated for the *exact decimal*
   (`cos(3/10)`) while the code computes `cos(double(0.3))`; these diverge after ~16
   digits, producing false "precision loss" reports that masked the real bug.
4. **Thresholds off by 4×.** `progressive_precision` asserted "expect ≥ 15/30/45/…
   digits" — i.e. **3.75 digits/limb** instead of the real ~15.95 — so a function
   delivering a quarter of its precision still "passed."
5. **Saturating / capped metrics.** The digit-count helper returned a hardcoded
   `100.0` on an exact match, so genuinely-better results were indistinguishable
   from the cap.
6. **Tests that print but never assert.** (Sibling example caught in the same
   sweep: the qd constants test printed every constant vs its reference and
   returned `EXIT_SUCCESS` unconditionally — which is exactly how `qd_pi_3 = pi/2`
   survived, #914.) The ereal suite had the same "diagnostic, not assertion"
   pattern in places.
7. **Tolerances loose enough to pass broken code.** The elreal `two_mult` EFT test
   allowed `128 · ulp²` — looser than the bug it was meant to catch.
8. **A `WILL_FAIL TRUE` leftover** on `er_api_progressive_precision` inverted
   ctest's verdict, so once the test *did* start passing it reported as *failed* —
   a phantom CI failure that cost real diagnosis time.

### 2.4 Process / CI mishaps

- **Instrumented CI jobs (ASan/UBSan/coverage, Debug `-O0`, ~40× slower)** ran the
  heavy L1 property-fuzz, taking ~1 h; the fuzz wasn't tiered by regression level
  (#1007).
- **ccache eviction** (unrelated to ereal, surfaced during the work) made CI
  builds ~3× slower (#1009) — a reminder that "CI got slow" is not always a code
  regression; it needed telemetry (build-step duration, hit-rate) to diagnose, not
  guesswork.

---

## 3. Root-cause themes

1. **"Looks like" is not "is."** The type *resembled* Priest arithmetic and the
   tests *resembled* validation. Both were accepted on resemblance. The single
   most expensive decision was implementing to an *association* with the Priest
   papers rather than to a *checked specification* (the normal-form predicate, the
   EFT exactness identities).

2. **Layered masking.** The bugs were stacked: fixing the constants (the obvious
   read of #1002) would have revealed the ~130-digit reciprocal ceiling, which
   would have revealed the parse cliff. Each defect hid the next, so any
   *partial* fix would have looked "done" while the type was still broken. Only an
   **end-to-end measurement against an independent oracle** could see through the
   whole stack at once.

3. **The hidden floor is always the exponent range.** Three separate problems —
   EFT product exactness, reciprocal accuracy, and parse — all bottomed out on
   *subnormal underflow* and the ~1022-exponent span of `double`. `1/10^300` cannot
   be multi-limb; `cfloat<24,5>`'s two_prod residual cannot be exact; a 300-digit
   `10^(-e)` cannot be represented. These are **representability limits of the
   format, not algorithm defects** — and conflating the two (as the original #942
   premise did) sends you chasing fixes that cannot exist.

4. **Ambiguous core semantics.** Whether `maxlimbs` bounds *inputs* or *results*
   was never decided, so neither the implementation nor the tests could reason
   about precision consistently.

---

## 4. What actually worked (the recovery methodology)

The recovery is worth codifying because it is reusable:

- **An independent exact oracle.** `verification/dyadic_exact.hpp` — an
  `einteger`-backed dyadic rational (`numerator · 2^scale`) — represents every
  double exactly and is closed under +,−,× with no rounding. Because it shares no
  code with `ereal`, `exact_value(a op b) == exact_value(a) op exact_value(b)`
  is a real proof, not a tautology (epic #987, Layer 2).
- **Layered verification:** EFT primitive identities (Layer 1) → exact-value
  conformance (Layer 2) → a line-by-line audit against the published Priest/
  Shewchuk definitions (Layer 3).
- **Honest measurement:** limb-injected MPFR/mpmath references generated *offline*
  (`tools/generators/ereal_reference_gen.py`), built from the *exact double* of
  each input, with correct ~15.95-digit/limb thresholds and no saturating cap.
- **Offline-generated constants** (the dd/qd approach): store pi/ln2/ln10 as
  precomputed non-overlapping limb expansions and reconstruct by exact summation,
  bypassing the lossy `parse()` entirely.
- **Diagnosis from telemetry, not guesswork:** the CI slowdown and the
  reciprocal/parse failures were all pinned by measuring (ccache hit rate, build
  step duration, residual-vs-magnitude curves, limb-by-limb comparison to MPFR),
  which repeatedly contradicted the "obvious" first hypothesis.

---

## 5. Introspective: the assistant's failure modes

The originating critique — *"too loose with associations; not including Priest
normal form is a major oversight that destroys trust"* — was accurate, and the
pattern recurred at smaller scale throughout:

- **Implementing to a remembered pattern instead of a checked spec.** The Priest
  normal form omission, and later the *assumption* that cfloat's `fma()` was a true
  fused op (#942), the *assumption* that "300-digit string ⇒ 300-digit reference,"
  and the initial #942 premise that odd-`p` was the sole cause — all were plausible
  associations that measurement falsified.
- **Trusting a passing test.** More than once a green test was taken as evidence of
  correctness when the test was self-referential or under-asserted. The correction
  was to ask *"would this test fail if the code were wrong?"* — and when the answer
  was no, to fix the test first.
- **Over-investigation vs. under-verification.** Counterweight lesson: once the
  data was in hand (e.g. parse perf, #913) the right move was to *conclude*, not
  keep digging — the user flagged this directly.

The thing that consistently worked was **forcing an independent measurement
before believing any claim** — limb-by-limb vs MPFR, dyadic oracle, ccache stats —
and **writing down the representability limits honestly** rather than reporting a
clean win where the format forbids one.

---

## 6. Recommendations

1. **Verify the invariants a type claims.** If the docs say "Priest normal form,"
   ship `check_priest_normal` and a conformance test on day one. A claimed property
   with no enforcement is a latent trust bomb.
2. **A test must be able to fail.** For every new test ask: what wrong
   implementation does this catch? If "none," it is documentation, not a test.
   Tolerances must be tighter than the bug class; references must be *independent*
   and *more precise* than the thing under test.
3. **Never validate high precision through a `double` or a self-reference.**
   Use an exact oracle (`dyadic_exact.hpp`) or offline arbitrary-precision
   references built from the exact computed input.
4. **Decide and document `maxlimbs` semantics** (input ceiling vs result cap) and
   make every operation and test consistent with it.
5. **Treat subnormal/exponent-range limits as first-class.** Document where a
   result *cannot* be exact (tiny reciprocals, narrow-`es` EFT residuals,
   out-of-range parse) instead of silently degrading or chasing impossible fixes.
6. **Don't let timing-sensitive or instrumented jobs gate, and tier fuzz by
   regression level** so the sanity tier stays fast.
7. **Prefer stored/offline-computed constants** over runtime string parsing for
   anything beyond ~16 digits, until `parse()` is rebuilt (still open: the parse
   architecture is patched, not redesigned).

---

## 7. Status

The correctness work is complete and on `main`: transcendentals scale to ~304
digits at `ereal<19>`, division/parse are correct across the magnitude range, the
overlap predicate is sound, and an independent exact oracle now guards the EFT and
expansion layers. The remaining debt is architectural, not correctness:
`parse()` is patched rather than redesigned (a true big-decimal → expansion parser
would remove the `double`-intermediate dependence), and the `maxlimbs` semantics
question is still unresolved.

*References:* `docs/bugs/ereal-failure-mode.md`,
`docs/bugs/ereal-priest-conformance-audit.md`,
`docs/multi-component/eft-operator-diagrams.md`,
`docs/sessions/2026-05-27-ereal-precision-ci-qd-bfloat16.md`,
`tools/generators/ereal_reference_gen.py`. Issues: #987 (epic), #999, #1002,
#1005, #1006, #913.
