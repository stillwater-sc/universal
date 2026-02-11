# Mixed-precision algorithm design paper status

## Key Findings

The good news: you're much further along than the STATUS.md suggests

The original roadmap (Feb 2025) listed energy tools at 5%, type advisor as missing, Pareto explorer as missing, etc. All of those are now implemented —
over 4,000 lines of SDK utility code, 2,400 lines of energy models across 10 architectures, RAPL integration, 19 methodology examples. The blocking items
from the original roadmap are resolved.

The block formats change everything

The mxblock/nvblock/zfpblock work is the strongest publishable contribution. No other library provides unified, header-only C++ implementations of OCP MX
v1.0, NVIDIA NVFP4, and ZFP under a single type-parameterized API. The benchmark data you already have (nvfp4 achieving 3x lower RMSE than mxfp4, zfp
scaling to 108 dB SNR) is exactly the kind of quantitative result referees want.

Five problems with the current draft

1. No data — Section 5 says "[Requires benchmark data]", but the data now exists
2. Thesis too broad — "mixed-precision for all physical-world computing" is unfalsifiable. Narrow it to the block format comparison + SDK demonstration
3. No related work — Needs positioning against MPFR, FloatX, ZFP, microxcaling
4. No reproducibility — Need CSV output and regeneration scripts
5. Identity crisis — Part position paper, part systems paper. For ArXiv, write a systems paper with experimental evaluation. Save the position paper for
IEEE CSE

Recommendation: Pivot to systems paper

The restructured paper has 8 sections focused on architecture, block format implementations, mixed-precision methodology, and experimental evaluation
(quantization accuracy + throughput + energy estimation + CG case study). Estimated ~14 pages.

5-week roadmap with gates
┌──────┬─────────────────────────────────────────────────────────────────────────────────┬───────────────────────────────────────────────┐
│ Week │                                      Focus                                      │                     Gate                      │
├──────┼─────────────────────────────────────────────────────────────────────────────────┼───────────────────────────────────────────────┤
│ 1    │ Benchmark infrastructure (CSV output, energy benchmark, CG case study)          │ run_all_benchmarks.sh succeeds                │
├──────┼─────────────────────────────────────────────────────────────────────────────────┼───────────────────────────────────────────────┤
│ 2    │ Reference validation (vs microxcaling, vs LLNL ZFP), expanded test signals      │ Implementations match references              │
├──────┼─────────────────────────────────────────────────────────────────────────────────┼───────────────────────────────────────────────┤
│ 3    │ Write Sections 1-5 (intro through methodology)                                  │ Architecture + block format sections complete │
├──────┼─────────────────────────────────────────────────────────────────────────────────┼───────────────────────────────────────────────┤
│ 4    │ Write Sections 6-8 (evaluation through conclusion), generate all tables/figures │ Full draft, all tables from CSV               │
├──────┼─────────────────────────────────────────────────────────────────────────────────┼───────────────────────────────────────────────┤
│ 5    │ Internal review, polish, submit                                                 │ arXiv package uploaded                        │
└──────┴─────────────────────────────────────────────────────────────────────────────────┴───────────────────────────────────────────────┘
The full roadmap with tasks, governance roles, quality gates, risk register, and file inventory is in docs/papers/arxiv-roadmap.md. Want me to walk through
any section in detail, or shall we start executing Phase A?

Files Modified/Created
┌─────────────────────────────────────┬────────────────────────────────────────────────────────────────────────┐
│                File                 │                                 Action                                 │
├─────────────────────────────────────┼────────────────────────────────────────────────────────────────────────┤
│ docs/papers/arxiv-roadmap.md        │ Renamed → arxiv-systems-paper.md                                       │
├─────────────────────────────────────┼────────────────────────────────────────────────────────────────────────┤
│ docs/papers/arxiv-position-paper.md │ Created — full roadmap for the IEEE CSE position paper                 │
├─────────────────────────────────────┼────────────────────────────────────────────────────────────────────────┤
│ docs/papers/STATUS.md               │ Updated — reflects two-paper strategy and current infrastructure state │
└─────────────────────────────────────┴────────────────────────────────────────────────────────────────────────┘
Position Paper Roadmap Highlights

Thesis: "For embedded intelligence, the numeric precision used at each computational stage is already as important as the algorithm itself — and the energy
cost of ignoring this is measured in watts, not just ULPs."

Key structural decisions:
- The foil: The IEEE 754 monoculture — compilers default to double, textbooks teach double, libraries require double. The paper argues against inherited
defaults, not against IEEE 754 itself.
- "Why now?": The block format explosion (OCP MX 2023, NVFP4 2024) — industry is voting with silicon. This is the timing argument the draft was missing.
- Library discipline: Universal appears only in Sections 5-6, never before. The argument must stand without the tool. If a reviewer says "this is a product
pitch," the paper has failed.
- One concrete case study over three hypothetical ones: The robotics pipeline (already implemented, 19 KB) with instrumented energy breakdown.
- The Bailey mirror as closing: Bailey showed you sometimes need more precision. We show you usually need less. Together they prove the center is the wrong
default.

6-week schedule: Sharpen argument → Gather evidence → Write first half → Full draft → Review → Submit.

Submission order: Systems paper to arXiv first (citable immediately), position paper to IEEE CSE second (cites arXiv for depth).

