# ArXiv Paper: Implementation Roadmap

**Working Title**: "Universal: A C++ Template Library for Mixed-Precision Algorithm Design with Block Floating-Point Formats"

**Target**: arXiv cs.MS (Mathematical Software) / cs.AR (Hardware Architecture)

**Status**: Analysis complete, roadmap defined

---

## Part 1: Critical Assessment of Current State

### What Has Changed Since the Original Roadmap

The `docs/papers/STATUS.md` (dated 2025-02-03) and `implementation-roadmap.md` are
**stale**. Significant work has been completed since:

| Component | STATUS.md Assessment | Actual State (Feb 2026) |
|-----------|---------------------|------------------------|
| Energy cost models | "5% complete" | **Implemented** — 10 architectures, 2,409 LOC |
| RAPL integration | "Blocked" | **Implemented** — Linux sysfs interface |
| Range analyzer | "Missing" | **Implemented** — 372 LOC |
| Type advisor | "Missing" | **Implemented** — 344 LOC |
| Algorithm profiler | "Missing" | **Implemented** — 499 LOC |
| Pareto explorer | "Missing" | **Implemented** — 794 LOC |
| Autotuner | "Missing" | **Implemented** — 525 LOC |
| Memory profiler | "Missing" | **Implemented** — 433 LOC |
| Instrumented types | "Missing" | **Implemented** — 641 LOC |
| Mixed-precision BLAS | Not mentioned | **Implemented** — mp_dot, mp_gemm |
| Block formats | Not mentioned | **Implemented** — mxblock, nvblock, zfpblock, zfparray |
| Quantization metrics | qsnr only | **Implemented** — RMSE, SNR, QSNR (error_metrics.hpp) |
| Block format benchmarks | Not mentioned | **Implemented** — quantization_error.cpp, throughput.cpp |
| Methodology examples | Not mentioned | **Implemented** — 19 programs, 3,636 LOC |

**Bottom line**: The blocking items from the original roadmap are resolved. The paper
can now be written with real experimental data.

### What the Block Formats Add to the Paper

The three block format implementations are the **strongest new contribution** since the
JOSS paper. No other library provides unified, header-only C++ implementations of:

1. **OCP MX v1.0** (mxblock) — Industry standard for AI inference
2. **NVIDIA NVFP4** (nvblock) — Production format in H100/B200 GPUs
3. **ZFP** (zfpblock/zfparray) — Transform-based lossy compression

Under a single type-parameterized API. This is publishable on its own.

---

## Part 2: Weaknesses in the Current Draft

### Problem 1: The Paper Has No Data

Section 5 ("Energy Efficiency Analysis") says "[SECTION IN PROGRESS - Requires
benchmark data]". The draft is ~60% prose argument with 0% empirical validation.
ArXiv referees (and readers) will dismiss a position paper without quantitative
support.

**Fix**: We now HAVE the data. The block format benchmarks produce real RMSE/SNR/QSNR
numbers. The energy models produce real pJ estimates. We need to collect and present them.

### Problem 2: The Thesis Is Too Broad

"Mixed-precision is the natural paradigm for any system that interfaces with the
physical world" is unfalsifiable. It reads as advocacy, not science.

**Fix**: Narrow to a falsifiable, demonstrable claim:

> "A unified type-parameterized library enables systematic precision optimization
> across the full spectrum from 4-bit block formats to quad-double, and we demonstrate
> that the choice of quantization strategy (power-of-two scaling, fractional scaling,
> transform coding) depends measurably on data statistics — with nvfp4's fractional
> scale achieving 3x lower RMSE than mxfp4 on smooth signals, while zfp's decorrelating
> transform achieves 30+ dB better SNR at comparable rates."

### Problem 3: No Related Work Section

The draft cites Bailey (2005) and Horowitz (2014) and nothing else. A publication
needs to position against:

- **MPFR/GMP** — arbitrary precision (different goal, complementary)
- **FloatX** — multi-format floats (limited scope, no block formats)
- **QPyTorch** — quantization-aware training (Python, no hardware targeting)
- **Microsoft microxcaling** — MX reference impl (Python, not a library)
- **LLNL ZFP** — compression library (C, not a number system)
- **Posit standard (2022)** — Gustafson's work
- **Carson & Higham (2018)** — iterative refinement in three precisions
- **arXiv:2310.10537** — Microscaling data formats for deep learning

### Problem 4: Missing Reproducibility Artifacts

ArXiv papers with code need:
- A single script to regenerate all tables and figures
- Version-pinned build instructions
- Machine-readable output (CSV/JSON), not just console output

### Problem 5: The Paper Tries to Be Two Things

It oscillates between:
1. **Position paper** (philosophical argument about precision)
2. **Systems paper** (library description with benchmarks)

