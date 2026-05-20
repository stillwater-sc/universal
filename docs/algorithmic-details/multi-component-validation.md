# Multi-Component algorithm documentation validation

## Audit summary — what was found and corrected

Major fabrications (multi-component doc section 5, "Universal's unification"):
- FloatCascade<Limb, N> (templated on limb type) → actual is floatcascade<N> (lowercase, double-only)
- EFTs as static members of the cascade template → actual are free functions in error_free_ops.hpp
- VariableCascade<Limb> class → does not exist
- AdaptivePrecision config_ member on ereal → does not exist
- ereal::to_dd() / to_qd() / to_td() extraction helpers → do not exist
- Implicit dd↔dd_cascade↔ereal conversions → do not exist
- elrealo type in the picker → does not exist (only mentioned in a tutorial as a future name)
- Cascade math at internal/floatcascade/math/ → does not exist; lives per-type at number/<type>/math/

Code-snippet inaccuracies (section 3, Bailey/Hida):
- dd operator+= snippet had std::isfinite (actual: is_finite_cx) and missing the else { lo = 0.0; } branch; missing the CONSTEXPRESSION macro
- "dd multiplication uses six two_prod invocations" → actual is three two_prod plus one plain product
- "qd_mul ~25 EFTs total" → actual is 6 two_prod for approximate_multiplication, 10 for accurate_multiplication
- two_prod shown with fma(a, b, -p) idiom → actual uses split-form by default, FMS via RELIABLE_FUSED_MULTIPLY_SUBTRACT_OPERATOR macro guard

Minor fixes (LNS doc):
- ArnoldBailey tail described as flat 0 below -6 → actually a secant ramp on [-6, -5] then exact 0
- SplitJ default written as min((f+4)/2, 10) → actually clamp((f+4)/2, 2, 10) (missing lower clamp)

## Updates

README + Starlight index: FloatCascade<N> → floatcascade<N>

For each EFT, the pseudo-algorithm in a text block followed by the actual Universal implementation
(runtime branch) in a cpp block — verbatim from error_free_ops.hpp. Section 5 has a new "what is and is not unified today" subsection that distinguishes
the aspirational design in floatcascade-design.md from the shipped code.

Why so many fabrications. The same root cause as the MPFR claim: I drafted from general knowledge of how multi-component libraries are typically organized,
lifted snippets from older docs without verifying they still match the code (the original comparison-priest-bailey-shewchuk.md had similar inaccuracies),
and didn't read floatcascade.hpp or ereal_impl.hpp before claiming what their APIs looked like. The lesson — for me to apply going forward — is: when
writing docs that quote APIs, read the header first; do not lift snippets from existing docs without verifying them against current code.

