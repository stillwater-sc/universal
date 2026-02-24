# Session: MSVC and uint64_t Limb Cross-Platform Fixes

**Date:** 2026-02-23
**Branch:** `v3.99`
**Build directories:** `build/` (gcc, BUILD_ALL), `build_clang/` (clang, BUILD_ALL)

## Objective

Fix MSVC build failures and undefined behavior discovered during cross-platform testing of the uint64_t block limb support added in v3.99. The session addressed six distinct issues spanning carry arithmetic, nibble UB, MSVC intrinsics, posit long double overload ambiguity, and zfpblock shift UB.

## Changes Made (7 commits on v3.99)

### 1. `574aeed1` — Fix nibble() UB in all block types for uint64_t limbs

- `0x0Fu` is a 32-bit `unsigned int`; shifting it by `nibbleIndexInWord * 4` when `nibbleIndexInWord >= 8` (shift >= 32) is undefined behavior
- On MSVC this caused corrupt `to_hex()` output and cascading test failures
- Fix: cast to `bt` before shifting so the shift operates on the block type width
- Applied to all four block types: `blockbinary`, `blockdecimal`, `blockfraction`, `blocksignificand`

### 2. `b0cb371e` — Fix MSVC intrinsic output via reference-derived pointers in carry.hpp

- MSVC `_addcarry_u64` / `_subborrow_u64` / `_umul128` write output through pointers
- When the output pointer is derived from a reference parameter, MSVC may miscompile at higher optimization levels
- Fix: write to a local variable first, then assign back through the reference

### 3. `f85cca72` — Fix blockbinary mul with uint64_t limbs passing multi-bit carry to addcarry

- When `bt = uint64_t`, the schoolbook multiplication inner loop produces carries that can exceed 1 bit (up to 64 bits wide)
- The existing code passed these multi-bit carries directly to `addcarry()`, which expects a single-bit carry input
- Fix: accumulate partial products with proper widening arithmetic; use `addcarry()` only for the final single-bit carry propagation

### 4. `59245f00` — Fix MSVC build failures for long double ambiguity and M_PI undeclared

- Added `#define _USE_MATH_DEFINES` before `<cmath>` in `directives.hpp` so `M_PI` is available on MSVC
- Added `long double` constructor/assignment overloads in the `#else` branch of posit's `LONG_DOUBLE_SUPPORT` guard (initial attempt)

### 5. `6cab5263` — Fix zfpblock shift UB and refine posit long double fix

- **zfp_codec.hpp**: `uint64_t(1) << N` is UB when `N == 64` (3D blocks with `zfp_block_size<3> = 64`). The ternary guard `(N < 64) ? ... : ~uint64_t(0)` doesn't prevent MSVC C4293 because both branches are evaluated at compile time
- Fix: added `zfp_lowbits_mask<N>()` helper using `if constexpr` to avoid the shift entirely when `N >= 64`
- **posit_impl.hpp**: Removed the `#else` long double branch (reverted in next commit)

### 6. `e7c55a63` — Restore long double overloads for MSVC

- MSVC treats `long double` and `double` as **distinct types** for overload resolution despite identical representation (`sizeof(long double) == sizeof(double)`)
- Without explicit `long double` constructor/assignment, `long double` assignment is ambiguous among `float`, `double`, and integer overloads (no single best conversion)
- The `#else` branch overloads are necessary: they provide an exact match for `long double` arguments, delegating to `double` via `static_cast`
- Updated comment to explain the actual MSVC overload resolution behavior

## Key Insight: MSVC `long double` Overload Resolution

MSVC's `long double` is identical to `double` in size and representation, but they are **distinct types** for C++ overload resolution:
- `posit(double)` + `posit(long double)`: passing `long double` → `posit(long double)` wins (exact match)
- `posit(double)` only: passing `long double` → ambiguous (equally-ranked conversions to `float` and `double`)
- Therefore the `#else` branch with `long double` overloads must be kept

## Files Modified

| File | Changes |
|------|---------|
| `include/sw/universal/internal/blockbinary/blockbinary.hpp` | nibble() UB fix, mul carry fix |
| `include/sw/universal/internal/blockdecimal/blockdecimal.hpp` | nibble() UB fix |
| `include/sw/universal/internal/blockfraction/blockfraction.hpp` | nibble() UB fix |
| `include/sw/universal/internal/blocksignificand/blocksignificand.hpp` | nibble() UB fix |
| `include/sw/universal/internal/blocktype/carry.hpp` | MSVC intrinsic pointer fix |
| `include/sw/universal/number/posit/posit_impl.hpp` | long double overload fix |
| `include/sw/universal/number/zfpblock/zfp_codec.hpp` | shift UB fix with `if constexpr` |
| `include/sw/universal/utility/directives.hpp` | `_USE_MATH_DEFINES` for MSVC |

## Testing

- All posit regression tests pass on both gcc and clang (api, conversion)
- All 12 zfpblock tests pass (including 3D roundtrip that exercises `N=64`)
- Playground skeleton builds clean (exercises `long double` assignment path)
- posit CLI tool builds and runs correctly
- ASan CI test (`fp_rounding_error_multiplication`) passes locally with clang+ASan at all optimization levels