These require different structures. A position paper for IEEE CSE can be discursive.
An ArXiv systems paper needs architecture, implementation, evaluation, comparison.

**Recommendation**: Write a **systems paper** for ArXiv. The library + block formats +
benchmarks + SDK provide enough substance. Save the position paper for a venue like
IEEE Computing in Science & Engineering later.

---

## Part 3: Proposed Paper Structure

### Restructured for ArXiv (Systems Paper)

```
Title: Universal: A C++ Template Library for Mixed-Precision
       Algorithm Design with Block Floating-Point Formats

1. Introduction (1.5 pages)
   - Precision as a design parameter (Bailey inversion)
   - The block format explosion (MX, NVFP4, ZFP, MSFP)
   - Contribution: unified library + systematic comparison

2. Background & Related Work (1.5 pages)
   - Number systems taxonomy (IEEE 754, posit, fixed, LNS, block)
   - Block floating-point formats (MX spec, NVFP4 spec, ZFP algorithm)
   - Related libraries (MPFR, FloatX, ZFP, microxcaling)
   - Mixed-precision in DL (AMP, quantization-aware training)

3. Architecture (2 pages)
   - Plug-in replacement pattern (template parameterization)
   - Number system categories (static vs elastic)
   - Block format design: mxblock, nvblock, zfpblock
   - Mixed-precision SDK components

4. Block Format Implementations (2.5 pages)
   - mxblock: OCP MX v1.0 (e8m0 scale, quantize/dequantize)
   - nvblock: NVIDIA NVFP4 (e4m3 scale + tensor scale)
   - zfpblock: ZFP codec (lifting transform, embedded coding)
   - zfparray: Multi-block compressed array with random access
   - Unified API comparison table

5. Mixed-Precision Design Methodology (1.5 pages)
   - Energy cost models (10 architectures)
   - Instrumented types for operation profiling
   - Type advisor and Pareto explorer
   - Workflow: profile → analyze → explore → generate

6. Experimental Evaluation (3 pages)
   6.1 Quantization accuracy (RMSE, SNR, QSNR)
       - Sinusoidal and ramp test signals
       - 6 format configurations
       - Analysis of scaling strategy impact
   6.2 Throughput
       - Quantize+dequantize cycles
       - Block size and transform overhead
   6.3 Energy estimation
       - Operation-count-based energy comparison
       - Memory bandwidth reduction
   6.4 Mixed-precision case study
       - CG solver or robotics pipeline
       - Precision scheduling across pipeline stages
   6.5 Comparison with reference implementations
       - mxblock vs Microsoft microxcaling
       - zfpblock vs LLNL ZFP library

7. Discussion (1 page)
   - When to use which format
   - Limitations (software emulation, no hardware targeting yet)
   - The precision selection problem

8. Conclusion (0.5 pages)

References (~30 entries)

Appendix A: Complete benchmark results
Appendix B: Number system inventory (37 types)
```

**Estimated length**: ~14 pages (ArXiv has no page limit)

---

## Part 4: Implementation Roadmap

### Phase A: Experimental Infrastructure (Week 1)

Create reproducible benchmark scripts and machine-readable output.

| Task | Owner | Deliverable | Blocked By |
|------|-------|-------------|------------|
| A1. Create `paper/benchmarks/` directory | — | Directory structure | — |
| A2. Refactor quantization_error.cpp for CSV output | — | CSV + console output | — |
| A3. Refactor throughput.cpp for CSV output | — | CSV + console output | — |
| A4. Create energy estimation benchmark | — | `energy_comparison.cpp` | — |
| A5. Create mixed-precision CG case study | — | `cg_mixed_precision.cpp` | — |
| A6. Create `run_all_benchmarks.sh` | — | Regenerates all data | A2-A5 |

**A2 detail**: Add `--csv` flag to quantization_error.cpp that outputs:
```
format,rate_bpv,ratio,rmse,snr_db,qsnr_db,signal
mxfp4,4.25,7.5,1.1292e-01,15.93,15.93,sinusoidal
```

**A4 detail**: New benchmark that runs the same operations across multiple number
types and reports estimated energy using the SDK energy models:
```cpp
#include <universal/energy/energy.hpp>
#include <universal/utility/instrumented.hpp>
// Profile dot product with instrumented<cfloat<16,5>>, instrumented<posit<16,1>>,
// instrumented<float>, etc. Report ops × pJ/op for each architecture.
```

**A5 detail**: Adapt existing `mixedprecision/tensor/cg/cg_fdp.cpp` into a
paper-quality case study showing:
- FP64 baseline accuracy
- FP32 + FP64 accumulator
- FP16 + FP32 accumulator
- posit<16,1> + posit<32,2> accumulator
- Convergence curves + energy estimates

### Phase B: Comparison Benchmarks (Week 2)

Validate our implementations against reference libraries.

