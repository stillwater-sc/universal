# Plan: Mixed-Precision Attention Head with KV Cache

## Context

The systems paper (Section 4.1) argues that LLM deployment hits a hard memory wall: LLaMA-70B at FP32 = 280 GB, exceeding any single GPU. This application provides a concrete, runnable demonstration: a scaled dot-product attention head with KV cache, parameterized over Universal number types, wrapped in energy/memory/latency/accuracy measurement. It shows exactly how precision choice determines whether the KV cache fits in memory, and what the accuracy tradeoff looks like.

## Files to Create

### 1. `applications/mixed-precision/attention/CMakeLists.txt`

```cmake
file (GLOB SOURCES "./*.cpp")

compile_all("true" "attention" "Applications/Mixed Precision/Attention" "${SOURCES}")
```

### 2. `applications/mixed-precision/attention/attention.cpp`

Single self-contained file (~550 lines), following the `robotics_pipeline.cpp` pattern.

## File to Modify

### 3. `CMakeLists.txt` (top-level, line ~1199)

Add after the existing robotics line (1198):
```cmake
add_subdirectory("applications/mixed-precision/attention")
```

## Structure of `attention.cpp`

### Section 1: Includes (~40 lines)

```
<iostream>, <iomanip>, <vector>, <cmath>, <random>, <chrono>, <string>, <algorithm>
<universal/utility/directives.hpp>
<universal/number/cfloat/cfloat.hpp>
<universal/number/posit/posit.hpp>
<universal/energy/energy.hpp>
<blas/mixed_precision.hpp>
```

Using: `sw::universal`, `sw::blas`

### Section 2: Constants (~20 lines)

Attention geometry:
- `D_MODEL = 128` (head dimension, d_k = d_v)
- `SEQ_LEN = 64` (prefill context)
- `N_TOKENS = 32` (autoregressive generation steps)

LLaMA-70B projection:
- `LLAMA_LAYERS = 80`, `LLAMA_HEADS = 64`, `LLAMA_DK = 128`
- `LLAMA_CONTEXT = 2048`, `GPU_HBM_GB = 80.0`

### Section 3: AttentionHead Template Class (~150 lines)

```cpp
template<typename QKType, typename VType, typename AccumType>
class AttentionHead
```

**Members:** `d_k`, `k_cache` (vector-of-vectors of QKType), `v_cache` (vector-of-vectors of VType), `MixedPrecisionStats stats`

**Methods:**
- `appendKV(k_row, v_row)` — push one row to each cache, track loads/stores
- `forward(q)` → `vector<VType>` — the full attention computation:
  1. QK^T: dot(q, k_cache[t]) for each cached token, accumulated at AccumType
  2. Scale by 1/sqrt(d_k)
  3. Softmax at AccumType (subtract max, exp, normalize) — uses `using std::exp;` ADL pattern
  4. Weighted V sum: output[j] += weight[t] * v_cache[t][j], accumulated at AccumType
  5. Cast output back to VType
  6. Track all operations in `stats`
- `kvCacheBytes()` — returns `cached_tokens * d_k * (sizeof(QKType) + sizeof(VType))`
- `cachedTokens()`, `getStats()`, `resetStats()`

**Design decisions:**
- Vector-of-vectors (not `matrix<T>`) because KV cache grows row-by-row
- Softmax at AccumType for numerical stability (matches real practice: NVIDIA FP8 uses FP32 softmax)
- `using std::exp;` before `exp(x)` enables ADL for both Universal types and native float/double

### Section 4: Accuracy Measurement (~40 lines)

```cpp
struct AccuracyResult { double max_abs_error; double rmse; };
```

`computeReferenceOutputs(...)` — runs attention at full `double` precision, returns all N_TOKENS output vectors.

### Section 5: Energy Estimation (~30 lines)

`estimateAttentionEnergy(stats, compute_bw, accum_bw, mem_bw)` — follows `robotics_pipeline.cpp` pattern using `energy::getDefaultModel()`, maps operation counts to picojoules.

`toBitWidth(bytes)` helper — maps sizeof to `energy::BitWidth` enum.

### Section 6: Benchmark Runner (~80 lines)

```cpp
struct AttentionBenchmarkResult {
    string config_name;
    size_t element_bytes;      // sizeof KV element
    size_t kv_cache_bytes;     // total after N_TOKENS steps
    double energy_pj;          // estimated energy
    double latency_us;         // wall-clock
    double max_abs_error;      // vs double reference
    double rmse;               // vs double reference
};
```

```cpp
template<typename QKType, typename VType, typename AccumType>
AttentionBenchmarkResult runBenchmark(name, q_data, k_data, v_data, ref_outputs)
```

1. Create AttentionHead, prefill with SEQ_LEN tokens
2. Time N_TOKENS forward passes with `chrono::high_resolution_clock`
3. Collect outputs, compute accuracy vs reference
4. Compute energy from stats
5. Return result

### Section 7: LLaMA-70B Projection (~50 lines)

`printScalingProjection()` — computes KV cache memory per token and at 2048 context for each precision. Also shows model weights + KV cache combined fit analysis.

Output table:
```
Precision     Bytes/elem  KV/token (KB)  KV@2048 (GB)  Fits 80GB?
double             8         2560.00         5.24         YES (KV only)
float              4         1280.00         2.62         YES
fp16               2          640.00         1.31         YES
...
```

Then: "Model weights at FP32: 280 GB — does NOT fit. At INT4: 35 GB + FP16 KV cache 1.31 GB = 36.31 GB — fits."

### Section 8: main() (~100 lines)

1. Print banner
2. Generate deterministic test data: `mt19937(42)`, uniform[-1,1] for Q,K,V
3. Compute double-precision reference outputs
4. Run type sweep (7 configurations):

| Config | QKType | VType | AccumType |
|--------|--------|-------|-----------|
| double | double | double | double |
| float | float | float | double |
| fp16 | half | half | float |
| bf16 | bfloat_t | bfloat_t | float |
| posit<16,1> | posit<16,1> | posit<16,1> | posit<32,2> |
| fp8e4m3 | fp8e4m3 | fp8e4m3 | float |
| posit<8,0> | posit<8,0> | posit<8,0> | posit<32,2> |

5. Print comparison table:
```
Config        KV Cache   Energy(uJ)  Latency(us)  Max Error    RMSE
```
6. Print energy breakdown for FP16 vs FP32
7. Call `printScalingProjection()`
8. Print key takeaways
9. Exception handlers: `catch (const char*)`, `catch (const std::exception&)`

## Reusable Infrastructure

| Component | File | What we use |
|-----------|------|-------------|
| MixedPrecisionStats | `include/sw/blas/mixed_precision.hpp` | Operation counting struct |
| EnergyEstimator | `include/sw/universal/energy/energy.hpp` | getDefaultModel(), Operation, BitWidth, MemoryLevel |
| Type aliases | `include/sw/universal/number/cfloat/cfloat.hpp:87,107,117` | `half`, `bfloat_t`, `fp8e4m3` |
| compile_all macro | `CMakeLists.txt:509-529` | Build wiring |

## Verification

1. Build: `cmake -DUNIVERSAL_BUILD_APPLICATIONS=ON .. && make -j4 attention_attention`
2. Run: `./attention_attention`
3. Check: all 7 configs produce output, double config shows 0 error, 8-bit configs show visible accuracy loss
4. Check: LLaMA projection table shows correct byte counts (80×64×128×2×sizeof = expected)
5. Check: energy estimates show ~4× reduction for 16-bit vs 32-bit, ~8× for 8-bit
