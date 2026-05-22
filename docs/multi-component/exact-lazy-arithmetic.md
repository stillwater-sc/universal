# Lazy Exact Real Arithmetic in Universal

This document tracks Universal's lazy exact real arithmetic story. The
McCleeary-style `elreal` type is currently being implemented from
scratch following the dissertation; a previous attempt that mixed
Shewchuk-style multi-component storage with McCleeary-style lazy
materialisation was backed out because the architectural mismatch broke
the non-overlap property at depth 3+.

## Reference

The implementation follows:

> McCleeary, R. (2019). *Lazy Exact Real Arithmetic Using Floating Point
> Operations*. Ph.D. dissertation, University of Iowa.
> <https://iro.uiowa.edu/esploro/outputs/doctoral/Lazy-exact-real-arithmetic-using-floating/9983776998202771>

In particular, Chapter 4 of the dissertation specifies the LFPERA
algorithm: a co-list of k-bit floating-point blocks (`ZBCL_k`) with the
**0-overlap (gap) property** `e_n >= e_{n+1} + k`. The gap is what makes
lazy left-to-right block emission correct without a renormalisation
sweep.

## Why lazy

Fixed-precision arithmetic systems (IEEE `double`, `dd`, `qd`, even
adaptive `ereal<N>`) commit to a precision target at type-instantiation
time. McCleeary's contribution is a true **lazy** exact-real system: the
user specifies precision *at the end of the computation*, and the
algorithms walk just deep enough into the input streams to produce that
many bits of output.

This is qualitatively different from "deeper expansion on demand".
McCleeary's algorithms emit blocks left-to-right and never need to
revisit an already-emitted block, because the gap property guarantees
that downstream blocks cannot carry up far enough to affect what was
already output. That is what makes the system a *true* lazy real, not
a forwarded-precision approximation.

## Applications

The use cases that motivate exact-real arithmetic:

1. **Computational geometry** -- robust predicates (orient, incircle,
   insphere) for mesh generation, CAD/CAM, robotics. Near-degenerate
   configurations resolve correctly without bespoke tolerance tuning.
2. **Symbolic computation** -- comparing reals constructed via different
   algebraic paths. Termination guarantees in CAS-style systems.
3. **Verified numerical computation** -- iterative refinement with
   on-demand residual precision; interval-bound + lazy-refined-inner
   combinations.
4. **Cross-implementation validation** -- a higher-precision reference
   to validate against, without committing to MPFR or another external
   dependency.

## Status

The McCleeary `elreal` implementation is in development. Track progress
on the rewrite epic (filed alongside this document on the same branch
that backed out the prior implementation).

When the new `elreal` ships, the API will follow the dissertation
naming: blocks of a configurable bit width, co-lists with the 0-overlap
gap, `threeAdd`-style primitives, and the precision-on-demand contract.

## Related documents

- `docs/algorithmic-details/multi-component-arithmetic.md` -- broader
  Priest / Bailey-Hida / Shewchuk / McCleeary landscape
- `docs/multi-component/comparison-priest-bailey-shewchuk.md` -- earlier
  reference document, predates the McCleeary thread

## References

- **McCleeary, R.** (2019). *Lazy Exact Real Arithmetic Using Floating
  Point Operations*. Ph.D. dissertation, University of Iowa.
- **Priest, D. M.** (1991). *On Properties of Floating Point Arithmetics:
  Numerical Stability and the Cost of Accurate Computations*. Ph.D.
  dissertation, University of California, Berkeley.
- **Hida, Y., Li, X. S., Bailey, D. H.** (2000). "Algorithms for
  Quad-Double Precision Floating Point Arithmetic." *Proc. 15th IEEE
  Symposium on Computer Arithmetic*, 155-162.
- **Shewchuk, J. R.** (1997). "Adaptive Precision Floating-Point
  Arithmetic and Fast Robust Geometric Predicates." *Discrete &
  Computational Geometry*, 18(3), 305-363.
