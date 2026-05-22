# Exact Lazy Arithmetic in Universal

This document is the design narrative for **lazy exact real arithmetic** as
implemented by Universal's `elreal` type. The type ships as the
McCleeary-paradigm tier in Universal's multi-component arithmetic family,
complementing the Priest-foundation EFTs, the Bailey/Hida fixed-precision
`dd`/`qd`, and the Shewchuk-style adaptive `ereal<N>`.

For the shipped API, see:

- `docs/number-systems/elreal.md` -- user-facing API reference
- `docs/algorithmic-details/lazy-real-arithmetic.md` -- algorithmic deep-dive
  (lazy-stream representation, refinement protocol, sign-determination
  proof, per-phase delivery)
- `docs/algorithmic-details/multi-component-arithmetic.md` -- broader
  Priest/Bailey-Hida/Shewchuk/McCleeary landscape

This document focuses on **why lazy** rather than **how lazy is
implemented**.

---

## 1. The Ryan McCleeary contribution

**Dissertation**: "Lazy Exact Real Arithmetic Using Floating Point Operations"
**Author**: Ryan McCleeary
**Institution**: University of Iowa
**Year**: 2019
**URL**: <https://iro.uiowa.edu/esploro/outputs/doctoral/Lazy-exact-real-arithmetic-using-floating/9983776998202771>

McCleeary's dissertation argues that lazy exact real arithmetic over
floating-point primitives is a practical foundation for problems where
fixed-precision arithmetic cannot give a definite answer. The three core
ideas:

1. **Stream representation.** A real number is represented as a sequence
   of progressively refined floating-point approximations -- conceptually
   infinite, materialised only as deep as the caller asks.
2. **Hardware-leveraged.** Each stream element is an IEEE-754 `double`.
   The lazy machinery is pure C++; no extended-precision hardware required.
3. **Decidable sign with bounded budget.** Comparison of two lazy reals
   walks the streams in lockstep until a non-zero difference is found.
   The Shewchuk non-overlapping property bounds the work per refinement
   step; a per-call budget bounds the worst case.

The fourth idea -- correctness proofs for the core operations under finite
precision -- is what distinguishes McCleeary's treatment from earlier
lazy-real implementations. The dissertation provides formal arguments that
the operations remain exact in the limit of unbounded refinement.

---

## 2. Why lazy

Traditional exact real arithmetic systems face one of three pain points:

- **Symbolic systems** (e.g. computer algebra over rationals) are
  mathematically exact but often prohibitively slow for analytic
  computation.
- **Fixed-precision arithmetic** (`double`, `dd`, `qd`) is fast but limited
  to its committed precision. For ill-conditioned problems, the precision
  budget is fixed at compile time.
- **Adaptive precision** (Shewchuk's expansions, Universal's `ereal<N>`) is
  flexible but requires upfront error analysis. The user must know how many
  components might be needed before instantiating the type.

Lazy exact arithmetic adds a fourth option:

- **Computation proceeds on-demand** -- only as much precision as needed
  for the *current call*.
- **Results are exact within the refinement budget** -- no rounding errors
  accumulate.
- **Stream-based evaluation** -- precision can grow per-call without
  committing to a worst-case budget at type-declaration time.
- **Hardware-accelerated** -- uses native floating-point operations as
  the building block.

For exact-real workloads in which the *common case* is general-position
inputs that resolve quickly, the lazy paradigm wins on cost: the common
case is depth-0 work, with refinement spent only on the inputs that
actually need it.

---

## 3. Applications

Exact real arithmetic is critical for:

1. **Computational Geometry**
   - Robust geometric predicates (orient, incircle, insphere)
   - Mesh generation and CAD/CAM systems
   - Delaunay triangulation that does not loop on near-degenerate inputs

2. **Symbolic Computation**
   - Deciding equality of reals constructed via different algebraic paths
   - Termination guarantees in computer-algebra systems

3. **Numerical Analysis with Correctness Targets**
   - Iterative refinement with on-demand residual precision
   - Verified numerical computation (interval arithmetic outer bound +
     elreal refinement inner bound)

4. **Cross-Implementation Validation**
   - Validating that a multi-component arithmetic implementation
     (`dd`, `qd`, `dd_cascade`, etc.) matches an independent reference
     -- catching the class of bugs a self-comparison cannot

The geometric-predicate use case is the canonical one. Shewchuk's 1996
dissertation argued that exact arithmetic is needed for any geometric
algorithm that branches on sign; McCleeary's lazy paradigm makes the
cost of that exactness pay-as-you-go.

---

## 4. The shipped implementation: `elreal`

Universal's `elreal` (epic #873, shipped 2026 via PRs #883-#900) is the
McCleeary-paradigm tier:

- **Storage**: `std::vector<double>` of materialised components plus a
  `std::function<double(std::size_t)>` generator.
- **Refinement**: `at(k)` and `refine_to(precision_bits)` walk the generator
  on demand.
- **Sign determination**: per-call refinement budget (default 8 components,
  ~424 bits cumulative); `sign(v, budget)` walks the stream to the first
  non-zero component.
