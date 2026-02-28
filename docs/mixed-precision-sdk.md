# Mixed Precision Software Development Kit

Mixed-Precision SDK GAP Analysis Summary

## What EXISTS (Strong Foundation)

|    Category    |                               Components                               |                     Location                     |
|----------------|------------------------------------------------------------------------|--------------------------------------------------|
| Error Analysis | AbsoluteError, RelativeError, LogRelativeError, calculateNrOfValidBits | include/sw/universal/utility/error.hpp           |
| Error-Free Ops | two_sum, two_prod, two_diff for exact arithmetic                       | include/sw/universal/numerics/error_free_ops.hpp |
| Quantization   | qsnr() - Signal-to-Quantization-Noise Ratio                            | include/sw/universal/quantization/qsnr.hpp       |
| Stability      | condest() - condition number, nbe() - backward error                   | include/sw/blas/utes/                            |
| Performance    | PerformanceRunner benchmark framework                                  | include/sw/universal/benchmark/                  |
| BLAS           | Mixed-precision CG variants (fdp_fdp, dot_fdp, etc.)                   | include/sw/blas/solvers/                         |
| Examples       | DNN inference, DSP, roots, interpolation, integration                  | mixedprecision/, applications/mixed-precision/   |


## CRITICAL GAPS (Missing)

|              Gap              |                                  Description                                   |                Impact                 |
|-------------------------------|--------------------------------------------------------------------------------|---------------------------------------|
| Type Selection Framework      | No automated recommendation of posit vs cfloat vs fixpnt based on requirements | Users guess at type selection         |
| Error Budget Analyzer         | No error allocation across algorithm steps                                     | Can't predict accuracy before running |
| Precision Scheduling          | No framework for mixed-precision assignments                                   | Manual trial-and-error optimization   |
| Dynamic Range Analysis        | No saturation/overflow/underflow detection                                     | Silent precision loss                 |
| Algorithm Instrumentation API | No standard hooks for metrics collection                                       | Can't profile mixed-precision codes   |

## IMPORTANT GAPS


|            Gap            |                 Description                 |
|---------------------------|---------------------------------------------|
| Memory Footprint Analysis | No per-operation memory estimation          |
| Bandwidth Prediction      | No data movement quantification             |
| Cache-Aware Tools         | No cache line optimization                  |
| Auto-Tuning Framework     | No search for optimal precision assignments |
| Regression Detection      | No automated baseline comparison            |


## Proposed SDK Structure

```text
  include/sw/universal/sdk/
  ├── type_selection/
  |   ├── type_advisor.hpp         # Recommend types from requirements
  |   └── precision_calibrator.hpp # Calibrate for specific algorithms
  ├── error_analysis/
  |   ├── error_budget.hpp         # Allocate error across operations
  |   └── error_propagator.hpp     # Track error growth
  ├── quantization/
  |   ├── range_analyzer.hpp       # Dynamic range analysis
  |   └── overflow_detector.hpp    # Saturation detection
  └── profiling/
      ├── instrumentation.hpp      # Metrics collection hooks
      └── performance_model.hpp    # Execution time prediction
```

## Quick Wins (High Value, Low Effort)

  1. Type Advisor - Given range + error tolerance → recommend type
  2. Range Analyzer - Detect overflow/underflow risks
  3. Error Propagation Tracker - Wrap ops with cumulative error tracking
  4. Performance Predictor - Empirical model from benchmarks

## Completeness Assessment


|                         Tier                         |  Description  | Status |
|------------------------------------------------------|---------------|--------|
| Foundational (types, BLAS, basic error)              | ~70% complete |        |
| Workflow Support (calibration, tuning)               | ~40% complete |        |
| Algorithm Design Support (advisors, instrumentation) | ~20% complete |        |



## Energy Efficiency GAP Analysis

Current State: Named but Not Implemented

The benchmark/energy/ directory exists but does NOT measure energy - it only counts operations and measures accuracy. This is a critical gap since energy
efficiency is a PRIMARY motivation for mixed-precision.

What EXISTS for Energy