| Task | Owner | Deliverable | Blocked By |
|------|-------|-------------|------------|
| B1. Compare mxblock vs microxcaling Python | — | Bit-exact validation script | — |
| B2. Compare zfpblock vs LLNL ZFP C library | — | Roundtrip error comparison | — |
| B3. Expand test signals beyond sin/ramp | — | Gaussian noise, speech-like, natural image patch | A2 |
| B4. Add posit/cfloat scalar quantization to benchmark | — | Same RMSE/SNR table for scalar types | A2 |

**B1 detail**: Install `microxcaling` Python package, quantize identical test vectors,
compare output byte-for-byte. Report any deviations with analysis.

**B3 detail**: Add test signal generators:
- Gaussian white noise (tests uniform spectral content)
- Speech-like (sparse in frequency, tests real-world sparsity)
- Natural image patch (2D, spatial correlation)
- Constant DC signal (tests zero-variance edge case)

**B4 detail**: Add scalar quantization rows to the benchmark table so readers can
compare block formats against simple round-to-nearest:
```
posit<8,0>          8.00    4.0x    ...    ...    ...
cfloat<8,4>         8.00    4.0x    ...    ...    ...
fixpnt<8,4>         8.00    4.0x    ...    ...    ...
half (cfloat<16,5>) 16.00   2.0x    ...    ...    ...
```

### Phase C: Paper Writing (Weeks 3-4)

| Task | Owner | Deliverable | Blocked By |
|------|-------|-------------|------------|
| C1. Write Section 1 (Introduction) | Lead author | LaTeX draft | — |
| C2. Write Section 2 (Background & Related Work) | Lead author | LaTeX draft | — |
| C3. Write Section 3 (Architecture) | Lead author | LaTeX draft | — |
| C4. Write Section 4 (Block Formats) | Lead author | LaTeX draft | — |
| C5. Write Section 5 (Methodology) | Lead author | LaTeX draft | — |
| C6. Write Section 6 (Evaluation) | Lead author | LaTeX + generated tables | A6, B3 |
| C7. Write Section 7-8 (Discussion, Conclusion) | Lead author | LaTeX draft | C6 |
| C8. Create figures | — | Pipeline diagrams, Pareto plots, bar charts | A6 |
| C9. Compile references.bib | — | ~30 entries | — |

**C4 detail** (Section 4 is the strongest contribution): For each block format:
- Mathematical formulation of quantize/dequantize
- Storage layout diagram (scale byte + element bytes)
- Compression ratio derivation
- Code snippet showing the API

**C6 detail**: All tables generated from CSV output of Phase A benchmarks.
Script `paper/generate_tables.py` reads CSV → produces LaTeX tabular environments.

**C8 detail**: Figures needed:
1. Pipeline diagram (sensor → processing → actuator with precision annotations)
2. Block format storage layouts (mxblock vs nvblock vs zfpblock side-by-side)
3. RMSE bar chart (6 formats × 2 signals)
4. SNR vs bits-per-value scatter plot
5. Energy breakdown pie charts (compute vs memory)
6. Pareto frontier (accuracy vs energy)
7. CG convergence curves (multiple precisions)

### Phase D: Polish & Submit (Week 5)

| Task | Owner | Deliverable | Blocked By |
|------|-------|-------------|------------|
| D1. Internal review | Co-authors | Annotated draft | C7 |
| D2. Revise based on review | Lead author | Final draft | D1 |
| D3. Prepare arXiv submission package | — | .tar.gz with LaTeX + figures | D2 |
| D4. Create GitHub release tag | — | `v3.96-paper` tag | D2 |
| D5. Update README with paper citation | — | README.md edit | D3 |
| D6. Submit to arXiv | Lead author | arXiv ID | D3 |

---

## Part 5: Governance & Accountability

### Roles

| Role | Responsibility |
|------|----------------|
| **Lead Author** | Paper writing, thesis refinement, submission |
| **Implementation Lead** | Benchmark infrastructure, CSV output, new benchmarks |
| **Review Lead** | Internal review, reference checking, reproducibility verification |

### Weekly Checkpoints

| Week | Milestone | Gate Criteria |
|------|-----------|---------------|
| 1 | Phase A complete | All benchmarks produce CSV output; `run_all_benchmarks.sh` succeeds |
| 2 | Phase B complete | Reference comparison validates our implementations; expanded signals tested |
| 3 | Sections 1-5 drafted | Architecture and block format sections complete with figures |
| 4 | Full draft | All sections written; evaluation tables generated from data |
| 5 | Submission | Internal review addressed; arXiv package uploaded |

### Quality Gates

Each phase has explicit pass/fail criteria:

**Phase A gate**:
- [ ] `run_all_benchmarks.sh` produces all CSV files without error
- [ ] CSV files contain data for all 6+ format configurations
- [ ] Energy benchmark produces per-architecture pJ estimates
- [ ] CG case study shows convergence for all precision configurations

