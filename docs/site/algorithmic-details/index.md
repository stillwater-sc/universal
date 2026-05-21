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
- [Lazy exact real arithmetic](../algorithmic-details/lazy-real-arithmetic/)
  -- McCleeary's lazy-stream paradigm as shipped in `elreal`: the
  refinement protocol, sign-determination with bounded budget, depth-1
  derivative-based math, geometric predicates, and the cross-implementation
  validation oracle.
- [elreal performance baseline](../algorithmic-details/elreal-performance-baseline/)
  -- Phase I throughput measurements: per-operation cost shape,
  matched-precision comparison against `ereal<N>`, and the allocator
  hot path that bounds today's throughput.

## Companion sections

- [Number Systems](../number-systems/) -- API-level reference for each
  arithmetic type.
- [Design](../design/) -- design rationale and architectural decisions.
