# Position Paper: Implementation Roadmap

**Working Title**: "From High-Precision to Right-Precision: Mixed-Precision
Algorithm Design for the Embodied Intelligence Era"

**Target Venues** (in order):
1. IEEE Computing in Science & Engineering (Bailey's venue — natural home)
2. Communications of the ACM (broader reach, practitioner audience)
3. IEEE Micro (hardware/software co-design angle)

**Companion**: `arxiv-systems-paper.md` covers the systems/implementation paper
for arXiv. This document covers the broader argument paper.

---

## Part 1: What a Position Paper Is (and Isn't)

A position paper is **not** a systems paper. It does not need to describe an
implementation in detail or present exhaustive benchmarks. It needs to:

1. **Identify a paradigm shift** that the community has not fully recognized
2. **Make a clear, memorable thesis** that changes how readers think
3. **Provide enough evidence** to be credible, but not so much that the
   argument drowns in data
4. **Issue a call to action** that shapes future research and practice

Bailey's 2005 paper succeeded because it told readers: "You think double precision
is enough. It isn't. Here's why, and here's what to do about it." Our paper tells
readers the complementary story: "You think double precision is necessary. It isn't.
Here's why, and here's what to do about it."

### How This Differs from the Systems Paper

| Dimension | Systems Paper (ArXiv) | Position Paper (IEEE CSE) |
|-----------|----------------------|--------------------------|
| **Core claim** | Universal provides the first unified block format API | Mixed-precision is the natural paradigm for physical-world computing |
| **Evidence** | Detailed benchmarks, comparisons, evaluation | Selected examples that support the argument |
| **Library role** | The contribution | Supporting evidence |
| **Tone** | Technical, precise | Persuasive, forward-looking |
| **Length** | ~14 pages | ~8 pages (IEEE CSE limit: 6-8) |
| **Figures** | Data plots, architecture diagrams | Conceptual diagrams, pipeline illustrations |
| **Audience** | Library users, systems researchers | Broad scientific computing community |
| **Novelty test** | "Is this a useful tool?" | "Does this change how I think?" |

---

## Part 2: Assessment of the Current Draft

The existing `position-paper-draft.md` (v0.2) is a reasonable skeleton with
important structural problems.

### Strengths

1. **The Bailey mirror** — Inverting Bailey's high-precision argument is a
   powerful rhetorical device. Keep it.

2. **Sensor resolution table** — The concrete data showing no sensor exceeds
   ~20 effective bits is the paper's most persuasive evidence. It grounds an
   abstract argument in physical reality.

3. **Pipeline precision diagram** — The 5-stage pipeline (sensor → conditioning
   → features → decision → actuator) with precision annotations at each stage
   is immediately understandable. This should be Figure 1.

4. **The thesis statement** — "For embodied and embedded intelligence, the optimal
   precision for each computational kernel is as important to system efficiency
   as the choice of algorithm itself" is strong and quotable.

### Weaknesses

1. **No opponent** — The paper argues against "one-size-fits-all precision" but
   never identifies who advocates for it or why it persists. The argument needs
   a foil: the IEEE 754 monoculture and the toolchain/education ecosystem that
   perpetuates it (compilers default to double, textbooks use double, libraries
   require double, hardware optimizes for double).

2. **Energy section is empty** — The paper's energy argument is its strongest
   practical claim but has zero data. The Horowitz (2014) numbers are cited
   but not connected to worked examples.

3. **Block formats not mentioned** — The draft predates the mxblock/nvblock/zfpblock
   work. The block format explosion (OCP MX in 2023, NVFP4 in 2024, MSFP, etc.)
   is the strongest real-world validation of the thesis — industry is already
   moving to mixed-precision block formats. The paper must reference this.

4. **Case studies are hypothetical** — Section 5 lists autonomous vehicles,
   robotics, edge AI with no concrete data. A position paper can be lighter
   on data than a systems paper, but "3-10x efficiency gains" needs at least
   one worked example to be credible.

5. **No "what's new" moment** — Why publish this argument now? The answer is:
   the block format explosion (2023-2025) has created an industry-wide migration
   toward sub-8-bit formats, but without a principled framework for choosing
   among them. The Universal library provides that framework. This timing
   argument is missing from the draft.

6. **Conclusion is generic** — "Treat precision as a first-class design parameter"
   is correct but unmemorable. Needs a sharper formulation.

---

## Part 3: Proposed Paper Structure

### Title

"From High-Precision to Right-Precision: Mixed-Precision Algorithm Design
for the Embodied Intelligence Era"

Alternatives:
- "Right-Sizing Computation: Why Mixed-Precision Is the Natural Paradigm
  for Physical-World Computing"
- "The Precision Mismatch: How 64-bit Arithmetic Wastes 90% of Embedded
  System Energy"

### Structure (~8 pages for IEEE CSE)

```
1. Introduction: The Precision Pendulum (1 page)
   - Bailey's 2005 thesis: "precision matters — sometimes you need more"
   - Our complementary thesis: "precision matters — usually you need less"
   - The deep learning precedent: INT8 inference works
   - Why now: the block format explosion (MX, NVFP4, MSFP)
   - Thesis: mixed-precision is the natural paradigm, not an optimization

2. The Physical World Sets the Precision Budget (1.5 pages)
   - Sensor resolution reality (table: no sensor > 20 ENOB)
   - Actuator resolution reality (table: 8-16 bits)
   - The pipeline: precision grows then shrinks
   - Figure 1: 5-stage pipeline with precision annotations
   - The mismatch: 64-bit processing of 12-bit data

3. The Energy Imperative (1.5 pages)
   - Horowitz's energy hierarchy (operation vs memory access costs)
   - Worked example: 4K@60fps vision system (FP32 vs mixed-precision)
   - The memory wall: smaller types = less bandwidth = less energy
   - Block formats as an energy strategy (shared scale amortizes metadata)
   - Table: energy savings by precision reduction at each pipeline stage

4. The Block Format Revolution (1 page)
   - Three strategies: power-of-two scaling (MX), fractional scaling (NVFP4),
     transform coding (ZFP)
   - Industry adoption: OCP standard, NVIDIA hardware, LLNL science
   - Our benchmark: nvfp4 achieves 3x lower RMSE than mxfp4; zfp achieves
     30+ dB better SNR at 8 bpv
   - The choice depends on data statistics, not convention

5. A Framework for Precision Selection (1.5 pages)
   - Task-based precision taxonomy (sensor → conditioning → features →
     decision → actuator)
   - The precision selection workflow:
     profile → analyze range → explore Pareto frontier → validate
   - The Universal Numbers Library as enabling infrastructure
   - Code example: plug-in replacement pattern
   - Figure 2: Pareto frontier (accuracy vs energy, multiple types)

6. Case Study: Mixed-Precision Robotics Pipeline (1 page)
   - Concrete example with real numbers
   - Perception: 8-bit quantized inference (camera → object detection)
   - State estimation: posit<32,2> Kalman filter
   - Control: fixpnt<16,8> PID loop
   - Energy breakdown: FP32 baseline vs mixed-precision
   - Result: N× energy reduction with <0.1% accuracy loss

7. Discussion and Future Directions (0.5 pages)
   - When higher precision IS needed (ill-conditioning, long accumulations)
   - Hardware co-design opportunity (FPGA/ASIC with right-sized datapaths)
   - Automatic precision tuning as compiler optimization
   - The precision-aware programming model

8. Conclusion: A Call to Action (0.5 pages)
   - Precision is a first-class design parameter — as important as
     algorithm selection
   - The tools exist (Universal, block formats, energy models)
   - The community should: teach mixed-precision design, build
     precision-aware toolchains, standardize energy-aware precision
     selection
   - Closing: "Bailey showed that sometimes you need more precision
     than you think. We show that usually you need less — and that
     the difference is measured in watts."
```

---

## Part 4: Implementation Roadmap

### Dependencies Between the Two Papers

The position paper can reference the systems paper for technical depth. This
means the position paper needs the systems paper's benchmark results but not
its implementation details. The two papers share:

- Block format benchmark data (RMSE, SNR, QSNR tables)
- Energy model estimates
- The Universal library as the enabling tool

The systems paper should be submitted first (to arXiv, no review delay), then
referenced in the position paper submission.

### Phase P1: Sharpen the Argument (Week 1)

Focus: distill the thesis, identify the "why now" moment, establish the foil.

| Task | Deliverable | Notes |
|------|-------------|-------|
| P1.1 Rewrite Introduction | Section 1 draft | The Bailey mirror + block format explosion as "why now" |
| P1.2 Research the foil | 1-page analysis | Why does the IEEE 754 monoculture persist? Compiler defaults, textbook conventions, hardware optimization. Who has written about this? |
| P1.3 Compile the "block format revolution" narrative | Section 4 draft | Timeline: MX spec (2023), NVFP4 (2024), MSFP, Apple, Qualcomm. Industry is voting with silicon. |
| P1.4 Identify target venue requirements | Formatting guide | IEEE CSE author guidelines, page limits, figure requirements |

**P1.2 detail**: The foil matters. Bailey's foil was "double precision is enough
for everything." Our foil is "double precision is the safe default." The paper
should name the forces that perpetuate this:
- **Education**: Numerical analysis courses teach in double precision
- **Compilers**: C/C++ default promotions go UP, never down
- **Libraries**: BLAS, LAPACK, NumPy all default to FP64
- **Hardware**: x86 FPU is 80-bit internally
- **Culture**: "Just use double" is the path of least resistance

This is not an attack on IEEE 754 (which is excellent for its purpose). It is
an argument that the default should be *chosen*, not inherited.

### Phase P2: Build the Evidence (Week 2)

Focus: gather concrete numbers for the energy section and case study.

| Task | Deliverable | Blocked By |
|------|-------------|------------|
| P2.1 Worked energy example: 4K vision | Complete calculation with source data | — |
| P2.2 Block format benchmark summary | Key numbers from systems paper | Systems paper Phase A |
| P2.3 Robotics pipeline case study | Run applications/mixed-precision/robotics/ with energy estimates | — |
| P2.4 Pareto frontier figure | Generate from ParetoExplorer with our type database | — |
| P2.5 Literature survey: related position papers | 10-15 references on mixed-precision, energy-aware computing | — |

**P2.1 detail**: The 4K@60fps vision example in the draft claims "3.3x power
reduction" but shows no work. Flesh it out:
- 4K = 3840 × 2160 = 8.3M pixels × 3 channels × 60fps = 1.49 Gpixel/s
- FP32: 1.49G × 4B = 5.96 GB/s input bandwidth alone
- Mixed 12-bit average: 1.49G × 1.5B = 2.24 GB/s
- DRAM energy at 20 pJ/byte: 119 mW vs 45 mW (2.6x savings on bandwidth alone)
- Add compute savings from 8-bit demosaic, 16-bit color, LUT gamma

**P2.3 detail**: The `applications/mixed-precision/robotics/` pipeline (19 KB,
real implementation) already exists. Run it with the instrumented types and
energy models to produce an actual energy breakdown by pipeline stage. This
is the case study for Section 6.

**P2.5 detail**: Key papers to find and cite:
- Horowitz (2014) — energy tables (already cited)
- Carson & Higham (2018) — iterative refinement in three precisions
- Abdelfattah et al. (2021) — mixed-precision in HPC survey
- Micikevicius et al. (2018) — mixed precision training (NVIDIA)
- Banner et al. (2018) — scalable quantization for DNNs
- OCP MX spec (2023) — industry standardization
- Gustafson (2017) — posits as alternative to IEEE 754
- Universal JOSS paper (2023) — our prior work
- Lindstrom (2014) — ZFP
- The NVIDIA NVFP4 blog/paper

### Phase P3: Write the Paper (Weeks 3-4)

| Task | Deliverable | Blocked By |
|------|-------------|------------|
| P3.1 Draft Sections 1-2 | Introduction + Physical World Interface | P1.1, P1.2 |
| P3.2 Draft Sections 3-4 | Energy Imperative + Block Format Revolution | P2.1, P2.2 |
| P3.3 Draft Sections 5-6 | Precision Framework + Case Study | P2.3, P2.4 |
| P3.4 Draft Sections 7-8 | Discussion + Conclusion | P3.1-P3.3 |
| P3.5 Create Figure 1 | Pipeline diagram with precision annotations | — |
| P3.6 Create Figure 2 | Pareto frontier (accuracy vs energy) | P2.4 |
| P3.7 Create Figure 3 | Block format comparison (RMSE bar chart) | P2.2 |
| P3.8 Compile references.bib | ~20 entries | P2.5 |
| P3.9 Format for IEEE CSE | Double-column, IEEE style | P3.4 |

**Writing principles for the position paper**:
- Every paragraph should advance the argument, not describe the tool
- Use the Universal library as evidence, not as the subject
- Data supports claims; claims are not about data
- One concrete example beats three hypothetical ones
- The reader should finish the paper thinking differently about precision

### Phase P4: Review and Submit (Week 5-6)

| Task | Deliverable | Blocked By |
|------|-------------|------------|
| P4.1 Internal review round 1 | Annotated draft with comments | P3.9 |
| P4.2 Revise for clarity and concision | Revised draft | P4.1 |
| P4.3 External review (1-2 colleagues) | Feedback | P4.2 |
| P4.4 Final revisions | Camera-ready | P4.3 |
| P4.5 Submit to IEEE CSE | Submission confirmation | P4.4 |

**P4.3 detail**: Ideal external reviewers:
- Someone in embedded systems who cares about energy
- Someone in scientific computing who knows Bailey's work
- Someone in ML who understands quantization
Each brings a different lens to the argument.

---

## Part 5: Governance & Accountability

### Roles

| Role | Responsibility |
|------|----------------|
| **Lead Author** | Thesis refinement, paper writing, voice and tone |
| **Data Lead** | Benchmark results, energy calculations, case study numbers |
| **Literature Lead** | Related work survey, reference compilation, positioning |

### Weekly Checkpoints

| Week | Milestone | Gate Criteria |
|------|-----------|---------------|
| 1 | Argument sharpened | Introduction rewritten; foil identified; "why now" articulated |
| 2 | Evidence gathered | Energy worked example complete; case study has real numbers; Pareto figure generated |
| 3 | First half drafted | Sections 1-4 written; Figures 1, 3 created |
| 4 | Full draft | All 8 sections written; all figures; references complete |
| 5 | Internal review | All feedback addressed; formatted for IEEE CSE |
| 6 | Submission | Submitted to venue |

### Quality Gates

**Phase P1 gate** (argument):
- [ ] Can state the thesis in one sentence without using the word "library"
- [ ] Can name the foil (what the paper argues against)
- [ ] Can answer "why now?" in two sentences
- [ ] Introduction does NOT describe the Universal library (that comes in Section 5)

**Phase P2 gate** (evidence):
- [ ] 4K vision worked example produces a specific watt number, not a range
- [ ] Robotics case study shows actual energy breakdown by stage
- [ ] Block format comparison uses data from real benchmarks, not projections
- [ ] At least 15 references identified

**Phase P3 gate** (writing):
- [ ] Paper reads as an argument, not a library description
- [ ] Each section has a clear claim it advances
- [ ] Figures are conceptual and persuasive, not just data dumps
- [ ] Conclusion leaves the reader with a memorable formulation
- [ ] Under 8 pages (IEEE CSE limit)

**Phase P4 gate** (submission):
- [ ] At least 2 external reviewers provided feedback
- [ ] All "placeholder" and "TODO" markers removed
- [ ] Formatted per venue requirements
- [ ] Co-authors have approved final version

### Risk Register

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Argument dismissed as "obvious" | Medium | High | Emphasize that obvious ≠ practiced; the IEEE 754 monoculture persists despite the argument being obvious. The contribution is the framework, not the observation. |
| Reviewer says "this is a product pitch" | Medium | High | Strict discipline: Universal appears only in Sections 5-6, never before. The argument stands without the library. |
| Energy numbers challenged | Medium | Medium | Clearly label as estimates; cite Horowitz (2014) and architecture manuals; offer to share calculation methodology |
| IEEE CSE rejects (wrong fit) | Low | Medium | CACM backup venue; arXiv as parallel track |
| Block format landscape moves too fast | Medium | Low | Focus on the taxonomy (scaling strategies), not specific formats |

---

## Part 6: The Closing Argument

The position paper's success depends on one thing: **the reader finishes the
paper and thinks differently about precision.** Not "this library is useful"
(that's the systems paper's job), but "I've been wasting energy because my
tools default to double and I never questioned it."

The paper's arc should be:

1. **Recognition** — "Bailey was right that precision matters. He showed you
   sometimes need more. We show you usually need less."

2. **Evidence** — "No sensor exceeds 20 bits. Your actuators accept 8-16 bits.
   The mismatch is 3-4x at every operation."

3. **Quantification** — "That mismatch costs you N watts in a 4K vision system.
   The block format revolution shows industry already knows this."

4. **Framework** — "Here's how to choose the right precision for each stage.
   Here are the tools."

5. **Call to action** — "Teach it. Build it. Standardize it."

The closing sentence should mirror Bailey:

> Bailey (2005): "In the future, the numeric precision used for a scientific
> computation may be as important to the program design as are the algorithms
> and data structures."

> Our paper: "For embedded intelligence, the numeric precision used at each
> computational stage is already as important as the algorithm itself — and the
> energy cost of ignoring this is measured in watts, not just ULPs."

---

## Part 7: Cross-References Between Papers

### What the Position Paper Can Cite from the Systems Paper

| Position Paper Section | Systems Paper Reference | What It Provides |
|------------------------|------------------------|-----------------|
| Section 4 (Block Formats) | Systems paper Section 4 + 6 | Detailed implementation; RMSE/SNR/QSNR tables |
| Section 5 (Framework) | Systems paper Section 3 + 5 | Library architecture; SDK components |
| Section 6 (Case Study) | Systems paper Section 6.4 | CG solver convergence data |

### What the Systems Paper Can Cite from the Position Paper

| Systems Paper Section | Position Paper Reference | What It Provides |
|----------------------|------------------------|-----------------|
| Section 1 (Introduction) | Position paper Section 1-2 | Motivation for why mixed-precision matters |
| Section 7 (Discussion) | Position paper Section 3 | Energy imperative framing |

### Submission Order

1. **Systems paper to arXiv** (first) — establishes the technical contribution,
   gets a citable reference
2. **Position paper to IEEE CSE** (second) — cites the arXiv systems paper for
   technical depth, focuses on the argument

The arXiv paper provides the data; the position paper provides the meaning.

---

## Appendix: Comparison with Bailey's Paper Structure

| Bailey (2005) | Our Paper | Parallel |
|---------------|-----------|----------|
| IEEE 754 double is the status quo | IEEE 754 double is the status quo | Same starting point |
| Some problems need MORE precision | Most embedded problems need LESS precision | Mirror image |
| Three tiers: dd, qd, arbitrary | Pipeline stages: 4-bit → 64-bit | Taxonomy of precision needs |
| QD library enables exploration | Universal library enables exploration | Tool as enabler |
| New math discoveries (compelling) | Energy savings (compelling) | Impact demonstration |
| "Precision will be as important as algorithms" | "Precision IS as important as algorithms" | Evolved thesis |

Bailey argued for the long tail to the right. We argue for the long tail to the
left. Together they make the case that **the center is the wrong default** — precision
should always be chosen, never assumed.
