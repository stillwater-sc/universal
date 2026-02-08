# Mixed-Precision Algorithm Design Methodology

This document describes the systematic methodology for designing energy-efficient mixed-precision algorithms using the Universal Numbers Library SDK.

## Overview

Mixed-precision computing uses different numerical precisions for different stages of computation to optimize energy efficiency while maintaining required accuracy. This methodology provides a data-driven approach to precision selection.

## SDK Components

### 1. Energy Cost Models
Location: `include/sw/universal/energy/`

Energy models for various architectures:
- **Generic 45nm**: Baseline reference model
- **Intel Skylake**: Desktop/server (14nm)
- **AMD Zen 2/3/4**: Desktop/server (7nm/5nm)
- **Apple M1/M2/M3**: Mobile/desktop (5nm/3nm)
- **ARM Cortex-A**: Mobile (7nm)

```cpp
#include <universal/energy/energy.hpp>
using namespace sw::universal::energy;

// Auto-detect architecture
const auto& model = getDefaultModel();

// Or select specific model
const auto& skylake = getModel(Architecture::IntelSkylake);
const auto& m2 = getModel(Architecture::AppleM2);
```

### 2. Pareto Explorer
Location: `include/sw/universal/utility/pareto_explorer.hpp`

3D Pareto analysis optimizing accuracy vs energy vs bandwidth:

```cpp
#include <universal/utility/pareto_explorer.hpp>

ParetoExplorer explorer;
explorer.report(std::cout);           // Full 3D analysis
explorer.plotFrontier(std::cout);     // 2D visualization
explorer.rooflineAnalysis(std::cout, 100.0);  // Algorithm guidance
```

### 3. Algorithm Profiler
Location: `include/sw/universal/utility/algorithm_profiler.hpp`

Profile compute, memory, and energy characteristics:

```cpp
#include <universal/utility/algorithm_profiler.hpp>

auto profile = AlgorithmProfiler::profileGEMM(1024, 1024, 1024, "FP32", 32);
profile.report(std::cout);
```

### 4. Autotuner
Location: `include/sw/universal/utility/autotuner.hpp`

Automatic precision selection through empirical testing:

```cpp
#include <universal/utility/autotuner.hpp>

Autotuner tuner;
tuner.setAccuracyRequirement(1e-4);
tuner.setEnergyBudget(0.5);  // 50% of FP32

auto result = tuner.tuneUnaryFunction("sqrt",
    [](auto x) { return sqrt(x); },
    test_inputs);
result.report(std::cout);
```

### 5. PGO Framework
Location: `include/sw/universal/utility/pgo_energy.hpp`

Profile-guided optimization with RAPL measurements:

```cpp
#include <universal/utility/pgo_energy.hpp>

PGOCalibrator calibrator;
auto stats = calibrator.calibrate("my_kernel", kernel_fn, ops_count,
    Operation::FloatFMA, BitWidth::bits_32, iterations);
auto coefficients = calibrator.learnCoefficients(stats);
```

### 6. Mixed-Precision BLAS
Location: `include/sw/blas/mixed_precision.hpp`

Energy-efficient linear algebra operations:

```cpp
#include <blas/mixed_precision.hpp>

// FP16 input, FP16 compute, FP32 accumulator
using Config = MixedPrecisionConfig<half, half, float, half>;

auto result = mp_dot<Config>(x, y, &stats);
mp_gemm<Config>(m, n, k, alpha, A, B, beta, C, &stats);
```

## Methodology Steps

### Step 1: Characterize Algorithm

Identify the algorithm's computational characteristics:

1. **Arithmetic Intensity** (ops/byte): Determines compute-bound vs memory-bound
   - AI < 10: Memory-bound → focus on bandwidth reduction
   - AI > 10: Compute-bound → focus on energy reduction

2. **Working Set Size**: Determines cache behavior
   - < L1: Hot data stays in registers/L1
   - L1-L3: Data moves through cache hierarchy
   - > L3: Memory-bound, bandwidth critical

3. **Operation Mix**: FMA-heavy vs add-heavy vs multiply-heavy

```cpp
auto profile = AlgorithmProfiler::profileGEMM(m, n, k, "FP32", 32);
// Analyze: profile.arithmetic_intensity, profile.working_set_bytes
```

### Step 2: Define Accuracy Requirements

Establish accuracy bounds based on application domain:

| Domain | Typical Accuracy | Notes |
|--------|-----------------|-------|
| ML Inference | 1e-2 to 1e-3 | Often quantized to INT8 |
| Real-time Graphics | 1e-3 to 1e-4 | Visual quality matters |
| Signal Processing | 1e-4 to 1e-6 | SNR requirements |
| CAD/Engineering | 1e-6 to 1e-10 | Safety-critical |
| Scientific Computing | 1e-10 to 1e-15 | Numerical stability |

### Step 3: Analyze Pareto Frontier

Use the Pareto explorer to find optimal configurations:

