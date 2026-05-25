# ereal / Priest verification failure mode and remediation plan

Status: remediation in progress (epic to be filed)
Type: process + verification-coverage defect, not a single code bug
Affected: `elastic/ereal/` test suite and `include/sw/universal/internal/expansion/`

## Summary

The `ereal` type implements Douglas Priest / Jonathan Shewchuk multi-component
floating-point expansion arithmetic. During the epic #944 effort to strengthen
its regression suite, a serious oversight surfaced: the implementation was
built and tested as something *associated with* Priest's algorithms (the
error-free-transformation primitives, the citations, the expansion structure)
without being pinned to the *theorems that define them*. The most visible
instance was that Priest normal form was not enforced anywhere until issue #954
added a structural oracle -- which in turn revealed a real bug (#981:
`expansion_product` never renormalized its accumulated result, producing
overlapping multi-limb products; fixed in #982).

Normal form was the symptom. The disease is structural in how verification was
conceived, described below.

## The disease: self-referential and structural-only verification

Of the three oracles the epic produced, **none checks the property that
defines why the algorithm exists** -- that an expansion carries the *exact*
mathematical result of an operation.

| Oracle | File | What it proves | What it cannot catch |
|--------|------|----------------|----------------------|
| Structural | `arithmetic/priest_normal_form.cpp` | result is in Priest normal form (decreasing magnitude, non-overlapping, no interior zeros) | a normalized expansion that encodes the **wrong value** |
| Precision-lifting | `arithmetic/precision_lifting.cpp` | `ereal<N>` agrees with `ereal<19>` | a **systematic value error in the shared algorithm** -- both sides run the same code and agree with each other |
| Projection | most fuzzers / function tests | result is close to a `double` reference within tolerance | error in the **extra precision** the expansion exists to carry (it is discarded by the projection) |

Consequence: there is currently **no test that asserts `a (op) b` equals the
exact mathematical result of `a op b`**. The precision-lifting oracle is
self-referential -- it proves *consistency between configurations*, not
*correctness*. The structural oracle proves *shape*, not *value*. The whole
purpose of a Priest expansion is to carry the exact result, and that property
is never checked exactly.

### Concrete unverified precondition (live example)

`two_prod` (the foundation of every expansion product) relies on `std::fma`
being exact:

```cpp
inline void two_prod(double a, double b, double& x, double& y) {
    volatile double vx = a * b;
    x = vx;
    volatile double vy = std::fma(a, b, -vx);   // assumes exact FMA
    y = vy;
}
```

The project's own notes record that MinGW's *software* `fma` is off by 1-2 ULP,
which silently breaks the error-free transformation. So `two_prod` is *assumed*
exact per platform, never proven. This is the same failure pattern at the
primitive level: a theorem ("`x + y == a*b` exactly") treated as an assumption.

## Root cause

When implementing a named algorithm, its published invariants and theorems
must be part of the *definition of done*. Here they were not: the
implementation was written from a reading of the source, and verification was
added afterward in whatever shape was convenient (structural checks,
self-comparison, double projection) rather than in the shape the literature
dictates (exact identities checked against an independent reference). An
unaided reading of the source is exactly what produced the normal-form
omission, so verification cannot rest on that same reading.

## Remediation plan: three layers, all tied to the literature

Chosen approach: **full plan, in order**, each layer as its own PR.

### Layer 1 -- Primitive exactness (the error-free transformations)

Prove the named theorems for the building blocks, on the actual build platform:

- `two_sum` / `fast_two_sum`: `s + e == a + b` with **zero** rounding.
- `two_prod`: `x + y == a*b` exactly.
- Assert `fast_two_sum`'s `|a| >= |b|` precondition holds at every call site
  (currently assumed).
- Resolve the `std::fma` (MinGW software-FMA) correctness risk explicitly.

These rest under everything else, so they are verified first.

### Layer 2 -- Independent exact-value oracle (the keystone)

The oracle must be exact and have **no external dependency** (this sidesteps
exactly what closed the MPFR-oracle idea, issue #955, as wontfix). Every
`double` is a *dyadic* rational -- `sign * mantissa * 2^exp` with integer
mantissa and exponent -- so an exact reference needs only big-integer
arithmetic and power-of-two scaling, never general division or gcd.

**Oracle foundation: BLOCKED -- both candidate big-number types are broken.**
The first instinct was to use the library's `erational` (adaptive rational).
Probing it before building on it -- the discipline this whole effort is about --
revealed `erational` is broken (#986): its `double` conversion ignores the
exponent, so `erational(2.0)` yields `1` and `1 + 2 == 3` is false. The
fallback was `einteger` (adaptive integer); a small probe (operands < 2^20)
looked clean, so a dyadic-rational oracle was built on it -- but a *deeper*
probe at full 53-bit operands found `einteger`'s multiply is also broken
(#991): `(2^32)*(2^32) != 2^64`, ~99.99% of 53-bit products wrong, due to a
carry-propagation defect at the limb boundary. The shallow first probe is
itself an instance of the under-testing failure mode this effort exists to
correct.

Consequence: there is currently **no trustworthy exact big-number type in the
library** to anchor the keystone oracle. The dyadic oracle design
(`include/sw/universal/verification/dyadic_exact.hpp`, numerator = big integer,
denominator = power of two) is correct and ready, but needs a sound integer
backend. Resolution options (maintainer decision):

1. **Fix `einteger` multiply (#991)** -- appears to be a contained carry bug; a
   patch is proposed in the issue. Then `einteger` becomes the oracle backend.
2. **`__int128`-bounded oracle** -- no library dependency; covers `two_prod`
   fully (106-bit products) and `two_sum` for bounded exponent gaps, with
   extreme-gap cases handled structurally. Limited range, more caveats.
3. **Fix `erational` (#986)** and use it.

Recommended: option 1 -- it is small, fixes a real shipped bug, and yields a
general exact backend usable by the whole verification effort.

The oracle:

1. inputs (doubles / ereal limbs) -> exact dyadic rationals,
2. compute `+`, `-`, `*` in exact dyadic arithmetic (exact and closed; `/` is
   handled separately since a quotient need not be dyadic and `ereal`'s own
   division is not claimed exact -- there the oracle checks a residual bound,
   not bit-exact equality),
3. convert the `ereal` result's components -> dyadic rationals and sum them,
4. assert **bit-exact equality** for `+`, `-`, `*`.

This reference shares no code with the expansion algorithms, so it can catch a
normalized-but-wrong value -- the class of bug nothing currently catches. This
is the artifact that directly answers "is this actually computing Priest
arithmetic, or just something shaped like it." Whatever it finds (errors, or
millions of clean cases) is an independent check the maintainer holds, not a
claim to be taken on faith.

### Layer 3 -- Line-by-line conformance audit

Map each implemented routine to its published figure and confirm correspondence
in writing:

- `grow_expansion`, `fast_expansion_sum`, `linear_expansion_sum`,
  `scale_expansion`, `priest_renormalize`, `expansion_product` vs the
  Shewchuk (1997) figures and Priest's normalization.
- Confirm the encoded normal-form predicate (`|z_{k+1}| <= ulp(z_k)/2`,
  decreasing magnitude, no interior zeros) is **literally Priest's published
  definition** with citation, not a paraphrase.

The resulting conformance table becomes the contract for the type.

## Discovery during remediation: two more broken core types

Verifying the intended oracle *before* building on it (the central discipline
of this effort) uncovered two independent, serious bugs in the library's
exact-arithmetic types. This both validates the discipline and shows the trust
problem is broader than `ereal`.

### `einteger` multiply is broken (#991)

`einteger::operator*=` mishandles carries at the limb boundary:
`(2^32)*(2^32) != 2^64`, and ~99.99% of random 53-bit x 53-bit products are
wrong. Single-limb and some multi-limb products are correct, so a shallow probe
(operands < 2^20) passed and nearly led to building the oracle on it -- exactly
the under-testing trap. Root cause: the carry accumulator is not reset per
outer-limb iteration and the final carry is flushed once instead of rippled per
row. Patch proposed in #991.

### `erational` is broken (#986)

`erational` (adaptive-precision rational) was found to be fundamentally broken:

- `erational::convert_ieee754` ignores the floating-point exponent: its "normal"
  branch sets `numerator = significand`, `denominator = 2^52` and never applies
  the `2^(exp - bias)` scaling; its subnormal branch is an empty `{}`.
- Result: every value collapses into `[1,2)`. `erational(2.0) -> 1`,
  `erational(3.0) -> 1.5`, `erational(0.1) -> 1.6`, `erational(0.0) -> NaN` with
  a thrown `divide_by_zero`, and `erational(1.0) + erational(2.0) == erational(3.0)`
  is **false**.
- It has no regression tests and is OFF in CI
  (`UNIVERSAL_BUILD_NUMBER_ERATIONALS ... OFF`).

This is the same failure pattern as the `ereal` issue (a type shipped without
its defining behavior verified) and should be filed and fixed on its own track.
It is the reason the Layer-2 oracle is built on `einteger` instead. Note: this
finding is itself a validation of the "probe before you build on it" discipline
this remediation is meant to instill.

## Process change

For any named algorithm, encode its published invariants/theorems as tests
**first**, with the citation, as part of definition-of-done -- rather than
discovering missing properties later when a fuzzer happens to trip over them.

## References

- D. M. Priest, "On Properties of Floating Point Arithmetics: Numerical
  Stability and the Cost of Accurate Computations," PhD thesis, 1992.
- J. R. Shewchuk, "Adaptive Precision Floating-Point Arithmetic and Fast Robust
  Geometric Predicates," Discrete & Computational Geometry, 1997.
- Epic #944 (ereal regression-suite refactor); issues #954 (structural oracle),
  #981 / #982 (`expansion_product` renormalization bug + fix), #955 (MPFR
  oracle, closed wontfix -- superseded by the erational oracle in Layer 2).
