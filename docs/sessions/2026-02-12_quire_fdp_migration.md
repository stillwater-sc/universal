# Session: Port Quire, FDP, and Fused BLAS to New Posit

**Date:** 2026-02-12 (multi-session effort spanning Feb 11-12)
**Branch:** `v3.98`
**Build directories:** `build/` (gcc, BUILD_ALL), `build_clang/` (clang, BUILD_ALL)

## Objective

Complete the migration of quire, FDP (fused dot product), and fused BLAS features from posit1 (2-param `posit<nbits, es>`) to the new posit (3-param `posit<nbits, es, bt>`). This was the last major feature gap blocking full adoption of the new posit as the default. The new posit uses `blocktriple<>` and `blocksignificand<>` internally, while quire internals use `internal::value<>` and `bitblock<>` — bridge functions were needed at the interface points.

## Architecture Decisions

### Bridge Function Approach
Rather than rewriting the quire's internals (which are complex and well-tested), bridge functions were added at the posit-quire boundary:
- `convert(internal::value<fbits>&, posit<nbits,es,bt>&)` — quire output path
- `posit_to_value(posit<nbits,es,bt>&)` — quire input path
- `posit_normalize_to(posit<nbits,es,bt>&)` — quire convert_to path

This keeps the quire's `value<>`/`bitblock<>` internals unchanged while making it work with the new `blocktriple<>`-based posit.

### posit-agnostic BLAS Solver Headers
The fused solver headers (`posit_fused_backsub.hpp`, `posit_fused_forwsub.hpp`) were made posit-agnostic by removing their posit include. This allows both posit1 consumers (e.g., posito-based IR applications) and new posit consumers to use them without template redeclaration conflicts.

### posito Preserved Intentionally
posito (`include/sw/universal/number/posito/`) is a uniquely named copy of the original posit, kept as a reference implementation with its `value<>`-based engine for comparison during the posit2 transformation. Files using posito remain on posit1 includes.

## Commits

| Hash | Description |
|------|-------------|
| `27f1bf3b` | Port quire, FDP, and fused BLAS to new posit |
| `e48080e2` | Migrate remaining posit1 consumer files to new posit |
| `c5a52e69` | Rewrite education files to use blocktriple instead of internal::value |

## Changes Made

### 1. Bridge Functions (`posit_impl.hpp`)

Added `#include <universal/internal/value/value.hpp>` and three bridge functions:

- **`convert(value<fbits>&, posit<nbits,es,bt>&)`** — converts `internal::value<>` (quire output) to new posit by copying bitblock fraction into blocksignificand and calling `convert_()`
- **`posit_to_value(posit<nbits,es,bt>&)`** — extracts `internal::value<fbits>` from new posit by reading blockbinary fraction and converting to bitblock
- **`posit_normalize_to(posit<nbits,es,bt>&)`** — wraps the quire's `to_value()` output into posit via the convert bridge

### 2. Quire Port (`posit/quire.hpp`)

New file copied from `posit1/quire.hpp` with modifications:
- All posit-facing methods templated on `bt` to accept `posit<nbits, es, bt>` for any block type
- `quire_mul()` and `quire_add()` bridge `blockbinary<>` fractions to `bitblock<>` for the `module_multiply()`/`module_add()` calls
- Core quire internals (`operator+=(value<>)`, accumulator logic) unchanged

### 3. FDP Port (`posit/fdp.hpp`)

New file copied from `posit1/fdp.hpp`:
- Changed `enable_if_posit1` to `enable_if_posit`
- Updated traits include path

### 4. TwoSum (`posit/twoSum.hpp`)

New file adapted from `posit1/twoSum.hpp` with bt template parameter.

### 5. Consumer File Migration (23 files)

All migrated from `#include <posit1/posit1.hpp>` to `#include <posit/posit.hpp>`:

| Category | Files |
|----------|-------|
| Quire tests | `static/quire/api/api.cpp`, `static/quire/arithmetic/arithmetic.cpp` |
| BLAS reproducibility | `norms.cpp`, `l1_fused_dot.cpp`, `lu.cpp`, `l3_fused_mm.cpp` |
| Applications | `error_vs_cost.cpp`, `residual.cpp` |
| Linear algebra | `vector_ops.cpp`, `matrix_ops.cpp` |
| Education | `quires.cpp`, `signalling_nar.cpp`, `exceptions.cpp` |
| Benchmarks | 5x `gemm.cpp` (accuracy, range, energy, performance, reproducibility) |
| Tools | `propp.cpp`, `propq.cpp` |

