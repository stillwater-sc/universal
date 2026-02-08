# Development Session: Mixed-Precision Algorithm Design SDK

**Date:** 2026-02-03
**Branch:** v3.94
**Focus:** Complete SDK for energy-aware mixed-precision algorithm design
**Status:** ✅ Complete

## Session Overview

This session implemented a comprehensive SDK for mixed-precision algorithm design, enabling developers to make informed decisions about precision selection based on accuracy requirements and energy constraints. The implementation followed a three-phase roadmap covering energy cost modeling, analysis tools, and optimization utilities.

### Goals Achieved
- ✅ Energy cost tables for multiple architectures (45nm, Skylake, ARM Cortex-A)
- ✅ RAPL hardware energy measurement integration
- ✅ Range analyzer for tracking value distributions
- ✅ Type advisor for precision recommendations
- ✅ Memory profiler for cache/DRAM energy modeling
- ✅ Algorithm profiler combining all analysis dimensions
- ✅ Pareto explorer for accuracy/energy trade-offs
- ✅ Configuration generator for mixed-precision code

## Key Decisions

### 1. Energy Model Granularity
**Decision:** Per-operation energy costs in picojoules (pJ) by bit-width (8/16/32/64-bit)

**Rationale:** This granularity allows:
- Direct comparison between precision levels
- Algorithm-level energy estimation
- Memory vs compute energy breakdown

**Trade-off:** Models estimate marginal per-operation energy, not total system energy. RAPL measures total package power for validation.

### 2. RAPL Implementation Strategy
**Decision:** Use Linux powercap sysfs interface with zero external dependencies

**Rationale:**
- No library dependencies (unlike PAPI, likwid)
- Works on any Linux kernel ≥3.13
- Graceful degradation on unsupported platforms

**Implementation:** Platform guards with stubs for macOS/Windows:
```cpp
#if defined(__linux__)
    #define UNIVERSAL_RAPL_LINUX 1
#elif defined(__APPLE__)
    #define UNIVERSAL_RAPL_MACOS 1
#elif defined(_WIN32)
    #define UNIVERSAL_RAPL_WINDOWS 1
#endif
```

### 3. Type Recommendation Approach
**Decision:** Pareto-optimal frontier analysis with multi-objective optimization

**Rationale:** Precision selection involves trade-offs between:
- Accuracy (relative error tolerance)
- Energy consumption
- Memory bandwidth

A Pareto frontier identifies configurations where no other option is better in all dimensions.

### 4. Architecture Coverage
**Decision:** Include IEEE floats, posits, fixed-point, and LNS in recommendations

**Rationale:** Different number systems excel in different scenarios:
- Posits: Wide dynamic range, gradual underflow
- Fixed-point: Narrow range, high precision
- LNS: Efficient multiplication
- IEEE floats: Hardware support, compatibility

## Implementation Details

### Phase 1: Energy Cost Infrastructure

#### Energy Cost Model (energy_model.hpp)
```cpp
struct EnergyCostModel {
    const char* name;
    const char* description;
    int process_nm;

    struct OperationCosts {
        double int_add[4];   // 8, 16, 32, 64-bit
        double int_mul[4];
        double fp_add[4];
        double fp_mul[4];
        double fp_fma[4];
        // ...
    } ops;

    struct MemoryCosts {
        double reg_read, reg_write;
        double l1_read, l1_write;
        double l2_read, l2_write;
        double l3_read, l3_write;
        double dram_read, dram_write;
    } mem;
};
```

#### Architecture Models
| Model | Process | FP32 FMA (pJ) | DRAM Read (pJ) |
|-------|---------|---------------|----------------|
| Generic 45nm | 45nm | 4.5 | 1300 |
| Intel Skylake | 14nm | 1.5 | 650 |
| ARM Cortex-A76 | 7nm | 0.75 | 400 |
| ARM Cortex-A55 | 7nm | 0.30 | 400 |

#### RAPL Integration (rapl.hpp)
```cpp
class RaplReader {
public:
    static bool isAvailable();
    void start();
    RaplEnergy stop();

    bool hasPackage() const;
    bool hasCores() const;    // PP0
    bool hasUncore() const;   // PP1
    bool hasDram() const;
};
```

### Phase 2: Analysis Tools

#### Range Analyzer
Tracks per-variable statistics:
- Min/max values and absolute values
- Scale (exponent) range
- Zeros, denormals, infinities, NaNs
- Dynamic range utilization

