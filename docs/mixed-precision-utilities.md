# Mixed-Precision Algorithm Design Utilities

This document describes the utilities in `include/sw/universal/utility/` that support mixed-precision algorithm design and optimization. These tools help you analyze value ranges, estimate energy consumption, explore accuracy/energy trade-offs, and generate optimized precision configurations.

## Table of Contents

1. [Overview](#overview)
2. [Core Utilities](#core-utilities)
   - [occurrence.hpp](#occurrencehpp---operation-counting)
   - [scale_tracker.hpp](#scale_trackerhpp---exponent-distribution-tracking)
   - [range_analyzer.hpp](#range_analyzerhpp---value-range-analysis)
   - [type_advisor.hpp](#type_advisorhpp---type-recommendation)
   - [memory_profiler.hpp](#memory_profilerhpp---memory-access-profiling)
   - [algorithm_profiler.hpp](#algorithm_profilerhpp---unified-algorithm-profiling)
   - [pareto_explorer.hpp](#pareto_explorerhpp---accuracyenergy-trade-off-analysis)
   - [precision_config_generator.hpp](#precision_config_generatorhpp---code-generation)
   - [instrumented.hpp](#instrumentedhpp---operation-instrumentation)
3. [Workflow Guide](#workflow-guide)
4. [Complete Example](#complete-example)

---

## Overview

### Why Mixed-Precision?

Mixed-precision computing uses different numerical precisions for different parts of an algorithm to optimize:
- **Energy efficiency**: Lower-precision operations consume less power
- **Memory bandwidth**: Smaller data types reduce memory traffic
- **Performance**: More values fit in cache; SIMD lanes can process more elements

The challenge is selecting the right precision for each variable without sacrificing accuracy.

### The Mixed-Precision SDK

The Universal library provides a complete SDK for mixed-precision algorithm design:

```
┌─────────────────────────────────────────────────────────────────┐
│                    Mixed-Precision Workflow                      │
├─────────────────────────────────────────────────────────────────┤
│  1. Profile    →  2. Analyze   →  3. Explore   →  4. Generate   │
│  ───────────      ──────────      ──────────      ───────────   │
│  occurrence       range_analyzer  pareto_explorer config_gen    │
│  scale_tracker    type_advisor    algorithm_prof                │
│  instrumented     memory_profiler                               │
└─────────────────────────────────────────────────────────────────┘
```

---

## Core Utilities

### occurrence.hpp - Operation Counting

**Purpose**: Count arithmetic operations (add, sub, mul, div, sqrt) during algorithm execution to estimate compute costs.

**What it does**: A simple struct that tracks operation counts for a specific number type. Used as a building block for energy estimation.

**Header**: `#include <universal/utility/occurrence.hpp>`

```cpp
#include <universal/utility/occurrence.hpp>
using namespace sw::universal;

// Create occurrence tracker
occurrence<float> ops;

// Update counts (typically done by instrumented types)
ops.add = 1000;
ops.mul = 2000;
ops.div = 100;

// Report counts
ops.report(std::cout);
```

**Key fields**:
- `load`, `store`: Memory operations
- `add`, `sub`, `mul`, `div`, `rem`: Arithmetic operations
- `sqrt`: Square root operations

**Use case**: Foundation for energy estimation when combined with energy cost models.

---

### scale_tracker.hpp - Exponent Distribution Tracking

**Purpose**: Track the distribution of exponents (scales) in computed values to understand dynamic range requirements.

**What it does**: Maintains a histogram of exponent values, counting how many values fall into each exponent bucket. Also tracks underflows and overflows.

**Header**: `#include <universal/utility/scale_tracker.hpp>`

```cpp
#include <universal/utility/scale_tracker.hpp>
using namespace sw::universal;

// Track scales from 2^-10 to 2^10
scaleTracker tracker(-10, 10);

// During computation, record scales
for (double value : computed_values) {
    int scale = ilogb(value);  // Extract exponent
    tracker.incr(scale);
}

// Report distribution
tracker.report(std::cout);
// Output shows histogram + underflow/overflow counts
```

**Key methods**:
- `incr(scale)`: Increment count for given exponent
- `clear()`: Reset counts
- `report(ostream)`: Print histogram

**Use case**: Determine the required exponent range for your algorithm. If most values cluster in a narrow range, you can use a type with fewer exponent bits.

---

### range_analyzer.hpp - Value Range Analysis

**Purpose**: Comprehensive analysis of value distributions to recommend appropriate precision.

**What it does**: Observes values during computation and collects statistics including min/max values, scale range, subnormal counts, and special values (inf, nan). Generates precision recommendations.

**Header**: `#include <universal/utility/range_analyzer.hpp>`

```cpp
#include <universal/utility/range_analyzer.hpp>
using namespace sw::universal;

// Create analyzer for your computation type
range_analyzer<double> analyzer;

// Observe values during computation
for (const auto& result : computation_results) {
    analyzer.observe(result);
}

// Get comprehensive report
analyzer.report(std::cout);

// Get precision recommendation
PrecisionRecommendation rec = analyzer.recommendPrecision();
std::cout << "Suggested type: " << rec.type_suggestion << "\n";
std::cout << "Min exponent bits: " << rec.min_exponent_bits << "\n";
std::cout << "Min fraction bits: " << rec.min_fraction_bits << "\n";
```

**Key methods**:
- `observe(value)`: Record a single value
- `observe(begin, end)`: Record a range of values
- `statistics()`: Get `RangeStatistics` struct
- `minScale()`, `maxScale()`: Get exponent range
- `dynamicRangeUtilization()`: Fraction of type's range actually used
- `recommendPrecision()`: Get `PrecisionRecommendation`

**Output includes**:
- Value classification (zeros, normals, denormals, inf, nan)
- Signed value distribution
- Value range (min, max, min absolute, max absolute)
- Scale span (number of decades)
- Type recommendation with rationale

**Use case**: Profile your algorithm's intermediate values to understand what precision is actually needed vs. what you're using.

---

### type_advisor.hpp - Type Recommendation

**Purpose**: Recommend optimal Universal number types based on range analysis and accuracy requirements.

**What it does**: Takes range analysis results and accuracy requirements, then scores and ranks all available Universal types (cfloat, posit, fixpnt, lns) to recommend the best match.

**Header**: `#include <universal/utility/type_advisor.hpp>`

```cpp
#include <universal/utility/type_advisor.hpp>
using namespace sw::universal;

// First analyze your data
range_analyzer<double> analyzer;
for (auto v : data) analyzer.observe(v);

// Create type advisor
TypeAdvisor advisor;

// Specify accuracy requirements
AccuracyRequirement accuracy;
accuracy.relative_error = 1e-4;    // 0.01% tolerance
accuracy.require_inf = false;       // Don't need infinity
accuracy.require_nan = false;       // Don't need NaN

// Get recommendations
auto recommendations = advisor.recommend(analyzer, accuracy);

// Print report
advisor.report(std::cout, analyzer, accuracy);

// Get best recommendation
TypeRecommendation best = advisor.bestType(analyzer, accuracy);
std::cout << "Best type: " << best.type.name << "\n";
std::cout << "Score: " << best.suitability_score << "%\n";
std::cout << "Energy factor: " << best.estimated_energy << "x\n";
```

**Key structures**:

`TypeCharacteristics`: Properties of each number type
- `name`, `family`, `total_bits`, `exponent_bits`, `fraction_bits`
- `max_value`, `min_positive`, `epsilon`
- `energy_per_fma` (picojoules estimate)
- `has_subnormals`, `has_inf`, `has_nan`

`TypeRecommendation`: Scored recommendation
- `type`: The `TypeCharacteristics`
- `suitability_score`: 0-100 (higher = better match)
- `meets_accuracy`, `meets_range`: Boolean checks
- `estimated_energy`: Relative to FP32
- `rationale`: Why this type was recommended

**Known types in database**:
- IEEE: half, bfloat16, float, double
- Posit: posit<8,0>, posit<16,1>, posit<32,2>, posit<64,3>
- Fixed-point: fixpnt<16,8>, fixpnt<32,16>
- LNS: lns<16,8>, lns<32,16>

**Use case**: When you know your accuracy requirements, get an objective ranking of which Universal types best fit your needs.

---

### memory_profiler.hpp - Memory Access Profiling

**Purpose**: Model memory access patterns and estimate memory-related energy costs.

**What it does**: Tracks memory reads and writes, estimates cache behavior based on working set size, and calculates energy costs across the memory hierarchy (L1, L2, L3, DRAM).

**Header**: `#include <universal/utility/memory_profiler.hpp>`

```cpp
#include <universal/utility/memory_profiler.hpp>
using namespace sw::universal;

// Create profiler with cache configuration
MemoryProfiler profiler(CacheConfig::intel_skylake());

// Record memory accesses
size_t matrix_bytes = M * N * sizeof(float);
profiler.recordRead(matrix_bytes, AccessPattern::Sequential);
profiler.recordWrite(result_bytes, AccessPattern::Sequential);
profiler.setWorkingSetSize(total_working_set);

// Get report
profiler.report(std::cout);

// Get energy estimate
double energy_pj = profiler.estimateEnergyPJ();
double energy_uj = profiler.estimateEnergyUJ();

// Check cache tier
MemoryTier tier = profiler.estimatePrimaryTier();
// Returns: L1_Cache, L2_Cache, L3_Cache, or DRAM
```

**Pre-built BLAS profiles**:
```cpp
// Profile common linear algebra operations
auto gemm_profile = profileGEMM(M, N, K, sizeof(float));
auto dot_profile = profileDotProduct(N, sizeof(float));
auto gemv_profile = profileGEMV(M, N, sizeof(float));
```

**Access patterns**:
- `AccessPattern::Sequential`: Linear traversal (best cache utilization)
- `AccessPattern::Strided`: Regular stride (e.g., column access)
- `AccessPattern::Random`: Irregular access (worst case)
- `AccessPattern::Reuse`: Repeated access to same data

**Cache configurations**:
- `CacheConfig::intel_skylake()`: 32KB L1, 256KB L2, 8MB L3
- `CacheConfig::arm_cortex_a76()`: 64KB L1, 512KB L2, 4MB L3
- `CacheConfig::arm_cortex_a55()`: 32KB L1, 128KB L2, 2MB L3

**Use case**: Estimate memory energy costs for your algorithm and understand if you're memory-bound or compute-bound.

---

### algorithm_profiler.hpp - Unified Algorithm Profiling

**Purpose**: Combine operation counting, memory profiling, and energy estimation into a single analysis framework.

**What it does**: Provides `AlgorithmProfile` structs that capture all dimensions of algorithm cost: operation counts, memory access, value ranges, and energy estimates.

**Header**: `#include <universal/utility/algorithm_profiler.hpp>`

```cpp
#include <universal/utility/algorithm_profiler.hpp>
using namespace sw::universal;

// Profile a GEMM operation
AlgorithmProfile profile = AlgorithmProfiler::profileGEMM(
    1024, 1024, 1024,       // M, N, K dimensions
    "float", 32,            // precision name, bit width
    CacheConfig::intel_skylake()
);

// Print comprehensive report
profile.report(std::cout);

// Access individual metrics
std::cout << "FMAs: " << profile.fmas << "\n";
std::cout << "Working set: " << profile.working_set_bytes << " bytes\n";
std::cout << "Arithmetic intensity: " << profile.ops_per_byte << " ops/byte\n";
std::cout << "Compute energy: " << profile.compute_energy_pj << " pJ\n";
std::cout << "Memory energy: " << profile.memory_energy_pj << " pJ\n";
std::cout << "Total energy: " << profile.total_energy_pj << " pJ\n";
```

**Pre-built profilers**:
```cpp
// Dense linear algebra
auto gemm = AlgorithmProfiler::profileGEMM(M, N, K, "float", 32);
auto gemv = AlgorithmProfiler::profileGEMV(M, N, "float", 32);
auto dot = AlgorithmProfiler::profileDotProduct(N, "float", 32);

// Deep learning
auto conv = AlgorithmProfiler::profileConv2D(
    224, 224,    // H, W
    3, 64,       // C_in, C_out
    3,           // kernel size
    "float", 32
);
```

**Comparing precisions**:
```cpp
auto fp32 = AlgorithmProfiler::profileGEMM(1024, 1024, 1024, "float", 32);
auto fp16 = AlgorithmProfiler::profileGEMM(1024, 1024, 1024, "half", 16);

PrecisionComparison cmp = AlgorithmProfiler::compare(fp32, fp16);
cmp.report(std::cout);
// Shows: ops ratio, memory ratio, energy savings percentage

// Compare multiple precisions
std::vector<AlgorithmProfile> profiles = {fp32, fp16, bf16, int8};
AlgorithmProfiler::compareMultiple(std::cout, profiles);
```

**Use case**: Get a complete picture of your algorithm's resource requirements across multiple precisions.

---

### pareto_explorer.hpp - Accuracy/Energy Trade-off Analysis

**Purpose**: Find Pareto-optimal precision configurations that balance accuracy, energy, and memory bandwidth.

**What it does**: Given a set of precision configurations with their accuracy/energy/bandwidth characteristics, computes the Pareto frontier—the set of configurations where no other configuration is better in all dimensions.

**Header**: `#include <universal/utility/pareto_explorer.hpp>`

```cpp
#include <universal/utility/pareto_explorer.hpp>
using namespace sw::universal;

// Create explorer (comes pre-loaded with standard types)
ParetoExplorer explorer;

// Add custom configuration if needed
explorer.addConfiguration("my_custom_type", 24, 1e-5, 0.8, 0.75);
// Parameters: name, bits, accuracy, energy_factor, bandwidth_factor

// Compute 2D Pareto frontier (accuracy vs energy)
ParetoResult result = explorer.computeFrontier();

// Compute 3D Pareto frontier (accuracy vs energy vs bandwidth)
ParetoResult result3d = explorer.computeFrontier3D();

// Query recommendations
PrecisionConfig best_for_accuracy = result.bestForAccuracy(1e-4);
PrecisionConfig best_for_energy = result.bestForEnergy(0.5);  // 50% of FP32 energy

// For specific algorithm characteristics
AlgorithmCharacteristics gemm = ParetoExplorer::profileGEMM(1024, 1024, 1024);
PrecisionConfig best = result.bestForAlgorithm(1e-4, gemm);

// Generate comprehensive report
explorer.report(std::cout);

// Visualize frontier (ASCII art)
explorer.plotFrontier(std::cout);

// Roofline-style analysis
explorer.rooflineAnalysis(std::cout, 100.0);  // 100 GB/s bandwidth
```

**Pre-loaded configurations**:
| Type | Bits | Accuracy | Energy | Bandwidth |
|------|------|----------|--------|-----------|
| FP64 | 64 | 2.2e-16 | 3.53x | 2.0x |
| FP32 | 32 | 1.2e-7 | 1.0x | 1.0x |
| FP16 | 16 | 9.8e-4 | 0.31x | 0.5x |
| BF16 | 16 | 7.8e-3 | 0.31x | 0.5x |
| posit<32,2> | 32 | 7.5e-9 | 0.5x | 1.0x |
| posit<16,1> | 16 | 2.4e-4 | 0.15x | 0.5x |
| INT8 | 8 | 3.9e-3 | 0.13x | 0.25x |

**Algorithm profiles**:
```cpp
// Common profiles for different algorithm types
auto dot_product = ParetoExplorer::profileDotProduct(1000000);
auto gemm_large = ParetoExplorer::profileGEMM(1024, 1024, 1024);
auto conv2d = ParetoExplorer::profileConv2D(224, 224, 3, 64, 3);
```

**Use case**: Systematically explore the precision design space and identify optimal configurations for your accuracy and resource requirements.

---

### precision_config_generator.hpp - Code Generation

**Purpose**: Generate C++ header files with type aliases for mixed-precision algorithm implementations.

**What it does**: Based on Pareto analysis results, generates a ready-to-use header file defining InputType, ComputeType, AccumulatorType, and OutputType for your algorithm.

**Header**: `#include <universal/utility/precision_config_generator.hpp>`

```cpp
#include <universal/utility/precision_config_generator.hpp>
using namespace sw::universal;

// Configure the generator
PrecisionConfigGenerator gen;
gen.setAlgorithm("GEMM");
gen.setAccuracyRequirement(1e-4);
gen.setEnergyBudget(0.5);  // 50% of FP32 energy
gen.setProblemSize("1024x1024");

// Generate C++ header
std::string header = gen.generateConfigHeader();
std::cout << header;

// Generate example usage code
std::string example = gen.generateExampleCode();

// Generate comparison report
std::string report = gen.generateComparisonReport();

// Print full analysis
gen.printAnalysis(std::cout);
```

**Generated header example**:
```cpp
// Auto-generated mixed-precision configuration
// Algorithm: GEMM
// Generated: 2024-02-07 15:30:00
//
// Requirements:
//   Accuracy:      1.0e-04
//   Energy budget: 50% of FP32
//
// Estimated energy: 35% of all-FP32
//
#pragma once

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

namespace gemm_config {

// Input precision - for loading data
using InputType = sw::universal::half;

// Compute precision - for arithmetic operations
using ComputeType = sw::universal::posit<16,1>;

// Accumulator precision - for reductions and dot products
using AccumulatorType = float;

// Output precision - for storing results
using OutputType = float;

// Configuration metadata
constexpr double target_accuracy = 1.0e-04;
constexpr double estimated_energy_factor = 0.35;

} // namespace gemm_config
```

**Use case**: Automate the generation of precision configuration headers for your mixed-precision implementations.

---

### instrumented.hpp - Operation Instrumentation

**Purpose**: Transparently wrap any number type to count all operations performed on it.

**What it does**: The `instrumented<T>` wrapper behaves exactly like type T but increments global atomic counters for every arithmetic operation. Thread-safe for parallel algorithms.

**Header**: `#include <universal/utility/instrumented.hpp>`

```cpp
#include <universal/utility/instrumented.hpp>
using namespace sw::universal;

// Reset counters before profiling
instrumented_stats::reset();

// Use instrumented type instead of the base type
using Real = instrumented<cfloat<32,8>>;
// Or: using Real = instrumented<posit<32,2>>;
// Or: using Real = instrumented<float>;

// Run your algorithm normally
Real a = 1.5, b = 2.5;
Real c = a + b;    // add counted
Real d = a * b;    // mul counted
Real e = c / d;    // div counted

// Get operation counts
instrumented_stats::report(std::cout);

// Or get programmatic access
uint64_t total_ops = instrumented_stats::totalArithmeticOps();
auto snapshot = instrumented_stats::snapshot<Real::value_type>();
```

**Tracked operations**:
- Memory: `loads`, `stores`
- Arithmetic: `adds`, `subs`, `muls`, `divs`, `rems`, `sqrts`
- Other: `comparisons`, `conversions`

**Key features**:
- Thread-safe using atomic counters
- Zero overhead when not using instrumented types
- Works with any Universal number type
- Transparent drop-in replacement

**Use case**: Profile your algorithm to count exact operation counts, then combine with energy models for accurate energy estimation.

---

## Workflow Guide

### Step 1: Profile Your Algorithm

Use `instrumented<T>` to count operations and `range_analyzer` to track value distributions:

```cpp
// Profile with instrumented types
instrumented_stats::reset();
using Real = instrumented<double>;
run_algorithm<Real>(data, result);
auto ops = instrumented_stats::snapshot<double>();

// Analyze value ranges
range_analyzer<double> analyzer;
for (auto& v : result) analyzer.observe(v);
analyzer.report(std::cout);
```

### Step 2: Get Type Recommendations

Use `TypeAdvisor` to get recommendations:

```cpp
TypeAdvisor advisor;
AccuracyRequirement acc(1e-4);
auto recommendations = advisor.recommend(analyzer, acc);

for (const auto& rec : recommendations) {
    std::cout << rec.type.name << ": " << rec.suitability_score << "%\n";
}
```

### Step 3: Explore Trade-offs

Use `ParetoExplorer` to understand the design space:

```cpp
ParetoExplorer explorer;
auto result = explorer.computeFrontier3D();

// Find best for your constraints
auto best = result.bestForConstraints(
    1e-4,   // accuracy requirement
    0.5,    // max 50% energy vs FP32
    0.5     // max 50% bandwidth vs FP32
);
```

### Step 4: Generate Configuration

Use `PrecisionConfigGenerator` to create headers:

```cpp
PrecisionConfigGenerator gen;
gen.setAlgorithm("MyAlgorithm");
gen.setAccuracyRequirement(1e-4);
gen.setEnergyBudget(0.5);

std::ofstream out("my_algorithm_config.hpp");
out << gen.generateConfigHeader();
```

### Step 5: Implement Mixed-Precision Algorithm

Use the generated configuration:

```cpp
#include "my_algorithm_config.hpp"

using namespace my_algorithm_config;

void my_algorithm(const std::vector<InputType>& input,
                  std::vector<OutputType>& output) {
    // Load data at input precision
    std::vector<ComputeType> work(input.begin(), input.end());

    // Compute with accumulator for reductions
    AccumulatorType sum = 0;
    for (const auto& v : work) {
        sum += AccumulatorType(v);
    }

    // Store at output precision
    output.push_back(OutputType(sum));
}
```

---

## Complete Example

```cpp
// complete_mixed_precision_workflow.cpp
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/instrumented.hpp>
#include <universal/utility/range_analyzer.hpp>
#include <universal/utility/type_advisor.hpp>
#include <universal/utility/algorithm_profiler.hpp>
#include <universal/utility/pareto_explorer.hpp>
#include <universal/utility/precision_config_generator.hpp>
#include <vector>
#include <iostream>

using namespace sw::universal;

// Your algorithm template
template<typename T>
T dot_product(const std::vector<T>& x, const std::vector<T>& y) {
    T result = 0;
    for (size_t i = 0; i < x.size(); ++i) {
        result += x[i] * y[i];
    }
    return result;
}

int main() {
    const size_t N = 10000;

    // Step 1: Profile with double precision
    std::vector<double> x(N), y(N);
    for (size_t i = 0; i < N; ++i) {
        x[i] = sin(i * 0.01);
        y[i] = cos(i * 0.01);
    }

    // Count operations
    instrumented_stats::reset();
    using InstrumentedReal = instrumented<double>;
    std::vector<InstrumentedReal> ix(x.begin(), x.end());
    std::vector<InstrumentedReal> iy(y.begin(), y.end());
    auto result = dot_product(ix, iy);

    std::cout << "=== Step 1: Operation Profile ===\n";
    instrumented_stats::report(std::cout);

    // Step 2: Analyze value ranges
    range_analyzer<double> analyzer;
    for (const auto& v : x) analyzer.observe(v);
    for (const auto& v : y) analyzer.observe(v);

    std::cout << "\n=== Step 2: Range Analysis ===\n";
    analyzer.report(std::cout);

    // Step 3: Get type recommendations
    TypeAdvisor advisor;
    AccuracyRequirement acc(1e-6);

    std::cout << "\n=== Step 3: Type Recommendations ===\n";
    advisor.report(std::cout, analyzer, acc);

    // Step 4: Explore trade-offs
    ParetoExplorer explorer;

    std::cout << "\n=== Step 4: Pareto Analysis ===\n";
    explorer.report(std::cout);

    // Step 5: Generate configuration
    PrecisionConfigGenerator gen;
    gen.setAlgorithm("DotProduct");
    gen.setAccuracyRequirement(1e-6);
    gen.setEnergyBudget(0.3);

    std::cout << "\n=== Step 5: Generated Configuration ===\n";
    std::cout << gen.generateConfigHeader();

    return 0;
}
```

---

## Summary

| Utility | Purpose | When to Use |
|---------|---------|-------------|
| `occurrence` | Count operations | Basic operation profiling |
| `scale_tracker` | Track exponent distribution | Understand dynamic range needs |
| `range_analyzer` | Comprehensive value analysis | Profile algorithm value ranges |
| `type_advisor` | Recommend number types | Choose types for accuracy needs |
| `memory_profiler` | Model memory costs | Analyze memory-bound algorithms |
| `algorithm_profiler` | Unified profiling | Complete algorithm analysis |
| `pareto_explorer` | Trade-off exploration | Find optimal precision configs |
| `precision_config_generator` | Generate code | Create mixed-precision headers |
| `instrumented` | Operation counting | Accurate operation profiling |

These utilities work together to provide a systematic approach to mixed-precision algorithm design, replacing guesswork with data-driven precision selection.