|     Component      |                Location                 |         What It Does          |       Energy Support        |
|--------------------|-----------------------------------------|-------------------------------|-----------------------------|
| Operation Counting | utility/occurrence.hpp                  | Counts add/mul/div/load/store | No cost model               |
| Performance Runner | benchmark/performance_runner.hpp        | Wall-clock timing             | Time only, not Joules       |
| Memory Footprint   | applications/mixed-precision/dnn/conv2d | Tracks bytes used             | No energy conversion        |
| BLAS Benchmarks    | benchmark/energy/blas/                  | Dot, GEMM, MatVec             | Misleading name - no energy |


### CRITICAL GAPS (100% Missing)

```text
  Gap: Per-Operation Energy Costs
  Description: Cost tables mapping (operation + bit-width) → picojoules
  Why Critical: 8-bit mul uses ~10x less energy than 32-bit - this IS mixed-precision's value proposition
  ────────────────────────────────────────
  Gap: Hardware Counter Integration
  Description: RAPL (Intel), NVML (GPU), ARM energy probes
  Why Critical: Can't measure actual energy without hardware hooks
  ────────────────────────────────────────
  Gap: Memory Hierarchy Energy Model
  Description: L1/L2/L3/DRAM access costs
  Why Critical: Memory dominates energy in most algorithms
  ────────────────────────────────────────
  Gap: Energy-Aware Type Selector
  Description: Given accuracy + energy budget → recommend types
  Why Critical: Users can't make energy-informed decisions
  Energy Cost Reality (What's Missing)
```

```cpp
  // MISSING: This is what users need to make decisions
  constexpr struct IntelSkylakeEnergy {
      // Operations (picojoules)
      double add_8bit  = 0.5;   double add_32bit = 2.0;   // 4x ratio
      double mul_8bit  = 2.0;   double mul_32bit = 10.0;  // 5x ratio
      double div_8bit  = 10.0;  double div_32bit = 50.0;  // 5x ratio

      // Memory access (picojoules)
      double L1_access = 10;
      double L2_access = 50;
      double L3_access = 200;
      double DRAM_access = 1000;  // 100x more than L1!
  };
```

### Recommended SDK Structure for Energy

```text
  include/sw/universal/energy/
  ├── cost_models/
  |   ├── intel_skylake.hpp      # Per-op energy costs
  |   ├── arm_cortex_a.hpp
  |   └── generic.hpp            # Conservative defaults
  ├── hw_counters/
  |   ├── rapl.hpp               # Intel/AMD energy counters
  |   ├── nvml.hpp               # NVIDIA GPU energy
  |   └── perf_event.hpp         # Linux performance counters
  ├── profilers/
  |   ├── operation_energy.hpp   # Extend occurrence.hpp with costs
  |   ├── memory_energy.hpp      # Cache miss → energy
  |   └── algorithm_energy.hpp   # Full algorithm profiling
  └── optimization/
      ├── energy_type_advisor.hpp    # Recommend types for energy budget
      ├── energy_budget.hpp          # Allocate energy across steps
      └── pareto_explorer.hpp        # Energy vs accuracy tradeoffs
```


### Priority Implementation Order


| Priority |           Component            | Effort  |            Impact             |
|----------|--------------------------------|---------|-------------------------------|
| P1       | Energy cost tables (data)      | 1 week  | Enables all energy reasoning  |
| P1       | RAPL integration (Linux/Intel) | 1 week  | Actual energy measurement     |
| P2       | occurrence_with_energy.hpp     | 3 days  | Maps counts → joules          |
| P2       | Memory energy profiler         | 1 week  | Quantifies data movement cost |
| P3       | Energy-aware type advisor      | 2 weeks | Automated type selection      |
| P3       | Pareto front explorer          | 1 week  | Visualize tradeoffs           |


### Summary Assessment


|      Category      | Previous Assessment |          Revised with Energy Focus          |
|--------------------|---------------------|---------------------------------------------|
| Numerical Tools    | 70% complete        | 70% (unchanged)                             |
| Quantization Tools | 40% complete        | 40% (unchanged)                             |
| Memory Tools       | 20% complete        | 20% (unchanged)                             |
| Energy Tools       | Not assessed        | 5% complete (only op counting exists)       |
| Overall SDK        | ~45%                | ~35% (energy is foundational, not optional) |

The library cannot currently answer the fundamental question: "How much energy will my
mixed-precision algorithm use?"



