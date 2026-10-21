---
title: Algorithmic Details
description: Deep technical write-ups of selected algorithms used inside Universal, with math derivations and transfer-function plots.
---

Deep technical write-ups of selected algorithms used inside Universal.
These docs are for readers who want the math, the transfer functions, and
the trade-off space rather than the API-level usage.

## Available Documents

- [LNS log-domain add/sub algorithms](../algorithmic-details/lns-log-add-sub/)
  -- the seven shipped algorithms for evaluating `log2(1 + 2^d)` and
  `log2(1 - 2^d)`, with transfer-function plots, error envelopes, and a
  picker decision tree.
- [Multi-component floating-point arithmetic](../algorithmic-details/multi-component-arithmetic/)
  -- Priest's error-free transformations, Bailey/Hida's hand-crafted
  fixed-precision `dd` / `qd`, Shewchuk's adaptive expansions, and how
  Universal's `floatcascade<N>` building block ties them together.
- [Multi-limb arithmetic](../algorithmic-details/multi-limb-arithmetic/)
  -- how the fixed-size types store integers and significands in blocks, the
  32-bit-limb kernels that add, multiply (schoolbook) and divide (Knuth's
  Algorithm D) them under `blocktriple`, `integer` and `blockbinary`, how
  those changes were verified bit-identical, and what they gained.
- [efloat: correctly rounded arithmetic and math library](../algorithmic-details/efloat-mathlib/)
  -- how `efloat` rounds every function correctly at any precision (Ziv's
  strategy over stated error bounds), the algorithm behind each function, the
  alternatives tried and dropped, how it is verified against MPFR, measured
  accuracy and speed, and the open work.

## Companion sections

- [Number Systems](../number-systems/) -- API-level reference for each
  arithmetic type.
- [Design](../design/) -- design rationale and architectural decisions.
