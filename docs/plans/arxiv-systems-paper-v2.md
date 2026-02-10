# Plan: Rewrite Systems Paper Around Feasibility Thesis

## Problem with v1

The first draft was a catalog: pages of block format descriptions, weak enumerations of application domains, no thesis, no insight. It read like reference documentation, not a research paper.

## The Core Thesis

**Precision is the most underutilized design dimension in computing. In a growing class of critical applications, the difference between a feasible and infeasible system is not faster hardware or better algorithms, but correct precision assignment.**

This is *not* "mixed-precision is more efficient." It is: **uniform precision makes certain applications physically impossible, and matched precision makes them possible.** Every section must serve this argument.

## Redesigned Structure

```
Title: "Precision as a Design Dimension: When Matched Arithmetic
        Makes Infeasible Systems Feasible"

   (or keep current title, but the content must embody this thesis)

Abstract (~200 words)
   Thesis sentence + 5 go/no-go examples in one paragraph +
   Universal library as the methodology + solver validation

1. Introduction (~2 pages)
   NOT: "mixed precision saves energy"
   INSTEAD: "uniform precision creates four classes of infeasibility"

   1a. The precision inversion (Bailey): sensors deliver 6-20 bits,
       we compute at 32-64 — this isn't waste, it's a design error
       that creates hard infeasibilities

   1b. Four infeasibility classes (the paper's organizing framework):
       - MEMORY: LLaMA-70B at FP32 = 280 GB → won't fit on any single GPU
       - POWER: Brain implant at FP32 → exceeds 45 mW tissue damage threshold
       - TIME: Mars rover vision at FP64 → 180s/frame, landing needs <2s
       - CONVERGENCE: CG solver in half → diverges; same algorithm in
         posit<16,1> → converges

   1c. Table 1: Summary of 6 go/no-go transitions (the paper's roadmap)

   1d. Contributions:
       (1) Feasibility framework: four classes of precision-induced
           infeasibility with quantitative thresholds
       (2) Universal library: 37 number systems enabling systematic
           precision design-space exploration
       (3) Six case studies demonstrating go/no-go transitions across
           domains, validated with the library's plug-in architecture

2. The Cost of Precision (~1.5 pages)
   NOT: background/related work laundry list
   INSTEAD: the physical and mathematical foundations for WHY precision
   determines feasibility

   2.1 Arithmetic Energy Scaling
       - Horowitz 2014 data: 8-bit mul = 0.2 pJ, 64-bit FP mul = 15 pJ
       - Memory dominance: DRAM read = 1300 pJ >> any arithmetic op
       - Table 2: Energy per operation × technology node
         (45nm Horowitz + 14nm Skylake from Universal's energy module)
       - Key insight: narrower types reduce BOTH compute AND data movement

   2.2 Information-Theoretic Floor
       - Sensor ENOB table: no physical sensor delivers >20 effective bits
       - Implication: precision beyond sensor ENOB is thermodynamically
         wasted — you're spending energy to refine noise
       - This reframes precision reduction from "lossy approximation" to
         "matching computation to physical reality"

   2.3 The Convergence Boundary
       - Carson & Higham theorem: 3-precision IR converges iff
         κ(A) · u_f < threshold (u_f = factorization roundoff)
       - Implication: there exists a HARD precision boundary below which
         solvers diverge — this is a mathematical go/no-go, not a
         performance tradeoff
       - Cite: Carson & Higham 2018, Higham & Pranesh 2019

3. Go/No-Go Case Studies (~5 pages, the heart of the paper)
   Each case study follows the same pattern:
   (a) The engineering constraint (watts, bytes, seconds, convergence)
   (b) Why uniform precision violates it (quantitative)
   (c) How matched precision satisfies it (quantitative)
   (d) What Universal enables (plug-in exploration, energy modeling)

   3.1 LLM Deployment: The Memory Wall (~1 page)
       Constraint: Single-GPU memory (80 GB on A100)
       - LLaMA-70B at FP32: 280 GB → requires 4 GPUs minimum ($120K+)
       - LLaMA-70B at FP16: 140 GB → requires 2 GPUs
       - LLaMA-70B at INT4: 35 GB → fits on 1 GPU ($30K)
       - This is NOT optimization. At FP32, deployment on edge is IMPOSSIBLE.
       - Universal connection: microfloat types (e2m1, e4m3) + mxblock/nvblock
         enable exploring quantization strategies; energy model quantifies
         the compute savings
       - Cite: Dettmers 2022, NVIDIA NVFP4 2024, OCP MX 2023

   3.2 Brain-Computer Interfaces: The Thermal Ceiling (~1 page)
       Constraint: Intracranial implant must not heat tissue by >1°C →
       max ~45 mW at implant site
       - 68-channel neural recording SoC: 0.41 µW/channel in fixed-point
       - FP32 MAC uses ~23.8% more power → pushes multi-hundred-channel
         implants past thermal safety boundary
       - Neuralink data rate: 200 Mbps raw → 1-2 Mbps Bluetooth →
         200× compression required ON-CHIP with sub-mW budget
       - This is life-safety go/no-go: wrong precision = tissue damage
       - Universal connection: fixpnt<12,8> and integer<8> types model
         exactly the arithmetic these SoCs implement; energy model predicts
         the thermal envelope

   3.3 Spacecraft Navigation: The Compute Desert (~0.75 page)
       Constraint: RAD750 = 200 MHz, 266 MIPS (radiation-hardened,
       Pentium-1 class)
       - Perseverance stereo vision: 180 seconds/frame on RAD750 (FP)
       - Same algorithm on Virtex-5 FPGA in fixed-point: 1.5 seconds/frame
       - Landing sequence time budget: seconds, not minutes
       - Without reduced-precision FPGA processing, autonomous hazard
         avoidance during Mars landing was physically impossible
       - 120× speedup is not a performance improvement — it crosses the
         feasibility boundary from "crash" to "land safely"

   3.4 5G Baseband: The Throughput Floor (~0.75 page)
       Constraint: 5G NR LDPC decoding at 20 Gbps, real-time
       - FP32 belief propagation: cannot meet throughput in available silicon
       - 4-bit/6-bit fixed-point GA-MS decoder: 24.4 Gbps on 28nm ASIC,
         1.823 mm² die area, 0.05 dB BER penalty
       - FP32 decoder at same area → orders of magnitude less throughput
       - Every 5G phone on Earth uses this go/no-go transition

   3.5 Climate Data: The Storage Crisis (~0.75 page)
       Constraint: CMIP6 = 20-30 PB; CMIP7 projected 100+ PB;
       km-scale models produce 4.5 TB/simulated-day
       - DYAMOND project: researchers CANNOT SAVE the data their models
         produce. 3D variable output "severely limited" by storage.
       - ZFP at 5:1 compression: 20 PB → 4 PB, within storage budgets
       - Without lossy compression, km-scale climate science is
         infeasible — not compute-limited but STORAGE-limited
       - Universal's zfpblock/zfparray provide C++ type-level interface
         with transparent random access to compressed data

   3.6 Iterative Solvers: The Convergence Cliff (~1 page)
       Constraint: Must converge to required accuracy
       - CG on tridiag(-1,2,-1), n=32, κ≈414:
         · half → DIVERGES (DNF in 500 iterations)
         · bfloat16 → DIVERGES
         · posit<16,1> → CONVERGES (47 iterations, 8.3e-4)
         · float → CONVERGES (20 iterations, 3.7e-7)
       - Three-precision IR:
         · all-half → DIVERGES
         · half/float/double → CONVERGES in 3 iterations
       - This is a mathematical go/no-go: below a precision threshold,
         the algorithm produces NO answer, not a worse answer
       - Universal enables discovery of these thresholds by re-running
         identical code with different type arguments
       - Code listing: same algorithm, different using declarations
       - Table: convergence results across 8+ type configurations

4. The Universal Library (~2 pages)
   NOT: 37-type catalog
   INSTEAD: how the library's design enables the feasibility analysis
   demonstrated in Section 3

   4.1 Plug-in Architecture as Methodology (~0.75 page)
       - Template parameterization: change `using Real = ...` to explore
         precision space
       - Code listing: CG solver instantiated with 4 different types
         (the SAME code that produced Section 3.6 results)
       - Key: the library doesn't just provide types — it provides a
         methodology for systematic precision design-space exploration

   4.2 Energy-Aware Design (~0.5 page)
       - Built-in energy models for 6 architectures (45nm→3nm)
       - EnergyEstimator class: profile an algorithm's operation mix,
         predict energy at different precisions BEFORE committing to hardware
       - This closes the loop: explore precision in software → predict
         energy → verify feasibility → deploy

   4.3 From Scalar to Block: Unified Abstraction (~0.75 page)
       - Block formats (mxblock, nvblock, zfpblock) share
         quantize/dequantize/dot API
       - Brief comparison table (NOT 2.5 pages of format math)
       - Key insight: format choice depends on data statistics —
         power-of-two scale (MX) vs fractional (NVFP4) vs transform (ZFP)
       - One quantization accuracy comparison table
         (RMSE/SNR at ~4 bits/value)
       - Cite the specs; don't re-derive them

5. Discussion (~1 page)
   5.1 The Precision Selection Problem
       - No single format dominates — format choice is data-dependent
       - The library reframes this from guesswork to measurement

   5.2 Limitations
       - Software emulation: design-time exploration, not deployment perf
       - Energy models are estimates, not measurements
       - Solver case studies use small matrices (n=32) —
         precision sensitivity, not scalability study

   5.3 Related Work
       - MPFR, FloatX, ZFP, microxcaling — brief positioning table
       - Carson & Higham, Haidar et al., Micikevicius et al.
       - Compact: these are background, not the contribution

6. Conclusion (~0.5 page)
   Restate thesis: precision is a design dimension with hard feasibility
   boundaries. Summarize 6 go/no-go transitions. Future: hardware
   targeting, auto-tuning, ML framework integration.

Appendix A: Number System Inventory (table, from v1)
Appendix B: Block Format Details (moved from v1 Section 4)
```