**Phase B gate**:
- [ ] mxblock matches microxcaling reference to within 1 ULP
- [ ] zfpblock matches LLNL ZFP to documented precision
- [ ] At least 4 test signal types produce meaningful results
- [ ] Scalar quantization results included in comparison tables

**Phase C gate**:
- [ ] All sections drafted in LaTeX
- [ ] All tables generated from CSV (no hand-typed numbers)
- [ ] All 7 figures created
- [ ] References complete (no "[citation needed]" markers)

**Phase D gate**:
- [ ] Internal review completed with all issues addressed
- [ ] `make` in paper directory produces PDF without errors
- [ ] Benchmark code compiles with `cmake -DUNIVERSAL_BUILD_BENCHMARK_ACCURACY=ON`
- [ ] arXiv submission compiles on their TeX Live

### Risk Register

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Energy model numbers challenged by reviewers | Medium | Medium | Cite Horowitz (2014) and architecture manuals; clearly label as estimates |
| microxcaling comparison shows discrepancies | Low | High | Document any deviations with analysis of spec ambiguity |
| ZFP comparison shows our codec is slower | Medium | Low | We're header-only C++ vs optimized C; acknowledge tradeoff |
| Scope creep into position paper territory | High | Medium | Strict section structure; save position arguments for separate paper |
| Results don't show clear winner among formats | Low | Low | That IS the result — format choice depends on data statistics |

---

## Part 6: What Makes This Paper Publishable

### Novel Contributions (Referee Checklist)

1. **First unified C++ implementation of OCP MX, NVFP4, and ZFP under one API**
   - No existing library provides all three
   - Header-only, no dependencies, plug-in replacement

2. **First systematic quantitative comparison of block format strategies**
   - Power-of-two scaling vs fractional scaling vs transform coding
   - On identical data with identical metrics (RMSE, SNR, QSNR)

3. **Complete mixed-precision SDK with energy modeling**
   - 10 architecture-specific energy models
   - Type advisor with Pareto analysis
   - Instrumented types for operation profiling

4. **37 number systems in one library**
   - Largest collection of C++ arithmetic types
   - From 4-bit microfloats to quad-double

### Differentiators vs Related Work

| Feature | Universal | MPFR | FloatX | ZFP lib | microxcaling |
|---------|-----------|------|--------|---------|-------------|
| Language | C++20 | C | C++ | C | Python |
| Header-only | Yes | No | Yes | No | N/A |
| Number systems | 37 | 1 | ~10 | 0 | 4 |
| Block formats | 3 | 0 | 0 | 1 | 4 |
| Energy modeling | Yes | No | No | No | No |
| Type advisor | Yes | No | No | No | No |
| Hardware targetable | Yes | No | Partial | No | No |

---

## Part 7: File Inventory for Paper

### Files to Create

```
paper/
  arxiv/
    main.tex                    — Paper source
    references.bib              — Bibliography
    figures/
      pipeline.pdf              — Sensor-to-actuator pipeline
      block_layouts.pdf         — mxblock/nvblock/zfpblock storage
      rmse_comparison.pdf       — Bar chart
      snr_vs_bpv.pdf            — Scatter plot
      energy_breakdown.pdf      — Pie charts
      pareto_frontier.pdf       — Accuracy vs energy
      cg_convergence.pdf        — Convergence curves
    tables/
      (generated from CSV by script)

  benchmarks/
    run_all_benchmarks.sh       — Master script
    generate_tables.py          — CSV → LaTeX converter
    results/                    — CSV output directory

  validation/
    compare_microxcaling.py     — mxblock vs reference
    compare_zfp.py              — zfpblock vs LLNL ZFP
```

### Files to Modify

```
benchmark/accuracy/blockformat/quantization_error.cpp  — Add CSV output mode
benchmark/accuracy/blockformat/throughput.cpp           — Add CSV output mode
```

### Files to Create (Benchmarks)

```
benchmark/accuracy/blockformat/energy_comparison.cpp    — Energy estimation benchmark
benchmark/accuracy/blockformat/scalar_comparison.cpp    — Scalar type quantization
benchmark/accuracy/blockformat/signal_generators.hpp    — Extended test signals
```

---

## Summary

The paper is closer to completion than the stale STATUS.md suggests. The critical
path is:

1. **Week 1**: Wire existing benchmarks for reproducible output (CSV + scripts)
2. **Week 2**: Validate against reference implementations; expand test signals
3. **Week 3-4**: Write the paper (most prose exists in draft form)
4. **Week 5**: Review, polish, submit

The block formats are the paper's strongest differentiator. They should be the
centerpiece of Section 4 and the primary subject of the evaluation. The energy
modeling and SDK components provide supporting context but are not the main
contribution — the unified block format comparison is.