### 6. Template Conflict Resolution

IR application files (`luir.cpp`, `roundAndReplace.cpp`, `scaleAndRound.cpp`, `twoSidedScaleAndRound.cpp`) use posito which depends on posit1 internals. These were reverted to posit1 includes. `luir.cpp` also uses `posit<64, 11>` which exceeds the new posit's `es < 10` assertion.

### 7. Education File Rewrite (blocktriple)

**`values.cpp`:**
- `internal::value<fbits>` → `blocktriple<fbits, BlockTripleOperator::REP, uint8_t>`
- `ValidateValue()` → `ValidateBlocktriple()`
- `round_to<N>()` demo replaced with precision-across-sizes demo (48→24→16→12→8→4→2→1 fraction bits)
- Removed `ALGORITHM_TRACE_CONVERSION` define

**`extract.cpp`:**
- Removed `extract_fp_components()`, `extract_23b_fraction()`, `extract_52b_fraction()`, `internal::value<>`, `internal::bitblock<>` dependencies
- Uses `blocktriple<24, REP>` (float) and `blocktriple<53, REP>` (double) for IEEE-754 triple display
- Uses direct posit assignment (`p = f`) for conversion — the posit's `convert_ieee754()` uses frexp directly, bypassing blocktriple's known `round()` off-by-one for same-precision sources
- Replaced obscure alternating-bit subnormal test patterns with meaningful normal values

## Key Technical Findings

### blocktriple `round()` Off-by-One
When `blocktriple<N, REP>` is assigned from a source type whose mantissa bits exactly match N (e.g., `blocktriple<23>` from `float`, `blocktriple<52>` from `double`), the `round()` function shifts the hidden bit below the radix position. This causes `to_native<double>()` to return half the correct value and `convert(blocktriple, posit)` to produce incorrect posit values.

The posit's own `convert_ieee754()` (line 1182) already works around this by using `std::frexp()` directly instead of the blocktriple path. The education files use this direct path for posit conversion and blocktriple only for display.

### posit `extract_fraction()` Portability
The `extract_fraction()` function in `attributes.hpp` needed to convert blockbinary `Unsigned` to `Signed` for the `bits()` call, since `blockbinary::bits()` returns `Signed` (the upper half of the integer type).

## Files Modified

| File | Change |
|------|--------|
| `posit/posit_impl.hpp` | Bridge functions, value.hpp include |
| `posit/quire.hpp` | **New** — ported from posit1 with bt-templated interface |
| `posit/fdp.hpp` | **New** — ported from posit1 with enable_if_posit |
| `posit/twoSum.hpp` | **New** — TwoSum for new posit |
| `posit/posit.hpp` | Uncommented quire/fdp/twoSum includes |
| `posit/posit_fwd.hpp` | Uncommented quire forward declarations |
| `posit/attributes.hpp` | Fixed extract_fraction() for blockbinary |
| `posit/manipulators.hpp` | Added posit_range() |
| `blas/ext/posit_fused_blas.hpp` | Updated include path |
| `blas/ext/solvers/posit_fused_lu.hpp` | Updated include path |
| `blas/ext/solvers/posit_fused_backsub.hpp` | Made posit-agnostic |
| `blas/ext/solvers/posit_fused_forwsub.hpp` | Made posit-agnostic |
| `verification/quire_test_suite.hpp` | Updated convert call |
| `education/number/posit/values.cpp` | Rewritten with blocktriple |
| `education/number/posit/extract.cpp` | Rewritten with blocktriple |
| 23 consumer files | posit1 → posit include migration |

## Test Results

```
gcc:   934/934 tests passed (100%)
clang: 934/934 tests passed (100%)
```

## Remaining posit1 Dependencies

The following files intentionally remain on posit1:
- `applications/performance/ir/*.cpp` — use posito (posit1-based reference type)
- `include/sw/universal/number/posito/` — reference implementation, preserved by design
- `include/sw/universal/number/posit1/` — legacy implementation, still needed by posito and quire internals