## What Changes from v1

| Aspect | v1 (Catalog) | v2 (Feasibility) |
|--------|-------------|------------------|
| Thesis | "37 types + 3 block formats" | "Precision determines feasibility" |
| Structure | Type descriptions → examples → solvers | Foundations → go/no-go cases → methodology |
| Block formats | 2.5 pages of math/algorithms | 0.75 page comparison table; details in appendix |
| Application examples | 8 shallow (0.3 pages each) | 6 deep (0.75-1 page each) with worked numbers |
| Solver case studies | Separate section, disconnected | Integrated as Case Study 3.6, connected to thesis |
| Library description | 2 pages of API surface | 2 pages of methodology (why it enables the analysis) |
| Energy data | Summary table at end | Foundational in Section 2, used throughout |
| Related work | Background section | Compact positioning in Discussion |

## Key Principles for v2

1. **Every paragraph must serve the thesis.** If it doesn't argue that precision determines feasibility, cut it.
2. **Go/no-go, not percentage improvements.** "3x faster" is boring. "Impossible → possible" is a paper.
3. **Show the numbers.** Each case study has a quantitative feasibility boundary: GB, mW, ms, κ(A).
4. **The library is methodology, not product.** It enables systematic precision design-space exploration.
5. **Block formats are tools, not contributions.** They appear when they enable a case study, not as standalone sections.

## File to Modify

**`papers/systems-paper/paper/main.tex`** — Complete rewrite following new structure.

## New References Needed

The bibliography needs ~10-15 additional entries for the new case studies:
- Neuralink / neural SoC papers (thermal safety)
- RAD750 / Perseverance FPGA papers (spacecraft computing)
- 5G LDPC decoder papers (throughput constraint)
- CMIP6/DYAMOND papers (climate data volume)
- LLaMA quantization papers (memory constraint)
- HPL-MxP benchmark (system-scale validation of Carson-Higham)
- Higham & Mary 2022 Acta Numerica survey

## Verification

1. LaTeX compiles cleanly with pdflatex + bibtex
2. Zero `% TODO` lines
3. All bib keys cited
4. Each of 6 case studies has quantitative go/no-go boundary
5. Block format details occupy ≤1 page in body (rest in appendix)
6. Paper reads as a coherent argument, not a catalog
