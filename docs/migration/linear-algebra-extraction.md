# Linear-algebra extraction (Universal -> MTL5 + mp-*)

Universal's BLAS (`include/sw/blas/`) and container types
(`include/sw/numeric/containers/`) were built to study posit/quire dynamics.
That functionality is now superseded by **MTL5**
(https://github.com/stillwater-sc/mtl5), a standalone C++20 linear-algebra
library, and the mixed-precision *experiments* have moved to the **mp-*
composition repos**. Tracking epic: universal#1204.

## Where things moved

| Was in Universal | Now |
|------------------|-----|
| `blas/` L1/L2/L3, solvers, generators, vmath, utes | MTL5 (`MTL5::mtl5`) -- already present; the few missing utilities (`linspace`, `magic`) were added |
| `numeric/containers/` (matrix/vector/tensor) | MTL5 containers (`mtl::mat`, `mtl::vec`) |
| dense LU iterative refinement (`ext/solvers/luir`, `squeeze`, `nbe`) | MTL5 `mtl::lu_iterative_refine` + `mtl::normwise_backward_error`; the posit/precision experiments in **mp-ir** |
| fdp / quire reproducibility studies, mpdot/mpfma | **mp-blas** (via the quire accumulator bridge) |
| stationary + CG iterative-method studies | **mp-iterative** |
| numerics studies (roots/integration/interpolation/optimization/approximation) | **mp-numerics** |

## Architectural rule

MTL5 never depends on Universal -- it is the general linear-algebra layer,
generic over the scalar type. All MTL5 + Universal coupling (the quire
super-accumulator plugged into MTL5's `accumulator_traits`) lives in the `mp-*`
repos, via `include/mtl/math/quire_accumulator.hpp`.

## Deprecation

`<blas/...>` and `<numeric/containers/...>` are **deprecated** and emit a
`#pragma message` on include (silence with `-DUNIVERSAL_SUPPRESS_DEPRECATION`).
They will be removed from Universal after this deprecation release, leaving
Universal a pure number-systems library. New linear-algebra work should target
MTL5 + the mp-* repos.
