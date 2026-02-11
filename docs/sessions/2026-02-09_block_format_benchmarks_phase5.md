# Development Session: Block Format Benchmarks & CI Cache Fix (Phases 4b, 5)

**Date:** 2026-02-09
**Branch:** v3.95
**Focus:** zfparray container, block format benchmark suite, sccache fix
**Status:** Complete

## Session Overview

This session completed Phase 4b (zfparray compressed array container) and
Phase 5 (head-to-head accuracy and throughput benchmarks for all three block
formats). A long-standing Windows CI sccache misconfiguration was also
diagnosed and fixed, bringing cache hit rate from 0% to 100%.

### Goals Achieved
- Phase 4b: zfparray multi-block compressed array container
- Phase 5: quantization error and throughput benchmarks
- Shared `error_metrics.hpp` header with RMSE, SNR, QSNR
- Windows sccache fix: `SCCACHE_GHA_ENABLED=true`

## Commits

| Hash | Description |
|------|-------------|
| `8962daaf` | Add zfparray compressed array container and fix Windows CI sccache |
| `808d48cc` | Add block format accuracy benchmarks and shared quantization error metrics (Phase 5) |
| `f88a13c4` | Fix Windows sccache: enable GHA cache backend and bump action to v0.0.9 |
| `6d440465` | Add legend and comparison examples to block format quantization benchmark |

## Phase 4b: zfparray

`zfparray<Real, Dim>` wraps the single-block `zfpblock` codec into a
multi-block compressed array with random access. All blocks use fixed-rate
mode so block N starts at a computable byte offset.

Key design decisions:
- **Single-block write-back cache** — Decompresses one block at a time into
  a small buffer. Sequential access is efficient; random access triggers
  cache eviction. The cache is flushed in the destructor and copy operations.
- **Partial last block** — When `n` is not a multiple of `BLOCK_SIZE` (4 for
  1D), the last block is zero-padded during compression and only valid
  elements are copied during decompression.
- **Fixed-rate only** — Required for O(1) random access. Variable-rate modes
  would need an index table.

### Files Created

```
include/sw/universal/number/zfpblock/zfparray.hpp        - umbrella header
include/sw/universal/number/zfpblock/zfparray_fwd.hpp    - forward decls + aliases
include/sw/universal/number/zfpblock/zfparray_impl.hpp   - implementation
static/zfpblock/array/array_api.cpp                      - API test
static/zfpblock/array/array_cache.cpp                    - cache behavior test
static/zfpblock/array/array_copy_move.cpp                - copy/move semantics test
static/zfpblock/array/array_roundtrip.cpp                - compress/decompress roundtrip
```

## Phase 5: Block Format Benchmarks

Benchmark suite at `benchmark/accuracy/blockformat/` comparing mxblock,
nvblock, and zfparray on two axes:

### Quantization Error (quantization_error.cpp)

Runs all six format configurations against sinusoidal and linear ramp data
(N=1024), measuring RMSE, SNR (dB), and QSNR (dB).

Representative results (sinusoidal data):

```
Format                  Rate    Ratio          RMSE     SNR(dB)    QSNR(dB)
---------------------------------------------------------------------------
mxfp4  (e2m1,32)        4.25    7.5x    1.1292e-01       15.93       15.93
mxfp8  (e4m3,32)        8.25    3.9x    4.3720e-02       24.18       24.18
nvfp4  (e2m1,16)        4.50    7.1x    3.5102e-02       26.08       26.08
zfp1f  rate=4           4.00    8.0x    9.6531e-02       17.30       17.30
zfp1f  rate=8           8.00    4.0x    1.5249e-03       53.32       53.32
zfp1f  rate=16         16.00    2.0x    2.6959e-06      108.38      108.38
```

Key findings:
- **nvfp4 beats mxfp4** at comparable rates: 3x lower RMSE, +10 dB SNR,
  for only 0.25 extra bpv. The fractional e4m3 scale fits data more tightly.
- **zfp scales dramatically with rate**: at 8 bpv it achieves 53 dB vs
  mxfp8's 24 dB. The decorrelating transform concentrates energy into fewer
  coefficients.
