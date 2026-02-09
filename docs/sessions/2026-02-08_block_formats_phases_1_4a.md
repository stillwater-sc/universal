# Development Session: Block Floating-Point Formats (Phases 1-4a)

**Date:** 2026-02-08
**Branch:** v3.95
**Focus:** Implement microfloat, mxblock, nvblock, and zfpblock number types
**Status:** Complete

## Session Overview

This session implemented four new number types for block floating-point
arithmetic, progressing from sub-byte element types through OCP Microscaling
blocks, NVIDIA NVFP4 blocks, and ZFP compressed floating-point blocks.
All four phases passed CI across Linux (GCC + Clang), macOS (x64 + ARM64),
and Windows (MSVC).

### Goals Achieved
- Phase 1: microfloat + e8m0 sub-byte element types
- Phase 2: mxblock OCP Microscaling block format
- Phase 3: nvblock NVIDIA NVFP4 two-level block scaling
- Phase 4a: zfpblock ZFP single-block transform codec
- CI pipeline fix: safe parallelism with `--parallel 2`
- MSVC portability fix for `__builtin_ctzll`

## Architecture Decisions

### Block Format Taxonomy

The four block formats represent distinct points in the compression design space:

| Format | Scale Type | Scale Granularity | Compression | Random Access |
|--------|-----------|-------------------|-------------|---------------|
| mxblock | e8m0 (power-of-2) | per block | None (fixed format) | Yes |
| nvblock | e4m3 (fractional) + tensor_scale | two-level | None (fixed format) | Yes |
| zfpblock | shared exponent | per block | Transform + bit-plane | Fixed-rate only |

**mxblock** follows the OCP MX v1.0 spec exactly: one e8m0 exponent shared
across BlockSize microfloat elements. Scale computation uses `floor(log2(amax))`.

**nvblock** differs from mxblock in two ways: (1) fractional e4m3 scale
instead of power-of-two e8m0, and (2) a second tensor-level scale factor.
This gives finer granularity and consistently lower quantization error.

**zfpblock** is fundamentally different — it's a transform-based codec, not
a quantization format. The five-stage pipeline (block-float, lifting, reorder,
negabinary, bit-plane) exploits spatial correlation for compression.

### ZFP Lifting Transform Lossiness

The ZFP lifting transform uses truncating integer right-shifts (`>>= 1`),
which lose the LSB. This means the forward/inverse pair is NOT exactly
invertible for arbitrary integers — round-trip error is ±1 per lifting step,
accumulating to ±2 per dimension.

For the full encode/decode pipeline, this is acceptable because:
1. The block-float quantization already introduces rounding
2. The bit-plane truncation (in lossy modes) dominates the error budget
3. In reversible mode, all bit planes are preserved, giving near-exact
   reconstruction (relative error < 10^-6 for float)

A true bit-exact reversible mode would require storing the LSBs lost during
lifting, as the reference ZFP library does in its `revcodec` path. This is
left for future work.

## Files Created

### Phase 4a: zfpblock (19 new files)

**Headers** (`include/sw/universal/number/zfpblock/`):
- `zfpblock_fwd.hpp` — Forward declarations, `zfp_mode` enum, 6 type aliases
- `exceptions.hpp` — Codec and mode exception classes
- `zfp_codec_traits.hpp` — Float/double → int32/int64 mapping, block sizes, buffer sizes
- `zfp_codec.hpp` — Core codec (~500 lines): bitstream, negabinary, block-float, lifting, bit-plane, full pipeline
- `zfpblock_impl.hpp` — Container class with compress/decompress and 4 modes
- `manipulators.hpp` — `type_tag()`, `to_binary()`, `to_hex()`
- `attributes.hpp` — `zfp_block_info()`, `zfp_compression_stats()`
- `zfpblock.hpp` — Umbrella header

**Traits**: `include/sw/universal/traits/zfpblock_traits.hpp`