```cpp
range_analyzer<double> analyzer;
for (auto v : data) analyzer.observe(v);
auto rec = analyzer.recommendPrecision();
// rec.type_suggestion = "half (cfloat<16,5>) or posit<16,1>"
```

#### Memory Profiler
Models cache hierarchy energy:
```cpp
MemoryProfiler profiler;
profiler.recordRead(matrix_bytes, AccessPattern::Sequential);
profiler.setWorkingSetSize(total_bytes);

auto tier = profiler.estimatePrimaryTier();  // L1, L2, L3, or DRAM
double energy_pj = profiler.estimateEnergyPJ();
```

### Phase 3: Optimization Tools

#### Algorithm Profiler
Unified view combining:
- Operation counts (adds, muls, FMAs, etc.)
- Memory access (bytes read/written, working set)
- Energy breakdown (compute vs memory)
- Arithmetic intensity (ops/byte)

```cpp
auto profile = AlgorithmProfiler::profileGEMM(1024, 1024, 1024, "FP32", 32);
// profile.total_ops = 1,073,741,824
// profile.working_set_bytes = 12 MB
// profile.total_energy_pj = 1,660,730,000 pJ
```

#### Pareto Explorer
Identifies optimal trade-offs:
```cpp
ParetoExplorer explorer;
auto result = explorer.computeFrontier();

// Frontier: posit<8,0>, INT8, INT16, posit<32,2>, posit<64,3>
auto best = result.bestForAccuracy(1e-4);  // INT16
```

#### Configuration Generator
Produces ready-to-use headers:
```cpp
PrecisionConfigGenerator gen;
gen.setAlgorithm("GEMM");
gen.setAccuracyRequirement(1e-4);
gen.setEnergyBudget(0.3);

std::cout << gen.generateConfigHeader();
// Outputs: namespace gemm_config { using InputType = ...; }
```

## Technical Insights

### Energy Dominance Analysis

For GEMM 1024x1024 (12 MB working set):
```
Compute energy: 1,610 uJ (97%)
Memory energy:     50 uJ (3%)
```

For GEMM 4096x4096 (192 MB working set):
```
Compute energy: 25,800 uJ (93%)
Memory energy:   1,965 uJ (7%)
```

**Insight:** Compute dominates for dense linear algebra, but memory becomes significant for:
- Large matrices exceeding L3 cache
- Sparse operations
- Memory-bound algorithms

### Precision Scaling Laws

Empirical energy scaling from models:
| Precision | FMA Energy | vs FP32 |
|-----------|------------|---------|
| INT8 | 0.2 pJ | 0.13x |
| FP16 | 0.47 pJ | 0.31x |
| FP32 | 1.5 pJ | 1.00x |
| FP64 | 5.3 pJ | 3.53x |

**Insight:** Energy scales roughly with bit-width squared for multiplication (quadratic transistor count).

### Model vs RAPL Comparison

Model estimates **marginal** per-operation energy.
RAPL measures **total** package power.

Example discrepancy:
- Model: 1.5 uJ for 1M FMAs
- RAPL: 300,000 uJ measured

The 200,000x difference comes from:
- Static/leakage power (~10-30W baseline)
- Pipeline overhead (fetch, decode, retire)
- All cores (not just active one)
- Memory controller, uncore

**Use case distinction:**
- Models: Relative comparisons, algorithm design
- RAPL: Absolute measurements, validation

## Testing & Validation

### Benchmarks Created
1. `energy_models_energy_models` - Energy cost tables and comparisons
2. `energy_hw_counters_rapl_measurement` - RAPL hardware measurement
3. `accuracy_range_range_analyzer` - Range analysis and type advisor
4. `accuracy_range_algorithm_profiler` - Algorithm profiling and Pareto

### Key Test Results

**GEMM Precision Comparison:**
```
Precision     Energy (uJ)    vs FP32
FP64              5867.85      3.53x
FP32              1660.73      1.00x
FP16               510.91      0.31x
INT8               217.76      0.13x
```

**Conv2D (ResNet-like) Totals:**
```
Precision     Energy (uJ)    Savings
FP32              2011.90         -
FP16               618.35      69.3%
INT8               263.77      86.9%
```

## Challenges & Solutions

### Challenge 1: RAPL Permissions
**Problem:** `/sys/class/powercap/intel-rapl/` requires root read access by default.