- **SNR vs QSNR**: Identical for zero-mean signals (sinusoid), but QSNR is
  lower for DC-offset signals (ramp) because variance < E[x^2].

### Throughput (throughput.cpp)

Measures 100K iterations of quantize+dequantize per format configuration.

### Shared Error Metrics (error_metrics.hpp)

Created `include/sw/universal/quantization/error_metrics.hpp` providing
two API styles:

1. **Pre-quantized pair** — `rmse(src, dst)`, `snr(src, dst)`, `qsnr(src, dst)`
   taking `const std::vector<Real>&`. For block formats where quantization is
   external.

2. **Scalar-type quantization** — `rmse<NumberType>(data)`,
   `snr<NumberType>(data)`, `qsnr<NumberType>(data)` taking
   `const std::vector<double>&`. Mirrors the existing `qsnr<Scalar>()` in
   `qsnr.hpp`.

Design note: The original implementation used raw `float*` + `size_t n`
parameters. Refactored to `const std::vector<Real>&` to eliminate C-style
pointer/size bugs. Raw `.data()` calls remain only at the boundary where
block format APIs require pointers (quantize/dequantize/compress/decompress).

### QSNR Formula Reconciliation

Three different QSNR implementations existed in the codebase:

| Location | Formula | Log | Signal Power |
|----------|---------|-----|-------------|
| `quantization/qsnr.hpp` | `10 * log10(σ² / noise)` | log10 | variance |
| `mpdot.cpp` (local) | `-10 * ln(E[(Q-x)²] / E[x²])` | natural log | E[x²] |
| `error_metrics.hpp` (new) | `10 * log10(σ² / noise)` | log10 | variance |

The new `error_metrics.hpp` follows the canonical `qsnr.hpp` formula.

## Windows sccache Fix

### Root Cause

The `mozilla-actions/sccache-action@v0.0.7` only installs the sccache binary
and puts it on PATH. It does **not** set `SCCACHE_GHA_ENABLED`. Without this
env var, sccache defaults to local disk storage at
`C:\Users\runneradmin\AppData\Local\Mozilla\sccache\cache`, which is
ephemeral — destroyed when the GitHub Actions runner is recycled. Every run
was a cold cache with 0% hit rate.

### Fix

```yaml
# Before (broken)
uses: mozilla-actions/sccache-action@v0.0.7
run: |
  echo "CMAKE_C_COMPILER_LAUNCHER=sccache" >> $env:GITHUB_ENV
  echo "CMAKE_CXX_COMPILER_LAUNCHER=sccache" >> $env:GITHUB_ENV

# After (working)
uses: mozilla-actions/sccache-action@v0.0.9
run: |
  echo "SCCACHE_GHA_ENABLED=true" >> $env:GITHUB_ENV
  echo "CMAKE_C_COMPILER_LAUNCHER=sccache" >> $env:GITHUB_ENV
  echo "CMAKE_CXX_COMPILER_LAUNCHER=sccache" >> $env:GITHUB_ENV
```

### Verification

Cache location changed from `Local disk` to `ghac` (GitHub Actions Cache).

| Run | Hits | Misses | Hit Rate | Avg Compile |
|-----|------|--------|----------|-------------|
| Cold cache (first) | 0 | 386 | 0% | 6.133s |
| Warm cache (second) | 386 | 0 | 100% | 0.207s |

All platforms now have working compiler caches:

| Platform | Tool | Hit Rate |
|----------|------|----------|
| Linux GCC | ccache | 99.6% (737/740) |
| Linux Clang | ccache | 99.6% (737/740) |
| Windows MSVC | sccache | 100% (386/386) |

## Files Created

```
benchmark/accuracy/blockformat/CMakeLists.txt
benchmark/accuracy/blockformat/quantization_error.cpp
benchmark/accuracy/blockformat/throughput.cpp
include/sw/universal/quantization/error_metrics.hpp
```

## Files Modified

```
CMakeLists.txt                      - add_subdirectory for blockformat benchmark
.github/workflows/cmake.yml        - sccache v0.0.9 + SCCACHE_GHA_ENABLED
```
