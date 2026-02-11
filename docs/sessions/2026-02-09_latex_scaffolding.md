# Development Session: LaTeX Scaffolding for arXiv Systems Paper

**Date:** 2026-02-09 (session 3)
**Branch:** v3.96
**Focus:** Create LaTeX paper scaffolding for arXiv cs.MS submission
**Status:** Complete

## Session Overview

Created the LaTeX paper scaffolding in `papers/systems-paper/paper/` so that
writing the actual systems paper can begin. The paper targets arXiv cs.MS
(Mathematical Software) and uses plain `article` document class (12pt,
single-column) for maximum arXiv compatibility.

## What Was Created

### 1. `papers/systems-paper/paper/main.tex` (754 lines)

Full section structure with `% TODO` placeholders:

| Section | Title | Est. Pages |
|---------|-------|-----------|
| 1 | Introduction | 1.5 |
| 2 | Background and Related Work | 1.5 |
| 3 | Library Architecture | 2 |
| 4 | Block Format Implementations | 2.5 |
| 5 | Mixed-Precision Solver Case Studies | 3 |
| 6 | Discussion | 1 |
| 7 | Conclusion | 0.5 |
| A | Number System Inventory (appendix) | 1 |

Key features:
- One populated table already in place (block format API comparison)
- Commented-out table stubs for solver results (IR, CG, IDR(s))
- Subsections for each block format (mxblock, nvblock, zfpblock)
- Subsections for each solver case study matching existing `.cpp` files
- Packages: amsmath, amssymb, booktabs, hyperref, listings, xcolor, graphicx, algorithm2e

### 2. `papers/systems-paper/paper/references.bib` (29 entries)

Merged 14 existing entries from `joss/references.bib` with 15 new entries:

| Key | Topic |
|-----|-------|
| gustafson:2017 | Posit standard / unum computing |
| ieee754:2019 | IEEE 754-2019 standard |
| ocp_mx:2023 | OCP Microscaling Formats spec v1.0 |
| nvidia_fp4:2024 | NVIDIA Blackwell FP4 architecture |
| lindstrom:2014 | ZFP fixed-rate lossy compression |
| bailey:2005 | High-precision floating-point arithmetic |
| horowitz:2014 | Computing's energy problem (ISSCC) |
| sonneveld:2008 | IDR(s) algorithm |
| vangijzen:2011 | Algorithm 913: IDR(s) implementation |
| higham:2002 | Accuracy and Stability of Numerical Algorithms |
| saad:2003 | Iterative Methods for Sparse Linear Systems |
| fousse:2007 | MPFR library |
| flegar:2019 | FloatX library |
| dettmers:2022 | LLM.int8() quantization |
| micikevicius:2018 | Mixed precision training |

### 3. `papers/systems-paper/paper/Makefile`

Standard pdflatex + bibtex triple-pass recipe with `clean` target.

## Verification

- All `\begin`/`\end` pairs balanced (15 environments)
- All braces balanced (144 open / 144 close)
- All BibTeX entries have required fields and balanced braces
- Makefile uses tabs (not spaces) for recipe lines
- No pdflatex installed on this machine; compilation deferred to Overleaf

## Workflow Note

The user will use Overleaf for final compilation and editing. Files can be
uploaded as a zip or individually. The Makefile is provided for optional
local builds once TeX Live is installed.

## Commits

| Hash | Message |
|------|---------|
| f404aeed | Add LaTeX scaffolding for arXiv systems paper |

## Next Steps

- Upload to Overleaf and verify compilation
- Begin writing Section 4 (Block Format Implementations) — strongest contribution
- Run the three solver case studies to generate data for Section 5 tables
- Create figures (pipeline diagram, storage layouts, RMSE bar charts)
