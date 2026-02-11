# Phase 5: Block Format Benchmarks — mxblock vs nvblock vs zfpblock

## Context

Phases 1-4b implemented three block floating-point formats with fundamentally
different compression strategies. Phase 5 creates a benchmark suite that
compares them head-to-head on three axes: **quantization error** (RMSE/SNR),
**throughput** (quantize+dequantize ops/sec), and **compression ratio**
(bytes saved). This gives users concrete data for choosing the right format.

The three formats occupy different design points:
- **mxblock** — OCP MX v1.0: e8m0 power-of-2 scale, BlockSize=32, no compression
- **nvblock** — NVIDIA NVFP4: e4m3 fractional scale + tensor scale, BlockSize=16, no compression
- **zfpblock** — ZFP: transform-based lossy codec, block_size=4 (1D), actual compression

## Location

New subdirectory: `benchmark/accuracy/blockformat/`

Sits alongside existing `benchmark/accuracy/quantization/` and
`benchmark/accuracy/blas/`. Using the `accuracy` tree because the primary
value is accuracy-at-compression, and timing is secondary context.

## Files to Create (3 files)

### 1. `benchmark/accuracy/blockformat/CMakeLists.txt`

```cmake
file (GLOB SOURCES "./*.cpp")
compile_all("true" "accuracy" "Benchmarks/Accuracy/blockformat" "${SOURCES}")
```

### 2. `benchmark/accuracy/blockformat/quantization_error.cpp`

Compare quantization RMSE, SNR, and QSNR across all three formats on identical
input data (sinusoidal 1024 elements, linear ramp 1024 elements).
Generalize the quantization error metrics and add to include/sw/universal/quantization/

**Helpers**:
- `compute_rmse(src, dst, n)` — root mean square error
- `compute_snr(src, dst, n)` — signal-to-noise ratio in dB
- `generate_sinusoidal(float* dst, size_t n)` — sin(2pi*i/n)
- `generate_ramp(float* dst, size_t n)` — linear i/n

**Benchmark entries** (each operates on same input data):

| Config | API Call |
|--------|----------|
| mxfp4 (e2m1, BS=32) | `blk.quantize(src); blk.dequantize(dst);` — process 32 elements per block |
| mxfp8 (e4m3, BS=32) | same pattern |
| nvfp4 (e2m1, BS=16) | `blk.quantize(src, 1.0f); blk.dequantize(dst, 1.0f);` — 16 per block |
| zfp1f rate=4 | `zfparray1f arr(N, 4.0, src); arr.decompress(dst);` |
| zfp1f rate=8 | `zfparray1f arr(N, 8.0, src); arr.decompress(dst);` |
| zfp1f rate=16 | `zfparray1f arr(N, 16.0, src); arr.decompress(dst);` |

**Output** — aligned table per data pattern:
```
Sinusoidal data (N=1024):
Format              Rate(bpv)  Ratio   RMSE         SNR(dB)
mxfp4  (e2m1,32)     4.25      7.5x   ...          ...
mxfp8  (e4m3,32)     8.25      3.9x   ...          ...
nvfp4  (e2m1,16)     4.50      7.1x   ...          ...
zfp1f  rate=4         4.00      8.0x   ...          ...
zfp1f  rate=8         8.00      4.0x   ...          ...
zfp1f  rate=16       16.00      2.0x   ...          ...
```

### 3. `benchmark/accuracy/blockformat/throughput.cpp`

Measure quantize+dequantize throughput (wall-clock) using
`std::chrono::steady_clock`, following the `PerformanceRunner` pattern
from `universal/benchmark/performance_runner.hpp`.

**Design**: For each format, time `NR_OPS=100000` iterations of
quantize→dequantize on block-sized data. Report ops/sec.

**Entries**: mxfp4, mxfp8, nvfp4, zfp1f (rates 4/8/16)

**Output**:
```
Block quantize+dequantize throughput:
Format              Ops        Time(s)     Ops/sec
mxfp4  (e2m1,32)   100000     ...         ...
...
```

## Files to Modify (1 file)

### 4. `CMakeLists.txt` (root, ~line 1112)

Add `add_subdirectory("benchmark/accuracy/blockformat")` inside the
`if(UNIVERSAL_BUILD_BENCHMARK_ACCURACY)` block.

## Key References

| File | Purpose |
|------|---------|
| `include/sw/universal/number/mxfloat/mxblock_impl.hpp` | `mxblock::quantize()`, `dequantize()` |
| `include/sw/universal/number/nvblock/nvblock_impl.hpp` | `nvblock::quantize()`, `dequantize()` |
| `include/sw/universal/number/zfpblock/zfparray_impl.hpp` | `zfparray` compress/decompress |
| `include/sw/universal/benchmark/performance_runner.hpp` | `PerformanceRunner()` timing pattern |
| `benchmark/accuracy/quantization/CMakeLists.txt` | CMake pattern to follow |

## Compression Ratio Calculations

- **mxfp4**: 1B scale + 32×0.5B = 17B for 32 floats (128B) → 7.5x, 4.25 bpv
- **mxfp8**: 1B scale + 32×1B = 33B for 32 floats (128B) → 3.9x, 8.25 bpv
- **nvfp4**: 1B scale + 16×0.5B = 9B for 16 floats (64B) → 7.1x, 4.5 bpv
- **zfp1f rate=R**: `32/R` ratio, R bpv

## Implementation Order

1. `benchmark/accuracy/blockformat/CMakeLists.txt`
2. `benchmark/accuracy/blockformat/quantization_error.cpp`
3. `benchmark/accuracy/blockformat/throughput.cpp`
4. Root `CMakeLists.txt` — add `add_subdirectory`
5. Build and run

## Verification

```bash
pgrep -a make
cmake -B build_bench -DUNIVERSAL_BUILD_BENCHMARK_ACCURACY=ON .
make -j8 -C build_bench accuracy_quantization_error accuracy_throughput
./build_bench/benchmark/accuracy/blockformat/accuracy_quantization_error
./build_bench/benchmark/accuracy/blockformat/accuracy_throughput
```

Key checks:
- All three formats compile and produce tabular output
- nvfp4 has slightly lower RMSE than mxfp4 (fractional scale advantage)
- zfp RMSE decreases monotonically with rate
- zfp at rate=4 is competitive with mxfp4/nvfp4
- mxblock/nvblock faster than zfpblock (no transform overhead)
- No crashes, no warnings
- ONE build at a time, max `make -j8`
