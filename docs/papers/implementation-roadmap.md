# Implementation Roadmap: Mixed-Precision Position Paper & SDK

## Overview

This roadmap supports two parallel tracks:
1. **Paper Track**: Writing and supporting the position paper
2. **SDK Track**: Implementing missing components identified in GAP analysis

---

## Phase 1: Foundation (Weeks 1-2)

### Paper Track

- [ ] Finalize paper title and abstract
- [ ] Complete Section 3 (Physical World Interface) - strongest novel contribution
- [ ] Draft Section 4 (Mixed-Precision Thesis)
- [ ] Compile existing benchmarks into tables

### SDK Track

#### P1.1: Energy Cost Tables (Critical - enables all energy reasoning)
```
Location: include/sw/universal/energy/cost_models/
Files:
  - intel_skylake.hpp
  - arm_cortex_a.hpp
  - generic.hpp
```
- Populate with empirical data from ITRS, manufacturer specs, research papers
- Per-operation costs: add, mul, div, fma @ each bit-width (8, 16, 32, 64)
- Memory access costs: L1, L2, L3, DRAM

#### P1.2: Extend occurrence.hpp → occurrence_with_energy.hpp
```
Location: include/sw/universal/energy/operation_energy.hpp
```
- Attach energy costs to operation counts
- `total_energy_joules()` method
- Architecture-selectable cost model

---

## Phase 2: Demonstration (Weeks 3-4)

### Paper Track

- [ ] Write Section 5 (Compelling Applications) with concrete examples
- [ ] Draft Section 6 (Universal Solution) - library description
- [ ] Create energy benchmark tables for Section 7
- [ ] Begin Section 2 (Introduction) framing

### SDK Track

#### P2.1: Dynamic Range Analyzer
```
Location: include/sw/universal/sdk/range_analyzer.hpp
```
- Track min/max values through computation
- Detect overflow/underflow risks
- Recommend type bit-widths

#### P2.2: Type Advisor (First Version)
```
Location: include/sw/universal/sdk/type_advisor.hpp
```
- Input: value range, error tolerance, performance target
- Output: recommended number system and configuration
- Uses existing error utilities and number system properties

#### P2.3: RAPL Integration (Linux/Intel)
```
Location: include/sw/universal/energy/hw_counters/rapl.hpp
```
- Read Intel RAPL MSRs or /sys/class/powercap
- Start/stop energy measurement
- Report package, core, DRAM energy

---

## Phase 3: Validation (Weeks 5-6)

### Paper Track

- [ ] Run comprehensive benchmarks for paper figures
- [ ] Complete Section 7 (Energy Efficiency Results)
- [ ] Write Section 8 (Discussion)
- [ ] Draft Section 1 (Introduction) and Section 9 (Conclusion)
- [ ] Internal review and revision

### SDK Track

#### P3.1: Error Propagation Tracker
```
Location: include/sw/universal/sdk/error_propagator.hpp
```
- Wrap operations with error tracking
- Report cumulative error vs tolerance
- Uses error_free_ops.hpp infrastructure

#### P3.2: Memory Energy Profiler
```
Location: include/sw/universal/energy/memory_energy.hpp
```
- Estimate memory access patterns
- Map to memory hierarchy energy model
- Integrate with perf_event for cache miss counting (optional)

#### P3.3: Algorithm Energy Profiler
```
Location: include/sw/universal/energy/algorithm_profiler.hpp
```
- Combine operation energy + memory energy
- Per-function, per-loop energy reporting
- Enable energy-aware optimization

---

## Phase 4: Publication (Weeks 7-8)

### Paper Track

- [ ] Final revisions based on feedback
- [ ] Prepare supplementary materials (code, data)
- [ ] Submit to target venue
- [ ] Prepare presentation/talk materials

### SDK Track

#### P4.1: Pareto Front Explorer
```
Location: include/sw/universal/energy/pareto_explorer.hpp
```
- Sweep precision configurations
- Plot energy vs accuracy tradeoffs
- Identify Pareto-optimal configurations

