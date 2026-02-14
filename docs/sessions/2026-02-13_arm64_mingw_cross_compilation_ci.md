# Session: ARM64 and MinGW Cross-Compilation CI with Bug Fixes

**Date:** 2026-02-13
**Branch:** `v3.98`
**Build directories:** `build_mingw/` (MinGW, CI_LITE)

## Objective

Add ARM64 Linux and MinGW Windows x64 cross-compilation CI targets to the existing `cmake.yml` workflow, following the established pattern from RISC-V and PPC64LE entries. Fix all platform-specific compilation and runtime issues discovered during bring-up.

## Changes Made (7 commits)

### 1. `c21d3426` — Add ARM64 and MinGW-w64 cross-compilation CI targets

- Created `cmake/toolchains/aarch64-linux-gnu.cmake` (QEMU emulation)
- Created `cmake/toolchains/x86_64-w64-mingw32.cmake` (Wine emulation)
- Added two matrix entries and install steps to `.github/workflows/cmake.yml`

### 2. `2fc73cf7` — Fix long double decoder for ARM64 128-bit quad precision

- ARM64 `long double` is IEEE 754 quad (128-bit), not x87 80-bit
- `long_double_decoder` uses `bit63` member only available for x87 format
- Fixed `include/sw/universal/native/ieee754_decoder.hpp` to use `limb()` for quad format

### 3. `833d8892` — Fix extract_fp_components redefinition on MinGW

- MinGW defines `uint64_t` as `unsigned long long` (like MSVC), not `unsigned long` (like Linux GCC)
- The `extract_fp_components(unsigned long long, ...)` overload conflicted with the `uint64_t` overload
- Added `#if !defined(_WIN32)` guard around the `unsigned long long` overload

### 4. `a9236641` — Fix C API test linking for MinGW

- C API test targets needed `ws2_32` on Windows and static linking for Wine

### 5. `f7e1264d` — Use target-based add_test() for cross-compilation

- `compile_and_link_all` macro's `add_test(NAME x COMMAND x)` pattern didn't work for cross-compiled executables
- Switched C API tests to use `$<TARGET_FILE:target>` expressions

### 6. `869dcfae` — Static link GCC/C++ runtime in MinGW toolchain

- Wine couldn't find `libgcc_s_seh-1.dll` and `libstdc++-6.dll` at runtime
- Added `CMAKE_EXE_LINKER_FLAGS_INIT "-static"` to toolchain file

### 7. `4399b134` — Fix MinGW GCC optimizer bugs (LNS + floatcascade)

Two MinGW-specific GCC bugs caused 5 CI test failures:

**Bug 1: LNS sign bit loss (4 tests)**
- GCC's IPA ICF (Identical Code Folding) incorrectly merges `.part.0` fragments of different `lns<nbits>` template instantiations after function splitting
- When `lns<4>` and `lns<8>` are instantiated in the same TU, the 8-bit version loses all sign bits (125/256 encodings fail)
- Order-dependent: if 8-bit is instantiated first, both pass; if 4-bit first, 8-bit fails
- Does NOT reproduce on native Linux GCC 13.3.0 (same version) — MinGW PE/COFF specific
- Fix: `-fno-ipa-icf`

**Bug 2: floatcascade multiplication precision loss (1 test)**
- MinGW's software `std::fma()` (used when hardware FMA3 is not enabled) has precision errors of 1-2 ULPs for some inputs
- This breaks the error-free `two_prod` transformation that `floatcascade<4>` multiplication relies on
- Quad-double precision drops from 212 bits to as low as 77 bits (single double)
- Verified by bit-exact comparison: `fma(925.227, 242.061, -223961.373)` gives `0x...4148` on MinGW vs `0x...4146` on Linux
- Fix: `-mfma` to use hardware FMA3 instructions instead of software fallback

Both flags set via `CMAKE_CXX_FLAGS_INIT` in the MinGW toolchain file.

## Investigation Methodology

### LNS Bug Diagnosis

1. **Reproduced locally** with MinGW cross-compiler + Wine
2. **Narrowed to 8-bit configurations** — 4-bit and 9-bit pass, 8-bit fails
3. **Discovered order dependency** — swapping instantiation order made bug appear/disappear
4. **Identified ICF as cause** — `-fno-ipa-icf` fixed it; `-fno-partial-inlining` also fixed it
5. **Confirmed via GCC ICF dump** (`-fdump-ipa-icf-details`):
   - 4 functions merged: `lns<4>::setbit.part.0`, `lns<8>::setbit.part.0`, `blockbinary<4>::setbit.part.0`, `blockbinary<8>::setbit.part.0`
   - Functions are genuinely code-identical (nbits check is in the outer wrapper, not `.part.0`)
   - The merging SHOULD be safe, but something in the PE/COFF COMDAT interaction causes incorrect behavior

### floatcascade Bug Diagnosis

1. **Verified `std::fma()` correctness** — simple test cases match between MinGW and Linux
2. **Found discrepancy** — specific inputs (`925.227 * 242.061`) produce different `two_prod` error terms
3. **Confirmed hardware FMA fixes it** — `-mfma` makes all results match Linux

## Test Results

**Before fixes:**
- 5 test failures: `lns_assignment`, `lns_conversion`, `lns_division`, `lns_multiplication`, `fc_arith_multiplication_precision`

**After fixes:**
- 390/390 tests pass (0 failures)
- Full CI_LITE suite verified locally under Wine

## Files Modified

| File | Change |
|------|--------|
| `.github/workflows/cmake.yml` | Added ARM64 + MinGW matrix entries and install steps |
| `cmake/toolchains/aarch64-linux-gnu.cmake` | New — ARM64 cross-compilation toolchain |
| `cmake/toolchains/x86_64-w64-mingw32.cmake` | New — MinGW toolchain with static linking + workarounds |
| `include/sw/universal/native/ieee754_decoder.hpp` | ARM64 quad long double fix |
| `include/sw/universal/native/extract_fp_components.hpp` | MinGW uint64_t guard |
| `c_api/shim/test/posit/CMakeLists.txt` | Target-based add_test |
| `c_api/pure_c/test/posit/CMakeLists.txt` | Target-based add_test |

## Additional Fix: blockbinary operator[] vs test() misuse

### Bug: ASan stack-buffer-overflow in `positFraction::operator<<`

CI reported AddressSanitizer stack-buffer-overflow in `fp_rounding_error_multiplication` test. Root cause: `blockbinary::operator[](unsigned)` is a **block/limb** accessor (returns `BlockType`), NOT a bit accessor. Code was using it with bit indices.

For `posit<16,1,uint8_t>` → `positFraction<12, uint8_t>` → `blockbinary<12, uint8_t>`:
- 2 blocks (indices 0..1), but `_block[11]` accessed block 11 → out-of-bounds

### Fixes Applied

| File | Line | Bug | Fix |
|------|------|-----|-----|
| `positFraction.hpp` | 232 | `f._block[unsigned(i)]` in `operator<<` | `f._block.test(unsigned(i))` |
| `positFraction.hpp` | 66 | `_block[i]` in `get_fixed_point()` | `_block.test(i)` |
| `positFraction.hpp` | 191 | `_block[i + shift]` in `denormalize()` | `_block.test(unsigned(i + shift))` |
| `posit_impl.hpp` | 796 | `_block[nbits-1]` in reciprocal | `_block.test(nbits-1)` |

### Verification

- `fp_rounding_error_multiplication`: PASS (gcc + clang)
- `posit_api`: PASS (gcc + clang)
- `posit_division`: PASS (clang) — exercises reciprocal path