- **Arithmetic**: depth-1 EFT/Taylor refinement for `+ - * /` (the `/`
  depth-1 generator landed in Phase L.1, follow-up epic #903). Depth-2+
  Newton refinement for division and sqrt is Phase L.2 follow-up.
- **Math**: depth-1 derivative-based refinement for the full
  exp/log/pow/hyperbolic/trig family.
- **Geometric predicates**: `orient2d`, `orient3d`, `incircle`, `insphere`.
- **Validation oracle**: cross-implementation validation helper for the
  other multi-component types.

For the full API and per-phase breakdown, see
`docs/number-systems/elreal.md`.

---

## 5. Comparison with the other multi-component approaches

| Approach | When it wins | When it loses |
|---|---|---|
| **Priest EFTs** | foundation only -- not a user-facing type | n/a |
| **Bailey/Hida `dd`/`qd`** | hot loops at known precision (106 or 212 bits); SIMD-friendly | adaptive precision; undecidable comparison |
| **Shewchuk `ereal`** | adaptive precision with a committed upper bound; eager expansion | extreme precision-on-demand; lazy refinement of common cases |
| **McCleeary `elreal`** | general-position dominates; undecidable comparison; validation oracle | hot loops at committed precision (Bailey/Hida wins on raw speed) |

The four approaches are complementary, not competing. Universal ships all
four (Bailey/Hida via `dd`, `qd`, and the cascade-based rewrites; Shewchuk
via `ereal<N>`; McCleeary via `elreal`; with Priest as the shared
foundation underneath all of them).

---

## 6. Known limitations of the shipped `elreal`

Phase G of the epic completed in 2026. Outstanding work as of that
milestone:

- **Depth-2+ refinement.** Arithmetic operators and math functions refine
  to depth 1. Deeper refinement requires Newton iteration (for `/` and
  `sqrt`) or a higher-precision internal algorithm. The lazy-stream
  infrastructure supports arbitrary depth; only the per-function generators
  need extending.
- **Constants stop at 4 components (~212 bits).** `elreal_pi` and friends
  reuse the precomputed expansions from `qd_constants.hpp`. BBP-style
  on-demand bit extraction for pi, Taylor truncation for e/ln2 with lazy
  refinement -- both follow-up work.
- **Forward trig range reduction.** `sin/cos/tan` use the std library for
  depth 0; large-magnitude arguments lose precision. Payne-Hanek range
  reduction with lazy pi-pull is documented in `elreal_impl.hpp` as the
  path forward.
- **Oracle precision ceiling.** The validation-oracle helper caps at
  double precision today (limited by elreal's depth-1). When deeper
  refinement lands, the oracle's precision improves transparently --
  the helper's API doesn't change.

---

## 7. Future directions

Beyond the depth-2+ work, two longer-term directions for exact-real
arithmetic in Universal:

- **Verified numerical computation.** Pair `elreal` with `interval` to
  get a rigorous outer-bound + lazy-refined inner-bound representation.
  This is the foundation for verified scientific computing.
- **Mixed-precision oracle.** Extend the validation-oracle helper to be
  the standard reference for *all* multi-component math function tests
  across the library. Today's helper is a demonstration on `dd_cascade`;
  the natural follow-up is a sweep across `dd`, `qd`, the cascades, and
  `ereal<N>`.

---

## References

### Foundational papers

1. **McCleeary, R.** (2019). *Lazy Exact Real Arithmetic Using Floating
   Point Operations*. Ph.D. dissertation, University of Iowa.
   <https://iro.uiowa.edu/esploro/outputs/doctoral/Lazy-exact-real-arithmetic-using-floating/9983776998202771>
2. **Shewchuk, J. R.** (1997). "Adaptive Precision Floating-Point
   Arithmetic and Fast Robust Geometric Predicates." *Discrete &
   Computational Geometry*, 18(3), 305-363.
   <https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf>
3. **Priest, D. M.** (1991). *On Properties of Floating Point Arithmetics:
   Numerical Stability and the Cost of Accurate Computations*. Ph.D.
   dissertation, University of California, Berkeley.
4. **Hida, Y., Li, X. S., Bailey, D. H.** (2000). "Algorithms for
   Quad-Double Precision Floating Point Arithmetic." *Proc. 15th IEEE
   Symposium on Computer Arithmetic*, 155-162.
5. **Payne, M., Hanek, R.** (1983). "Radian Reduction for Trigonometric
   Functions." *SIGNUM Newsletter*, 18(1), 19-24.

### Universal library documentation

- `docs/number-systems/elreal.md` -- user-facing API reference for `elreal`
- `docs/algorithmic-details/lazy-real-arithmetic.md` -- algorithmic
  deep-dive on the lazy-stream paradigm
- `docs/algorithmic-details/multi-component-arithmetic.md` -- broader
  Priest/Bailey-Hida/Shewchuk/McCleeary comparison
- `docs/multi-component/comparison-priest-bailey-shewchuk.md` -- earlier
  reference document, superseded by the algorithmic-details version
