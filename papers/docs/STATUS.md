# Paper Status

**Last Updated**: 2026-02-09

## Two-Paper Strategy

The mixed-precision work has been bifurcated into two complementary papers:

| Paper | Venue | Focus | Roadmap |
|-------|-------|-------|---------|
| **Systems Paper** | arXiv cs.MS | Library architecture, block formats, benchmarks, SDK | `arxiv-systems-paper.md` |
| **Position Paper** | IEEE Computing in Science & Engineering | Mixed-precision as paradigm for physical-world computing | `arxiv-position-paper.md` |

**Submission order**: Systems paper first (arXiv, no review delay), then position
paper cites the arXiv paper for technical depth.

## Document Inventory

| Document | Status | Purpose |
|----------|--------|---------|
| `position-paper-analysis.md` | Complete | Bailey analysis, library inventory, gap analysis |
| `position-paper-outline.md` | Complete | Original outline (position paper) |
| `position-paper-draft.md` | v0.2, 40% | Draft prose — needs restructuring per new plan |
| `implementation-roadmap.md` | **Stale** | Original SDK roadmap — most items now complete |
| `arxiv-systems-paper.md` | Complete | Roadmap for the arXiv systems paper |
| `arxiv-position-paper.md` | Complete | Roadmap for the IEEE CSE position paper |

## Infrastructure Status (Feb 2026)

The original `implementation-roadmap.md` listed many SDK components as missing.
As of February 2026, the picture has changed dramatically:

| Component | Original Assessment | Current State |
|-----------|-------------------|---------------|
| Energy cost models | 5% | **Done** — 10 architectures |
| RAPL integration | Blocked | **Done** — Linux sysfs |
| Range analyzer | Missing | **Done** — 372 LOC |
| Type advisor | Missing | **Done** — 344 LOC |
| Algorithm profiler | Missing | **Done** — 499 LOC |
| Pareto explorer | Missing | **Done** — 794 LOC |
| Autotuner | Missing | **Done** — 525 LOC |
| Memory profiler | Missing | **Done** — 433 LOC |
| Instrumented types | Missing | **Done** — 641 LOC |
| Mixed-precision BLAS | Not mentioned | **Done** — mp_dot, mp_gemm |
| Block formats | Not mentioned | **Done** — mxblock, nvblock, zfpblock, zfparray |
| Quantization metrics | qsnr only | **Done** — RMSE, SNR, QSNR |
| Block format benchmarks | Not mentioned | **Done** — quantization_error, throughput |
| Methodology examples | Not mentioned | **Done** — 19 programs |

**The blocking items from the original roadmap are resolved.** Both papers can
now be written with real data from implemented infrastructure.

## Next Actions

### Systems Paper (arXiv)
1. [ ] Add CSV output mode to benchmarks (reproducibility)
2. [ ] Create energy estimation benchmark
3. [ ] Validate against reference implementations (microxcaling, LLNL ZFP)
4. [ ] Expand test signals (Gaussian, speech-like, image patch)
5. [ ] Write LaTeX paper

### Position Paper (IEEE CSE)
1. [ ] Rewrite introduction with "why now" (block format explosion)
2. [ ] Flesh out 4K vision energy worked example
3. [ ] Run robotics pipeline with instrumented types for case study
4. [ ] Literature survey (15-20 references)
5. [ ] Write LaTeX paper