**Solution:**
- Graceful degradation (returns 0 energy when permissions denied)
- Documentation: `sudo chmod -R a+r /sys/class/powercap/intel-rapl/`

### Challenge 2: Type Name Conflicts
**Problem:** `profileGEMM()` static method conflicted with `sw::universal::profileGEMM()` free function.

**Solution:** Fully qualified call: `sw::universal::profileGEMM(M, N, K, elem_size, cache)`

### Challenge 3: Double Overflow for LNS Range
**Problem:** `lns<32,16>` has range beyond `double` (10^±4932).

**Solution:** Use `std::numeric_limits<double>::max()` as proxy for comparison.

## Performance Characteristics

### Compilation Impact
- Header-only: No link-time overhead
- Template instantiation: Moderate compile time for heavily templated code

### Runtime Overhead
- Range analyzer: O(1) per observation
- Memory profiler: O(1) per region
- Pareto computation: O(n²) for n configurations (typically <20)

## Files Created

### Headers (12 files)
```
include/sw/universal/energy/
├── cost_models/
│   ├── energy_model.hpp
│   ├── generic_45nm.hpp
│   ├── intel_skylake.hpp
│   └── arm_cortex_a.hpp
├── energy.hpp
├── occurrence_energy.hpp
└── hw_counters/
    └── rapl.hpp

include/sw/universal/utility/
├── range_analyzer.hpp
├── type_advisor.hpp
├── memory_profiler.hpp
├── algorithm_profiler.hpp
├── pareto_explorer.hpp
└── precision_config_generator.hpp
```

### Benchmarks (3 files)
```
benchmark/
├── energy/
│   ├── models/energy_models.cpp
│   └── hw_counters/rapl_measurement.cpp
└── accuracy/
    └── range/
        ├── range_analyzer.cpp
        └── algorithm_profiler.cpp
```

## Commits

```
e4985609 Add RAPL hardware energy measurement via Linux powercap sysfs
f2e182e6 Add range analyzer and type advisor for mixed-precision design
52de89dd Add memory access profiler for cache/DRAM energy estimation
25ef39dc Add Phase 3 mixed-precision optimization tools
```

## Next Steps

### Immediate Follow-ups
1. Integration with existing `occurrence<T>` instrumented number types
2. Add more architecture models (AMD Zen, Apple M-series)
3. Extend Pareto explorer with memory bandwidth dimension

### Future Enhancements
1. **Autotuning:** Automatically select precision per kernel
2. **Profile-guided optimization:** Use RAPL measurements to refine models
3. **Mixed-precision BLAS:** Apply recommendations to sw::universal::blas

## References

1. Horowitz, M. "Computing's Energy Problem (and what we can do about it)" ISSCC 2014
2. Intel RAPL Documentation: https://www.kernel.org/doc/html/latest/power/powercap/powercap.html
3. ARM Cortex-A Technical Reference Manuals
4. Blem et al. "Power Struggles: Revisiting the RISC vs. CISC Debate" ISCA 2013

## Appendix

### Build Commands
```bash
cd build
cmake -DUNIVERSAL_BUILD_BENCHMARK_ENERGY=ON -DUNIVERSAL_BUILD_BENCHMARK_ACCURACY=ON ..
make energy_models_energy_models
make energy_hw_counters_rapl_measurement
make accuracy_range_algorithm_profiler

# Run
./benchmark/energy/models/energy_models_energy_models
sudo ./benchmark/energy/hw_counters/energy_hw_counters_rapl_measurement
./benchmark/accuracy/range/accuracy_range_algorithm_profiler
```

### Quick Usage Example
```cpp
#include <universal/utility/pareto_explorer.hpp>
#include <universal/utility/precision_config_generator.hpp>

using namespace sw::universal;

int main() {
    // Find best precision for ML inference
    ParetoExplorer explorer;
    auto best = explorer.recommendForAccuracy(1e-2);
    std::cout << "Best for ML: " << best.name << "\n";  // INT8

    // Generate configuration header
    PrecisionConfigGenerator gen;
    gen.setAlgorithm("MyGEMM");
    gen.setAccuracyRequirement(1e-4);
    gen.setEnergyBudget(0.3);
    std::cout << gen.generateConfigHeader();
}
```

---

**Session Duration:** ~3 hours
**Lines of Code:** ~4,500
**Files Created:** 15
**Commits:** 4