**Tests** (`static/zfpblock/`):
- `CMakeLists.txt` — Glob-based build for api/codec/roundtrip/modes
- `api/api.cpp` — Full API demo with all aliases, modes, display
- `api/zfp_explained.md` — Educational document on ZFP for software engineers
- `codec/lifting.cpp` — Forward/inverse lifting with ±1 tolerance
- `codec/negabinary.cpp` — int2uint/uint2int round-trip for int32/int64
- `codec/bitplane.cpp` — Bit-plane encode/decode for 4 and 16 elements
- `roundtrip/roundtrip_1d.cpp` — 1D float/double compress/decompress
- `roundtrip/roundtrip_2d.cpp` — 2D float with reversible and fixed-rate
- `roundtrip/roundtrip_3d.cpp` — 3D float with reversible, fixed-rate, constant block
- `modes/fixed_rate.cpp` — Exact bit count verification, rate vs error, compression ratio

**CMake wiring** — 4 insertion points in root `CMakeLists.txt`:
- Option at line 166
- CI_LITE cascade at line 738
- STATICS cascade at line 819
- `add_subdirectory` at line 1005

## Key Bug Fixes

### MSVC `__builtin_ctzll` (CI failure)
MSVC does not provide GCC's `__builtin_ctzll` intrinsic. Added a portable
`zfp_ctzll()` wrapper that dispatches to `_BitScanForward64` on MSVC and
`__builtin_ctzll` on GCC/Clang.

### CI OOM Kills (`--parallel` without limit)
`cmake --build --parallel` (no number) spawns one compiler per core, equivalent
to `make -j$(nproc)`. On GitHub Actions runners with 7 GB RAM, this causes
OOM kills as each template-heavy TU consumes 500MB-1.5GB. Fixed by using
`--parallel 2`, which gives ~35% speedup while staying safely under memory limits.

### Bit-plane maxbits budget
The bit-plane codec test initially used `maxbits = N * precision`, which didn't
account for the group-testing signaling overhead. Fixed by using buffer-sized
maxbits for round-trip tests and ensuring the encoder respects maxbits strictly
(writes zero block when header exceeds budget).

## Test Results

All 8 zfpblock tests pass on all 5 CI platforms (Linux GCC, Linux Clang,
macOS x64, macOS ARM64, Windows MSVC). Zero warnings.

### Compression Quality (from test output)

**2D float fixed-rate** (4x4 sinusoidal data):
| Rate (bpv) | Bits | RMSE | Ratio |
|------------|------|------|-------|
| 4 | 64 | 6.5e-3 | 8x |
| 8 | 128 | 6.2e-4 | 4x |
| 16 | 256 | 2.5e-6 | 2x |
| 32 | 512 | 0 | 1x |

**3D float fixed-rate** (4x4x4 sinusoidal data):
| Rate (bpv) | Bits | RMSE | Ratio |
|------------|------|------|-------|
| 2 | 128 | 5.1e-2 | 16x |
| 4 | 256 | 2.0e-2 | 8x |
| 8 | 512 | 1.3e-3 | 4x |
| 16 | 1024 | 6.4e-6 | 2x |

## Commits

```
e50b34d7 Fix MSVC build: replace __builtin_ctzll with portable wrapper
e179c710 Add zfpblock ZFP compressed floating-point block codec (Phase 4a)
f0e7a2dd removing --parallel from build as we are running out of resources
711d8aa2 Add nvblock NVIDIA NVFP4 two-level block scaling format (Phase 3)
0eced4bc Optimize CI pipeline: tiered matrix, build caching, parallel tests
9b7c5bd2 Add mxblock OCP Microscaling block floating-point formats (Phase 2)
e7126a4f Add exhaustive sub/div tests for microfloat; close out Phase 1
cc0ba422 Add microfloat and e8m0 number types for OCP Microscaling (MX) block formats
5928be8d Add block floating-point formats implementation roadmap
```

## Next Steps

- **Phase 4b**: ZFP compressed array container with block cache
  (multi-block storage, random access via fixed-rate mode)
- **Phase 5**: Benchmarks comparing mxblock vs nvblock vs zfpblock
  for quantization error, throughput, and compression ratio
- **ZFP reversible mode**: Implement proper LSB-preserving codec path
  for bit-exact lossless round-trip
