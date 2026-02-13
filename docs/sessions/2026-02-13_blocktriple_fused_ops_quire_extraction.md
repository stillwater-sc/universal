# Session: Rewrite Atomic Fused Operators to blocktriple and Extract Quire from posit.hpp

**Date:** 2026-02-13
**Branch:** `v3.98`
**Build directories:** `build/` (gcc, BUILD_ALL), `build_clang/` (clang, BUILD_ALL)

## Objective

Eliminate the last `internal::value<>` dependency from the posit arithmetic pipeline by:
1. Rewriting `atomic_fused_operators.hpp` (fma, fam, fmma) to use `blocktriple<>` exclusively
2. Extracting quire/fdp from `posit.hpp` so the base posit has zero `value<>` dependency

After this session, including `<universal/number/posit/posit.hpp>` brings in a fully blocktriple-based posit with fma/fam/fmma — no value<>, no bitblock<>, no module_multiply/module_add. Applications that need quire/fdp explicitly include `<universal/number/posit/quire.hpp>` or `<universal/number/posit/fdp.hpp>`.

## Architecture Decisions

### blocktriple Chaining Pattern for Fused Operations

The key challenge: blocktriple's `add()` and `mul()` require both operands to have the **same fbits template parameter**. When chaining operations (MUL→ADD for FMA, ADD→MUL for FAM), the intermediate result has different precision than the next operand.

**Solution — extract-and-reconstruct helpers:**
- `extractToAdd(src, tgt)`: transfers any post-operation blocktriple to an ADD-type blocktriple at target precision. Uses `significandscale()` to find the MSB, extracts fraction bits below it, places them in ADD layout (hidden bit + fraction + rounding shift).
- `extractToMul(src, tgt)`: same pattern for MUL-type targets (no rounding shift).
- `normalizeMultiplicationWide(posit, tgt)`: normalizes a posit to a wider-than-natural MUL blocktriple, zero-extending fraction bits.

### FAM Precision Fix: Wider MUL Type

Initial implementation used `blocktriple<fbits, MUL>` for the FAM multiply step, which truncated the ADD result's precision. For posit<3,0> (fbits=0), the sum 1+0.5=1.5 was truncated to 1.0 when extracted to MUL<0>.

