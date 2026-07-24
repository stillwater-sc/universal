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

## Removal

`<blas/...>`, `<numeric/containers/...>`, the DNN subsystem
(`<universal/dnn/...>`), and `<universal/quantization/qsnr.hpp>` were
**deprecated** in the v4.7.5 release (they emitted a `#pragma message` on
include) and have now been **removed** -- Universal is a pure number-systems
library. New linear-algebra work targets MTL5 + the mp-* repos.

A handful of BLAS-dependent demos are kept as source-only references (dropped
from the build, so they carry dangling `<blas/...>` includes): the
`mixedprecision/tensor/cg` study (rebuilt in mp-iterative, see mp-iterative#32),
`papers/systems-paper/{iterative_refinement,conjugate_gradient,idrs}.cpp`, and
`playground/conversion.cpp`. Rebuild them against MTL5 in the mp-* repos.
