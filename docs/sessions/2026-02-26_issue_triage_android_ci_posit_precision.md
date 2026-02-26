# Session: Issue Triage, Clang/Android Binary128, and Posit CLI Precision

**Date:** 2026-02-26
**Branch:** `v3.102`
**Build directories:** `build_ci/` (gcc/clang, CI_LITE)

## Objective

Triage and resolve outstanding GitHub issues, add Android NDK CI target, and fix Codacy static analysis configuration.

## Changes Made (3 commits on v3.102)

### 1. `23addaab` — Fix Clang long double for 128-bit binary128 targets and add Android CI (issue #485)

**Problem:** On Android aarch64 (Clang), `long double` is 128-bit IEEE binary128 (`__LDBL_MANT_DIG__ == 113`), but the Clang long double code assumed it was 64-bit (same as `double`, which is true on Apple ARM but not Android). This hit `static_assert(sizeof(long double) == 8)`.

**Fix — 3 header files + 2 CI files:**

- `include/sw/universal/native/nonconstexpr/clang_long_double.hpp`: The `#else` branch (non-POWER, non-X86) now discriminates on `__LDBL_MANT_DIG__`. For `== 113` (Android/Linux aarch64), added full binary128 implementations of `to_binary()`, `to_triple()`, `to_base2_scientific()`, `ieee_components()`, `color_print()`, and `to_hex()` using `long_double_decoder` with 15-bit exponent, 48-bit upper + 64-bit lower fraction. For `== 53` (Apple ARM), kept existing `double_decoder` code with `static_assert` removed.

- `include/sw/universal/native/compiler/ieee754_clang.hpp`: The `UNIVERSAL_ARCH_ARM` branch split with `__LDBL_MANT_DIG__` discrimination: `== 113` gets binary128 parameters (nbits=128, ebits=15, bias=16383, fbits=112), `== 53` keeps existing 64-bit double parameters.

- `include/sw/universal/native/nonconstexpr/extract_fp_components.hpp`: Removed runtime `assert(sizeof(long double) == 8 || ...)` that failed on Android. The `uint128` overload already handles the binary128 case.

- `cmake/toolchains/aarch64-linux-android.cmake`: New Android NDK toolchain file (CMAKE_SYSTEM_NAME=Android, arm64-v8a ABI, c++_static STL, API level 28). No QEMU emulator — compile-only.

- `.github/workflows/cmake.yml`: Added `Android ARM64 (NDK Clang cross)` matrix entry with `cross=android`. Test and Rerun steps skip when `matrix.cross == 'android'`.

**Verification:** 407/407 CI_LITE tests pass on both GCC and Clang locally. CI green.

### 2. `82015d33` — Fix posit CLI tool to use max_digits10 per type for decimal output (issue #281)

**Problem:** `tools/cmd/posit.cpp` used hardcoded `setprecision` values (e.g., `setprecision(3)` for posit<8,*>) that didn't show enough digits to distinguish representable posit values. Also had a copy-paste bug: posit<32,2> row printed posit<32,1>'s value.

**Fix:** Replaced all hardcoded print lines with a generic lambda using `std::numeric_limits<P>::max_digits10`:
```cpp
auto print = [](auto label, auto const& p) {
    using P = std::decay_t<decltype(p)>;
    std::cout << label << std::setw(BIT_COLUMN_WIDTH) << color_print(p)
              << " : " << std::setprecision(std::numeric_limits<P>::max_digits10) << p << '\n';
};
```
Updated static help text to match corrected output.

**Remaining:** `to_string()` converts through `long double`, so values that map to the same `long double` representation still print identically. Full fix requires native high-precision decimal conversion.

### 3. `77c4eebb` — Add .codacy.yml to exclude non-library paths from analysis

**Problem:** Codacy flagged single-quoted strings in Astro `.mjs` config files as style issues, lowering the project score. These follow Astro's default conventions and are not library code.

**Fix:** Created `.codacy.yml` excluding `docs-site/**`, `docs/**`, `.devcontainer/**`, and `bin/**` from Codacy analysis.

## Issues Triaged

| Issue | Title | Action | Status |
|-------|-------|--------|--------|
| #485 | Compiler errors for powerpc, arm, android, windows | Fixed Clang/Android binary128, added CI | **Closed** |
| #359 | Conversion error when using posit2 | Verified fixed — all conversions round-trip correctly | **Closed** |
| #341 | GCC defines wrong for PowerPC | Verified correct — binary128 decoder, POWER-aware manipulators, CI green | **Closed** |
| #281 | Posit representation precision problem | Partially fixed (max_digits10); `to_string()` long double bottleneck remains | **Open** |
| #228 | Create a fast posit<64,2/3/4> for Bayesian AI | Benchmarked: ~1 MPOPS arithmetic, needs fast specializations | **Open** |
| #224 | Specialized posits and posit_64_3 | Updated with benchmark results, same as #228 | **Open** |

## Codacy Triage

Reviewed several Codacy findings on library code:

- **`mkdirSync` with non-literal argument in `sync-content.mjs`**: False positive. All paths derived from hardcoded constants via `import.meta.dirname` and `path.join()`. No user input. Build-time script only. Now excluded by `.codacy.yml`.
- **`blockdigit::_negative` not initialized in constructor**: False positive. All value constructors delegate to `operator=` which calls `convert_signed()`/`convert_unsigned()`, both of which call `clear()` as first statement, which sets `_negative = false`.
- **`rational` variable `n` assigned in body instead of init list**: Pure style nit. `n` and `d` are scalar types; generated code is identical.
- **`rational` `operator=` should return reference**: False positive. Codacy confused `convert_ieee754()` (a private helper returning `rational&`) with `operator=`. All actual `operator=` overloads correctly return `*this`.

## Key Technical Details

### Binary128 Long Double Platform Matrix

| Platform | `sizeof(long double)` | `__LDBL_MANT_DIG__` | Format |
|----------|----------------------|---------------------|--------|
| x86_64 (GCC/Clang) | 16 | 64 | 80-bit extended (padded) |
| POWER (GCC) | 16 | 113 | IEEE 754 binary128 |
| aarch64 Linux (GCC) | 16 | 113 | IEEE 754 binary128 |
| aarch64 Linux (Clang/Android) | 16 | 113 | IEEE 754 binary128 |
| aarch64 macOS (Apple Clang) | 8 | 53 | Same as double |

### Posit Arithmetic Performance (uint64_t limbs)

| Operation | posit<64,2> | posit<64,3> |
|-----------|-------------|-------------|
| Bit manipulation (setbit/test/flip) | ~4 GPOPS | ~4 GPOPS |
| Conversion (assign+read) | ~100 MPOPS | ~100 MPOPS |
| Addition | ~1.2 MPOPS | ~1.2 MPOPS |
| Multiplication | ~0.8 MPOPS | ~0.8 MPOPS |
| Division | ~0.5 MPOPS | ~0.5 MPOPS |

Bottleneck is the generic decode-compute-encode pipeline, not the limb structure. Fast specializations needed for 100 MPOPS target.