**Fix:** Use `wfbits = fbits + 3` (matching ADD's rbits=3) for the MUL operands. Both the ADD→MUL transfer and the posit normalization use the wider type, preserving the addition's precision through the multiply.

### BLAS ext Headers: Consumer-Must-Include Pattern

The fused BLAS headers (`posit_fused_blas.hpp`, `posit_fused_lu.hpp`, etc.) use 2-param `posit<nbits, es>` which works with both old posit1 and new posit (via default `bt = uint8_t`). However, including `quire.hpp` from these headers pulls in the 3-param `posit.hpp`, which conflicts when consumers also include `posit1.hpp`.

**Solution:** All four headers use a "consumer must include posit and quire headers" comment pattern. New-posit consumers (lu.cpp, matrix_ops.cpp) include quire/fdp before the BLAS headers; old-posit1 consumers (luir.cpp) get quire bundled from posit1.hpp.

## Changes Made

### 1. `atomic_fused_operators.hpp` — Complete Rewrite

**Removed:** All `internal::value<>`, `bitblock<>`, `module_multiply()`, `module_add()` usage.

**Added helpers:**
- `extractToAdd<src_fbits, src_op, bt, tgt_fbits>()` — with fast path (uint64_t) and block-by-block path for large configs
- `extractToMul<src_fbits, src_op, bt, tgt_fbits>()` — same dual-path pattern
- `normalizeMultiplicationWide<nbits, es, bt, wfbits>()` — posit → wider MUL blocktriple

**Rewritten functions:**
- `fma(a, b, c)`: `product = a*b` via MUL → `extractToAdd(product)` + `extractToAdd(c)` → ADD → convert
- `fam(a, b, c)`: `sum = a+b` via ADD → `extractToMul(sum)` + `normalizeMultiplicationWide(c)` at `wfbits = fbits+3` → MUL → convert
- `fmma(a, b, c, d, opIsAdd)`: two MUL products → `extractToAdd` both → ADD → convert

### 2. `posit.hpp` — Quire/FDP Removed

Removed includes:
- `<universal/number/quire/exceptions.hpp>`
- `<universal/number/posit/quire.hpp>`
- `<universal/number/posit/fdp.hpp>`

Kept: `<universal/number/posit/atomic_fused_operators.hpp>` (now blocktriple-based)

### 3. `posit_fwd.hpp` — Cleaned

Removed forward declarations:
- `template<unsigned fbits> class value;`
- `template<unsigned nbits, unsigned es, unsigned capacity> class quire;`
- `template<...> internal::value<...> quire_mul(...)`
- `template<...> posit<...>& convert(const internal::value<...>&, ...)`

Kept: posit class, abs, sqrt, blocktriple convert, decode_regime.

### 4. `quire.hpp` / `fdp.hpp` — Made Standalone

- `quire.hpp`: added `#include <universal/number/posit/posit.hpp>` at top
- `fdp.hpp`: added `#include <universal/number/posit/quire.hpp>`

### 5. `math/sqrt.hpp` — fast_sqrt Guarded

Moved `fast_sqrt` function (which uses `internal::value<>`) inside `#if POSIT_NATIVE_SQRT` guard (defaults to 0). Added `#include <universal/internal/value/value.hpp>` inside the guard so it's only pulled in when explicitly enabled.

### 6. Consumer File Updates (25+ files)

| File | Change |
|------|--------|
| `static/quire/api/api.cpp` | Added `quire.hpp` |
| `static/quire/arithmetic/arithmetic.cpp` | Added `quire.hpp` |
| `education/quire/quires.cpp` | Added `quire.hpp` |
| `education/number/posit/exceptions.cpp` | Added `quire.hpp` |
| `education/number/posit/signalling_nar.cpp` | Added `quire.hpp` |
| `tools/cmd/propp.cpp` | Added `quire.hpp` |
| `tools/cmd/propq.cpp` | Added `quire.hpp` |
| `applications/reproducibility/blas/l1_fused_dot.cpp` | Added `fdp.hpp` |
| `applications/reproducibility/blas/norms.cpp` | Added `quire.hpp` |
| `applications/reproducibility/blas/l3_fused_mm.cpp` | Added `fdp.hpp` |
| `applications/reproducibility/blas/lu.cpp` | Added `fdp.hpp` |
| `applications/precision/numeric/residual.cpp` | Added `quire.hpp` |
| `applications/accuracy/optimization/error_vs_cost.cpp` | Added `fdp.hpp` |
| `benchmark/*/blas/gemm.cpp` (5 files) | Added `fdp.hpp` |
| `linalg/blas/vector_ops.cpp` | Added `fdp.hpp` |
| `linalg/blas/matrix_ops.cpp` | Added `fdp.hpp` |
| `blas/ext/posit_fused_blas.hpp` | Reverted to consumer-must-include |
| `blas/ext/solvers/posit_fused_lu.hpp` | Reverted to consumer-must-include |
| `blas/ext/solvers/posit_fused_backsub.hpp` | Reverted to consumer-must-include |
| `blas/ext/solvers/posit_fused_forwsub.hpp` | Reverted to consumer-must-include |

## Bugs Fixed

### FAM Precision Loss (fbits truncation)
- **Symptom:** All FAM tests failed; posit<3,0> had 16 failures, scaling up to thousands for larger configs
- **Root cause:** `extractToMul` at natural `fbits` precision dropped the ADD result's rounding bits. For fbits=0, the value 1.5 was truncated to 1.0.
- **Fix:** Use `wfbits = fbits + 3` for FAM's multiply step, with `normalizeMultiplicationWide` for posit c

### posit_fwd.hpp Template Conflict
- **Symptom:** `error: redeclared with 3 template parameters` when posit1 consumers included BLAS headers
- **Root cause:** BLAS headers pulled in new posit's quire.hpp → posit.hpp (3-param), conflicting with posit1 (2-param)
- **Fix:** BLAS ext headers use "consumer must include" pattern, not direct posit includes

## Verification

| Test | gcc | clang |
|------|-----|-------|
| `posit_fused_ops` (fma/fam/fmma exhaustive) | PASS | PASS |
| `quire_api` | PASS | PASS |
| `quire_arithmetic` | PASS | PASS |
| `posit_api` | PASS | PASS |
| Full BUILD_ALL build | 0 errors | 0 errors |

## Dependency Graph (After)

```
posit.hpp
  └── posit_impl.hpp (blocktriple-based arithmetic)
  └── atomic_fused_operators.hpp (blocktriple-based fma/fam/fmma)
  └── NO value<>, NO quire

quire.hpp (standalone, opt-in)
  └── posit.hpp
  └── value.hpp (quire internals)
  └── quire class + bridge functions

fdp.hpp (standalone, opt-in)
  └── quire.hpp
```
