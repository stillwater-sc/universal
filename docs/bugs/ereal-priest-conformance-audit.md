# ereal / Shewchuk-Priest conformance audit

Status: Layer 3 of epic #987 (the final layer)
Scope: `include/sw/universal/internal/expansion/expansion_ops.hpp` and the
Priest-normal predicate in `include/sw/universal/verification/ereal_test_support.hpp`
References: Shewchuk (1997); Priest (1992) -- see end.

## Purpose

Layers 1 and 2 proved the implementation is *correct* by independent oracle:
Layer 1 (#988) proved the error-free transformations exact; Layer 2 (#989)
proved `ereal` `+`,`-`,`*` reproduce the exact mathematical value. This audit
closes the loop by mapping each routine to its published algorithm, recording
where the implementation matches the literature literally and where it diverges
(with justification), and confirming the encoded Priest-normal predicate is the
published definition rather than a paraphrase.

Method: read each routine against the cited figure; classify as *literal match*,
*behavioral conformance* (different code, theorem still holds -- and is proven by
Layers 1-2), or *deviation* (record it). Line numbers are as of this audit.

## What `ereal` actually calls

The hot path is narrow, so the audit weights these most:

- `operator+= / -= ` -> `renormalize_expansion(linear_expansion_sum(a, b))`  (ereal_impl.hpp:244,261)
- `operator*`        -> `expansion_product(a, b)` -> `scale_expansion` + `linear_expansion_sum` + `renormalize_expansion`  (ereal_impl.hpp:273)
- division           -> `e * reciprocal(f)`, reciprocal by Newton iteration (inexact by construction)

`grow_expansion` and `fast_expansion_sum` are provided but are NOT on the ereal
hot path.

## Conformance table

| Routine | Cited source | Implementation reality | Classification |
|---------|--------------|------------------------|----------------|
| `two_sum` | Knuth/Dekker, 6 flops | literal 6-op TwoSum (x=a+b; bv=x-a; av=x-bv; br=b-bv; ar=a-av; y=ar+br) | **Literal match.** Proven exact, Layer 1. |
| `fast_two_sum` | Dekker, 3 flops, requires `\|a\|>=\|b\|` | literal (x=a+b; y=b-(x-a)) | **Literal match.** Proven exact + precondition shown load-bearing, Layer 1. |
| `two_prod` | Dekker + FMA | literal (x=a*b; y=fma(a,b,-x)) | **Literal match.** Proven exact; also confirms platform `fma` is correctly rounded, Layer 1. |
| `grow_expansion` | Shewchuk Fig 6 | TwoSum sweep, least- to most-significant | **Literal match** to GrowExpansion. |
| `renormalize_expansion` | Priest 1992 renormalize | sweepUp -> take leading -> recurse on tail -> remove zeros -> sweepDown -> prepend | **Behavioral conformance** to Priest's canonical renormalization (the sweepUp/recurse/sweepDown structure). Restored associativity in #959/#961. Output proven Priest-normal (#954) and value-preserving (Layer 2). |
| `scale_expansion` | Shewchuk Fig 9 | scale each component by `b` via `two_prod`, collect products+errors, renormalize | **Behavioral conformance** (collect-then-renormalize rather than the interleaved Fig 9 emission; same exact result, proven Layer 2). |
| `expansion_product` | Shewchuk adaptive product | sum of `scale_expansion(f, e_i)` via `linear_expansion_sum`, then `renormalize_expansion` | **Behavioral conformance.** Standard O(mn) product; final renormalize added in #981/#982; proven value-exact, Layer 2. |
| `linear_expansion_sum` | "Shewchuk Figure 7" | magnitude-merge of the two inputs accumulating with **TwoSum**, dropping zero residuals, reversed to decreasing order | **Deviation (citation).** Not a literal transcription of Fig 7's (Q,q) recurrence; it is a TwoSum merge-by-magnitude. Output is non-overlapping and the value is exact (proven Layer 2), and ereal always follows it with `renormalize_expansion`, so the *behavior* conforms -- but the "Figure 7" citation overstates the correspondence. |
| `fast_expansion_sum` | "Shewchuk Figure 8 / FAST-TWO-SUM" | magnitude-merge accumulating with **TwoSum** (not FastTwoSum), dropping zeros | **Deviation (citation).** The comment claims Figure 8 and FAST-TWO-SUM, but the code uses TWO-SUM and a magnitude merge. Not on the ereal hot path. The citation is inaccurate and should be corrected (see below). |

## Priest-normal predicate

`check_priest_normal` (ereal_test_support.hpp) enforces, for a non-zero
expansion `z_0, ..., z_{m-1}`:

1. decreasing magnitude: `|z_k| >= |z_{k+1}|`
2. non-overlapping: `|z_{k+1}| <= ulp(z_k)/2`, with `ulp(z_k)/2 == 2^(ilogb(z_k) - 53)`
3. no interior zeros (canonical zero is the single limb `{0.0}`)

This is the standard non-overlapping normal form (Shewchuk Sec 2; Priest's
normalization). Condition 2 states that `z_{k+1}` lies entirely below the unit-
in-the-last-place of `z_k`, i.e. the two components share no binary digit
positions -- exactly Priest's non-overlapping requirement for a normalized
expansion. The predicate is therefore the published definition, not a
paraphrase. (It is intentionally the strong, gap-of-at-least-one-ulp/2 form;
any expansion satisfying it is non-overlapping in Shewchuk's sense.)

## Findings

1. **Primitives are literal and proven** -- the foundation matches Knuth/Dekker
   exactly and is verified exact (Layer 1).
2. **The ereal hot path is behaviorally conformant and proven value-exact** --
   `linear_expansion_sum` + `renormalize_expansion` for add, and
   `expansion_product` for multiply, produce exact, Priest-normal results
   (Layers 1-2, #954, #981/#982), even though the sum routines are TwoSum
   merge variants rather than literal Fig 7/8 transcriptions.
3. **Two citation inaccuracies** (documentation only, no behavioral impact):
   - `linear_expansion_sum` cites "Figure 7" but implements a TwoSum
     merge-by-magnitude.
   - `fast_expansion_sum` cites "Figure 8 / FAST-TWO-SUM" but uses TWO-SUM.
   These should be reworded to "based on / in the spirit of" the figures, or to
   state plainly that they are TwoSum merges whose canonical form is delegated
   to `renormalize_expansion`. (Corrected in this PR.)

## Conclusion

The implementation conforms to Shewchuk/Priest expansion arithmetic. Where the
code is a literal transcription (the primitives, `grow_expansion`), it is
verified literal; where it diverges in structure (the sum routines), the
divergence is benign and the governing theorems are independently proven to hold
(Layers 1-2). The encoded Priest-normal predicate is the published definition.
The only defects found are two inaccurate figure citations, corrected here.

## References

- J. R. Shewchuk, "Adaptive Precision Floating-Point Arithmetic and Fast Robust
  Geometric Predicates," Discrete & Computational Geometry 18:305-363, 1997.
  (Figures 6-9: GrowExpansion, LinearExpansionSum, FastExpansionSum,
  ScaleExpansion.)
- D. M. Priest, "On Properties of Floating Point Arithmetics: Numerical
  Stability and the Cost of Accurate Computations," PhD thesis, UC Berkeley,
  1992. (Renormalization to canonical non-overlapping form.)
- Companion: `docs/bugs/ereal-failure-mode.md` (the assessment that motivated
  epic #987); Layers 1-2 tests `internal/expansion/primitives/eft_exactness.cpp`
  and `elastic/ereal/arithmetic/exact_value_oracle.cpp`.