```cpp
ParetoExplorer explorer;

// Get recommendation for specific accuracy
auto config = explorer.bestForAccuracy(accuracy_requirement);

// Consider bandwidth constraints for memory-bound algorithms
auto algo = ParetoExplorer::profileGEMM(m, n, k, bytes_per_element);
auto config = explorer.bestForAlgorithm(accuracy, algo);
```

### Step 4: Autotune for Specific Kernel

Run autotuning to measure actual accuracy:

```cpp
Autotuner tuner;
tuner.setAccuracyRequirement(target_accuracy);
tuner.setEnergyBudget(energy_budget);
tuner.enableTiming(true);

auto result = tuner.tuneUnaryFunction("kernel_name", kernel, inputs);
// result.recommended contains the best configuration
```

### Step 5: Calibrate with Hardware Measurements

Use RAPL to calibrate energy models:

```cpp
// Check RAPL availability
if (energy::RaplReader::isAvailable()) {
    PGOCalibrator calibrator;
    auto stats = calibrator.calibrate("kernel", fn, ops, op_type, width);
    auto coefficients = calibrator.learnCoefficients(stats);

    // Apply calibration to optimizer
    PGOOptimizer optimizer;
    optimizer.setCalibration(coefficients);
}
```

### Step 6: Implement Mixed-Precision

Apply the selected configuration:

```cpp
// Define custom mixed-precision config
using MLConfig = MixedPrecisionConfig<
    half,           // Input: FP16
    half,           // Compute: FP16
    float,          // Accumulator: FP32
    half            // Output: FP16
>;

// Use in BLAS operations
MixedPrecisionStats stats;
mp_gemm<MLConfig>(m, n, k, alpha, A, B, beta, C, &stats);

// Verify energy savings
auto comparison = compareMixedPrecisionEnergy<MLConfig>(stats);
comparison.report(std::cout);
```

## Best Practices

### 1. Use Higher-Precision Accumulators

Reductions (dot products, sums) accumulate rounding errors. Use higher precision:

```cpp
// BAD: FP16 accumulator loses precision
using Bad = MixedPrecisionConfig<half, half, half, half>;

// GOOD: FP32 accumulator maintains accuracy
using Good = MixedPrecisionConfig<half, half, float, half>;
```

### 2. Consider Memory Bandwidth

For memory-bound algorithms, reducing precision directly reduces bandwidth:

- FP64 → FP32: 2x bandwidth reduction
- FP32 → FP16: 2x bandwidth reduction
- FP16 → INT8: 2x bandwidth reduction

### 3. Match Precision to Hardware

Modern GPUs have specialized units:
- Tensor cores: INT8, FP16, BF16, TF32
- SIMD: FP32, FP64

Choose precisions that map to hardware efficiently.

### 4. Validate Accuracy Empirically

Always verify accuracy with representative test data:

```cpp
auto test = testDotProductAccuracy<Config>(x_double, y_double);
if (test.mixed_relative_error > requirement) {
    // Increase precision
}
```

### 5. Profile Before Optimizing

Understand where time/energy is spent before optimizing:

```cpp
AlgorithmProfile profile = AlgorithmProfiler::profileGEMM(...);
profile.report(std::cout);
// Optimize the dominant cost factor
```

## Common Mixed-Precision Configurations

### ML Inference
```cpp
// Weights: INT8, Activations: INT8, Accumulator: INT32
using MLInference = MixedPrecisionConfig<int8_t, int8_t, int32_t, int8_t>;
```

### ML Training
```cpp
// FP16 compute with FP32 master weights
using MLTraining = MixedPrecisionConfig<half, half, float, float>;
```

### Scientific Computing
```cpp
// FP32 compute with FP64 accumulator for dot products
using Scientific = MixedPrecisionConfig<float, float, double, float>;
```

### Graphics Shaders
```cpp
// FP16 throughout for bandwidth
using Graphics = MixedPrecisionConfig<half, half, half, half>;
```

## Energy Savings Summary

Typical energy savings with mixed-precision:

| Configuration | Energy vs FP32 | Accuracy Loss |
|---------------|----------------|---------------|
| FP16+FP32acc | 45-55% savings | ~1e-4 |
| INT8+INT32acc | 70-80% savings | ~1e-2 |
| BF16+FP32acc | 45-50% savings | ~1e-3 |
| posit16+32acc | 45-55% savings | ~1e-4 |

## Conclusion

The Mixed-Precision SDK provides a complete toolkit for systematic precision optimization:

1. **Analyze**: Use profiler and Pareto explorer to understand trade-offs
2. **Select**: Use autotuner to find best configuration empirically
3. **Validate**: Verify accuracy meets requirements
4. **Calibrate**: Use RAPL to refine energy estimates
5. **Implement**: Apply mixed-precision to production code
6. **Monitor**: Track energy savings in deployment

By following this methodology, algorithms can achieve 50-80% energy savings while maintaining required accuracy for their application domain.