#### P4.2: Auto-Tuning Framework (Stretch Goal)
```
Location: include/sw/universal/sdk/auto_tuner.hpp
```
- Search space: bit-widths per algorithm step
- Objective: minimize energy subject to accuracy constraint
- Method: grid search or evolutionary algorithm

---

## SDK Component Priority Matrix

| Component | Priority | Effort | Paper Support | Standalone Value |
|-----------|----------|--------|---------------|------------------|
| Energy cost tables | P1 | 1 week | Critical | High |
| occurrence_with_energy | P1 | 3 days | Critical | High |
| RAPL integration | P1 | 1 week | High | High |
| Range analyzer | P2 | 3 days | Medium | High |
| Type advisor | P2 | 1 week | Medium | Very High |
| Error propagator | P2 | 1 week | Medium | High |
| Memory energy profiler | P2 | 1 week | High | High |
| Algorithm profiler | P3 | 1 week | High | Very High |
| Pareto explorer | P3 | 1 week | Medium | High |
| Auto-tuner | P4 | 2 weeks | Low | Very High |

---

## GAP Closure Tracking

### Numerical Tools (Currently 70%)
- [x] Error measurement (AbsoluteError, RelativeError, etc.)
- [x] Error-free operations (two_sum, two_prod)
- [x] Condition number estimation
- [ ] ULP tracking framework
- [ ] Forward/backward error propagation

### Quantization Tools (Currently 40%)
- [x] QSNR calculation
- [x] Type conversion utilities
- [ ] Dynamic range analysis ← P2.1
- [ ] Type selection framework ← P2.2
- [ ] Quantization error propagation

### Memory Tools (Currently 20%)
- [x] Memory footprint tracking (conv2d example)
- [ ] Per-operation memory estimation
- [ ] Cache-aware layout tools
- [ ] Bandwidth prediction

### Energy Tools (Currently 5%)
- [x] Operation counting (occurrence.hpp)
- [ ] Energy cost models ← P1.1
- [ ] Operation → Joules mapping ← P1.2
- [ ] Hardware counter integration ← P2.3
- [ ] Memory energy quantification ← P3.2
- [ ] Algorithm energy profiling ← P3.3

### Profiling Tools (Currently 10%)
- [x] Performance timing
- [x] Operation counting
- [ ] Instrumentation API
- [ ] Runtime precision annotation

---

## Milestones

| Week | Paper Milestone | SDK Milestone |
|------|-----------------|---------------|
| 1 | Outline finalized, Section 3 drafted | Energy cost tables populated |
| 2 | Sections 3-4 complete | occurrence_with_energy working |
| 3 | Section 5 drafted | Range analyzer complete |
| 4 | Sections 5-6 complete | Type advisor v1, RAPL working |
| 5 | Benchmarks running | Error propagator complete |
| 6 | Section 7 with data | Memory/algorithm profilers |
| 7 | Full draft complete | Pareto explorer |
| 8 | Submission ready | SDK documented |

---

## Success Criteria

### Paper
- [ ] Accepted at target venue (IEEE CSE, CACM, or IEEE Micro)
- [ ] Clear articulation of mixed-precision thesis
- [ ] Compelling energy efficiency data
- [ ] Universal positioned as enabling tool

### SDK
- [ ] Can answer: "How much energy will my mixed-precision algorithm use?"
- [ ] Can recommend types given accuracy + energy constraints
- [ ] Integrated profiling for mixed-precision codes
- [ ] Documented with examples

---

## Resources Needed

- Access to Intel/AMD systems for RAPL measurements
- ARM development board for ARM energy profiling (optional)
- GPU with NVML support for GPU energy (optional)
- Literature: ITRS roadmap, energy modeling papers, architecture specs

---

## Risk Mitigation

| Risk | Mitigation |
|------|------------|
| Energy data not available | Use published research data, clearly cite sources |
| RAPL not accessible | Provide software estimation fallback |
| Benchmarks don't show expected savings | Focus on specific domains where savings are clear |
| Paper scope too broad | Narrow to specific application domain if needed |
