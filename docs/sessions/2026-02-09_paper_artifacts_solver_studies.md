# Development Session: Paper Artifact Tree & Mixed-Precision Solver Case Studies

**Date:** 2026-02-09 (session 2)
**Branch:** v3.96
**Focus:** Create self-contained papers/ tree with three solver case studies
**Status:** Complete

## Session Overview

Created a new top-level `papers/` directory to house all code artifacts for
the two planned publications (arXiv systems paper, IEEE CSE position paper).
Developed three mixed-precision solver case studies that demonstrate Universal's
number system inventory across direct and iterative methods.

### Goals Achieved
- Reorganized paper docs from `docs/papers/` to `papers/docs/`
- Created `papers/systems-paper/` with CMake wiring (`UNIVERSAL_BUILD_PAPERS`)
- Implemented three solver expositions: LU-IR, CG, IDR(s)
- All programs build, run, and produce publication-quality tabular output

## Commits

| Hash | Description |
|------|-------------|
| `0ec0002a` | Add papers/ artifact tree with three mixed-precision solver case studies |

## Key Decisions

### Why `papers/` at repo root?
Three options were considered: (A) `papers/` at root, (B) `applications/reproducibility/papers/`,
(C) hybrid with symlinks. Option A was chosen because it gives reviewers a single
directory to zip and share, with all code and documentation co-located.

### Why self-contained solvers (not library headers)?
The solver implementations (PLU factor, CG iteration, IDR(s) loop) are embedded
directly in each `.cpp` rather than calling library solver headers. This avoids
template instantiation issues with cross-family type mixing (e.g., `posit` low
precision + `cfloat` working precision) and makes each file independently readable
for paper reviewers.

## Case Study 1: Iterative Refinement (`iterative_refinement.cpp`)

Carson & Higham three-precision LU-IR:
- **Low** precision: LU factorization (cheapest tier)
- **Working** precision: triangular solves
- **High** precision: residual computation

Selected results (N=50, kappa~1013):

| Config | Low | Working | High | Iters | Fwd Error |
|--------|-----|---------|------|-------|-----------|
| IEEE-1 | half | float | double | 5 | converged |
| IEEE-2 | bfloat16 | float | double | DNF | 9.54e-7 |
| Posit-1 | posit<16,1> | posit<32,2> | posit<64,3> | 3 | converged |
| X-2 | posit<16,1> | float | dd | 3 | converged |

Key insight: posit<16,1> outperforms half for low-precision factorization
because it has more significand bits near 1.0.

## Case Study 2: Conjugate Gradient (`conjugate_gradient.cpp`)

Preconditioned CG on tridiag(-1,2,-1) with Jacobi preconditioner.

Selected results (N=32, kappa~415):

| Type | Iters | Fwd Error |
|------|-------|-----------|
| half | DNF | 9.77e-4 |
| bfloat16 | 108 | 3.12e-2 |
| float | 48 | 3.58e-7 |
| double | 16 | 6.66e-16 |
| posit<32,2> | 46 | 8.20e-8 |
| dd | 16 | 1.11e-15 |

Two-precision finding: half-precision Jacobi preconditioner + float solver
converges identically to full-float CG (48 iterations). The preconditioner
only needs to approximate A^{-1} — precision doesn't matter much.

## Case Study 3: IDR(s) (`idrs.cpp`)

IDR(s) on non-symmetric convection-diffusion (eps=0.5): CG not applicable.

Shadow space sweep (double, N=32):

| s | Iters |
|---|-------|
| 1 | 272 |
| 2 | 103 |
| 4 | 112 |
| 8 | 49 |

Number system comparison (s=4):

| Type | Iters | Fwd Error |
|------|-------|-----------|
| half | DNF | 3.04e-1 |
| bfloat16 | DNF | diverged |
| float | 87 | 3.72e-5 |
| double | 112 | 1.41e-10 |
| posit<32,2> | 167 | 2.79e-5 |
| dd | 92 | 1.49e-12 |

Key insight: non-symmetric problems are much harder on low-precision types.
bfloat16 diverges completely (residual 1.3e+6). Float converges faster than
double (87 vs 112) — a known phenomenon where rounding helps escape local
minima in the non-normal residual landscape.

## Technical Notes

### Cross-type conversion
Different Universal number system families don't have implicit conversion
paths between each other (e.g., `dd` has no constructor from `cfloat<32,8>`).
The `convert_matrix<Dst,Src>()` and `convert_vector<Dst,Src>()` helpers route
all conversions through `double` as an intermediary.

### bfloat_t not bfloat16
The Universal namespace uses `bfloat_t` (not `bfloat16`) as the type alias
for `cfloat<16, 8, uint16_t, true, false, false>`.

## File Layout

```
papers/
  systems-paper/
    iterative_refinement.cpp    # Carson & Higham LU-IR
    conjugate_gradient.cpp      # CG for SPD systems
    idrs.cpp                    # IDR(s) for non-symmetric systems
    CMakeLists.txt
  position-paper/
    CMakeLists.txt              # empty, ready for future code
  docs/
    arxiv-systems-paper.md      # systems paper roadmap
    arxiv-position-paper.md     # position paper roadmap
    STATUS.md                   # two-paper strategy tracker
    implementation-roadmap.md
    position-paper-draft.md
    position-paper-outline.md
```

## Next Steps

1. Add CSV output mode to all three programs (for LaTeX table generation)
2. Create energy estimation benchmark for the systems paper
3. Expand test matrices beyond tridiagonal (Hilbert, random SVD, Laplace2D)
4. Write LaTeX paper draft using these three case studies as Section 4
