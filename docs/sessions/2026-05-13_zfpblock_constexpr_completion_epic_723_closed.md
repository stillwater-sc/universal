# Session: zfpblock constexpr completion and Epic #723 closure

**Date:** 2026-05-12 .. 2026-05-13 (rolled past midnight once)
**Branches merged:**
- `feat/issue-815-zfp-codec-constexpr` (PR #830)
- `feat/issue-816-zfparray-constexpr` (PR #831)

**Issues filed (research/design only, not implemented):**
- LNS Phase F: Arnold/Vouzis cotransformation algorithm ([#829](https://github.com/stillwater-sc/universal/issues/829))

**Issues closed (Epics):**
- [#745](https://github.com/stillwater-sc/universal/issues/745) -- zfpblock constexpr umbrella (all three sub-issues delivered)
- [#723](https://github.com/stillwater-sc/universal/issues/723) -- Epic: constexpr support across Universal number systems (all 32 sub-issues closed)

**Build directories:** `build/` (gcc), `build_clang/` (clang)

## Objectives

Close the final branch of the constexpr Epic by landing two PRs that promote the
remaining zfpblock surface -- the ZFP transform codec and the multi-block
`zfparray` container -- to constant-evaluable.  After the elastic-type cascade
landed on 2026-05-10 (PRs #824-#828), only the zfpblock umbrella ([#745]) was
still open.  The codec and the array container were explicit sub-issues
([#815], [#816]) deferred out of PR #814 (which had shipped only the trivial
accessor subset).

The two PRs deliver:

```
PR #814 (already merged) -- zfpblock accessor subset (constexpr trivial accessors)
                              |
                              v
PR #830 (this session)   -- zfpblock CODEC constexpr (encode_block / decode_block
                              + every helper they call, ~564 LOC of zfp_codec.hpp,
                              + zfpblock::compress / decompress)
                              |
                              v
PR #831 (this session)   -- zfparray multi-block container constexpr
                              (std::vector-backed compressed array, the
                               container layered on top of the codec)
```

With these two PRs, [#745] closes, and that was the last open sub-issue of
[#723] -- so the entire constexpr Epic for Universal number systems closes too.

## Why two PRs

The codec and the array container are independent units that could in principle
have been promoted together.  Splitting them gave us:

- A clean review boundary -- 270-line codec rewrite (PR #830) vs. 91-line
  array refactor (PR #831).  CodeRabbit's review windows are bounded; one
  combined PR would have hit the cap.
- A faster signal -- the codec PR's CI cycle exercised the whole compressed
  path through the existing zfpblock and array tests.  Any codec regression
  would have been caught before the array PR even started.
- An incremental Epic close -- one PR closes [#815], the next closes [#816],
  which closes [#745], which closes [#723].

## Summary of changes

### ZFP codec constexpr promotion (#815, PR #830)

`include/sw/universal/number/zfpblock/zfp_codec.hpp` is a 564-line transform
codec implementing LLNL ZFP's single-block algorithm:

```
  float[4^d] -> block-float -> lifting -> reorder -> negabinary -> bit-plane -> bits
```

PR #814 had promoted only the trivial accessors on the `zfpblock<Real, Dim>`
container.  The codec itself uses four non-constexpr stdlib facilities:

| Stdlib call                  | Used in                          | Constexpr replacement |
|------------------------------|----------------------------------|------------------------|
| `__builtin_ctzll` / `_BitScanForward64` | `zfp_ctzll`            | C++20 `std::countr_zero` (constexpr) |
| `std::frexp`                 | `fwd_cast`                       | New `cx_frexp_exp<Real>(v)` -- bit-cast IEEE 754 field extraction |
| `std::ldexp`                 | `fwd_cast`, `inv_cast`           | New `cx_ldexp<Real>(x, exp)` -- power-of-2 multiplication loop |
| `std::memset`                | `fwd_cast`, `decode_bitplanes`   | Explicit zero loops |

The runtime path keeps the stdlib intrinsics for speed; the constexpr branch
substitutes the pure replacements.  Both branches produce bit-identical output,
verified by the `zfpblock_api` test's compressed byte sequence
(`[01 f1 1c c5 61 01 ...]`) being byte-identical across gcc and clang and
across this change and main.

**Two new templated helpers** added at the top of the codec header:

```cpp
template<typename Real>
constexpr Real cx_ldexp(Real x, int exp) noexcept {
    Real two = static_cast<Real>(2);
    Real half = static_cast<Real>(0.5);
    if (exp >= 0) for (int i = 0; i < exp; ++i) x *= two;
    else          for (int i = 0; i < -exp; ++i) x *= half;
    return x;
}

template<typename Real>
constexpr int cx_frexp_exp(Real v) noexcept {
    // returns the exponent std::frexp would write to its out-parameter:
    // v = frac * 2^exp with frac in [0.5, 1.0) for finite nonzero v.
    // Implementation: sw::bit_cast of v -> uint32_t (float) or uint64_t (double),
    // extract biased exp + fraction.  Normal case: return raw_exp - 126 (float)
    // or raw_exp - 1022 (double).  Subnormal case: leading-zero scan on the
    // fraction, return k - 148 (float) or k - 1073 (double) where k is the
    // position of the highest set bit.
}
```

Both helpers are parameterized on `Real in {float, double}`.  They are local to
the codec header rather than extracted to a shared utility -- consistent with
the issue's "reuse, generalize, or copy the pattern" guidance, and avoids a
public API commitment when the same logic exists privately in `microfloat`
(PR #811's `extract_float_fields` / `cx_ldexp`).

**`zfp_bitstream` reader/writer split.**  Previously `decode_block` used
`const_cast<uint8_t*>(buffer)` to forward a `const uint8_t*` to the bitstream
constructor.  That is forbidden in constant evaluation when the storage
originated as const-qualified (the case here, since `zfpblock::decompress()
const` makes `_buffer` const).  Solved by adding a reader-mode constructor:

```cpp
constexpr zfp_bitstream(uint8_t* buffer, size_t max_bytes);         // writer (existing)
constexpr zfp_bitstream(const uint8_t* buffer, size_t max_bytes);   // reader (NEW)
```

Reader-mode stores `_write_buffer = nullptr` and `_read_buffer = buffer`;
write ops early-return when `_write_buffer == nullptr` (true no-op including
the `_bits` counter, per CodeRabbit's catch -- otherwise an accidental
`write_*()` on a reader stream would desync subsequent reads).

**Container-side promotion.**  `zfpblock::compress` / `decompress` and the
four mode convenience wrappers (`compress_fixed_rate`,
`compress_fixed_precision`, `compress_fixed_accuracy`, `compress_reversible`)
all marked constexpr.

**Test additions in `static/block/zfpblock/api/constexpr.cpp`:**
- All-zero block constexpr roundtrip (zero-flag fast path).
- Power-of-two block constexpr roundtrip (full pipeline through
  `cx_frexp_exp` + `cx_ldexp` + lifting + bit-plane) with tolerance-based
  static_assert on the decompressed values.
- Negabinary identity at both int32 and int64.

**CodeRabbit findings resolved:**
- `write_bits` reader-mode no-op (early-return before any state advance).
- Garbled comment "TN (a]er for sub-band decomposition" restored to
  "TN (filter) for sub-band decomposition" -- a corruption inherited
  unchanged from the pre-PR file.

**Drive-by CI fix:**  Added `zfpblock` to the conventional-commits scope
allowlist in `.github/workflows/conventional-commits.yml` alongside `mxblock`
and `nvblock`.  The PR-title linter had been rejecting `feat(zfpblock):` /
`fix(zfpblock):` because the scope was never registered after the type was
added to the library.

### zfparray multi-block container constexpr promotion (#816, PR #831)

`include/sw/universal/number/zfpblock/zfparray_impl.hpp` is the std::vector-
backed multi-block compressed array layered on the single-block codec.  No
`std::is_constant_evaluated()` branching was required: every helper the
container calls (`encode_block`, `decode_block`, `zfp_bitstream`, ...) is
already constexpr after PR #830.

Three classes of cleanup:

**1. Loop replacements (mechanical).**  `std::memset` and `std::memcpy`
replaced with element-wise loops throughout (cache init, store copy,
padded-block init).  Identical runtime behavior; constexpr-safe.

**2. `_store` made mutable + const_cast removal (const-correctness
improvement).**  The const lazy-cache-load methods (`operator()`,
`decompress`, `clear_cache`, `load_block`) previously cast away const to
call `flush()` / `write_back_cache()`.  These functions write the dirty
cache back into `_store` -- which is fine *semantically* because the
visible array data hasn't changed (the cache is an internal implementation
detail), but the const_cast pattern fails in constant evaluation when the
calling scope's `this` was const-qualified.

Solved by making `_store` mutable and turning `flush()` /
`write_back_cache()` into const member functions.  Strictly safer than the
prior code:

| Before | After |
|--------|-------|
| `const_cast<zfparray*>(this)->flush()` (4 sites) | `flush()` (direct call) |
| `flush()` -- non-const | `flush() const` |
| `write_back_cache()` -- non-const | `write_back_cache() const` |
| `_store` -- non-mutable; const methods can't write | `_store` -- mutable; legitimately so |

This removes a long-standing code smell that pre-dated the constexpr work.

**3. In-class member initializers.**  Every state member gets an in-class
initializer (`_n = 0`, `_rate = 0.0`, `_cache{}`, `_cached_block = SIZE_MAX`,
`_dirty = false`).  The default constructor becomes `= default` and the
explicit-rate constructors drop their `std::memset(_cache, 0, ...)` calls.
Cleaner; one less manual sync between constructors.

**Test additions in `static/block/zfpblock/array/array_constexpr.cpp` (NEW):**
- Default-construct accessor static_asserts (size, num_blocks,
  compressed_bytes, compression_ratio all == 0 at default-init).
- Sized-rate constructor static_assert (8 elements at 16 bpv -> 2 blocks
  of 4 with 8 bytes per block).
- Construct-from-source compress/decompress roundtrip on powers of two
  with tolerance check.
- set/flush/get through write-back cache.
- Cross-block access exercising cache eviction (load_block ->
  write_back_cache -> load_block).

**Naming detail:**  The new test file is `array_constexpr.cpp`, not
`constexpr.cpp`.  The `compile_all` helper derives target name from source
basename, so `api/constexpr.cpp` and `array/constexpr.cpp` would have
collided on target `zfpblock_constexpr`.  Renamed to match the existing
`array_*` prefix convention used by `array_api.cpp`, `array_cache.cpp`,
`array_copy_move.cpp`, `array_roundtrip.cpp`.

**CodeRabbit finding (skipped):**  A nitpick suggested `assert(i < _n)` in
`operator()` and `set()` for debug-build bounds checking.  Skipped because:
1. Inconsistent with the rest of the codebase -- other accessors like
   `zfpblock::data()` and `integer::operator[]` don't bounds-check either.
2. Not a real defect; a project-wide policy decision rather than a one-off
   fix.

### LNS Phase F filed (#829)

Orthogonal to the zfpblock work, this session also captured the requirements
for a more accurate LNS add/sub algorithm than the piecewise-linear
`ArnoldBaileyAddSub` shipped by Phase C ([#781], PR #786).  The current
implementation is named after Arnold but is in fact a 2.5%-relative-error
secant fit at integer-d knots -- not the cotransformation algorithm Arnold's
body of work actually develops.

[#829] specifies the "Novel Cotransformation Combination" of Vouzis,
Collange, and Arnold (J. Signal Processing Systems 58:29-40, 2010).  The
core identity:

```
  sb_sub(d) = d_l + sb_sub(d_h) + sb_add( (sb_sub(d_l) - d_l) - sb_sub(d_h) )

  where d = d_h + d_l, d_h consists of the (k + f - j) MSB of d,
                       d_l consists of the j LSB of d.

  Auxiliary LUTs:
    F3(d_l) = sb_sub(-d_l)      -- 2^j words
    F4(d_h) = sb_sub(d_h)       -- e_F4 / delta_h words
                                   where delta_h = 2^(j-f)
```

The cotransformation moves the `sb_sub` singularity at `d=0` away from the
evaluation point, so small LUTs deliver faithful rounding instead of the
percent-level error of the piecewise-linear approximation.  Vouzis et al.
report `1.93e-4` worst-case error at `f=12, n=4, g=5, h=4` (i.e. `~2^-12`
relative, matching the target precision).

[#829] is research/spec only -- no implementation.  Cross-linked to parent
Epic [#777] (configurable lns add/sub) and sibling Phase E ([#783], CORDIC,
deferred).  Ready to implement when a software-accuracy LNS consumer
materializes.

## Epic close-out

After PRs #830 and #831 merged, [#745] was the last open sub-issue of [#723].
With both Epics now closed:

**[#745] -- feat(zfpblock): full constexpr support across all operators -- CLOSED**

Delivered:
- PR #814 -- accessor subset
- PR #830 -- ZFP transform codec (encode + decode + zfp_bitstream + cx_ldexp + cx_frexp_exp)
- PR #831 -- zfparray<Real, Dim> multi-block container

Note on scope:  The issue body's "Arithmetic operators (`+`, `-`, `*`, `/`,
`%`)" and "Increment / decrement" bullets, inherited from the generic
per-type Epic template, are N/A for zfpblock: it is a *compressed-data
container*, not a scalar arithmetic type.  No such operators exist or are
intended.  The realistic constexpr surface is encode/decode + accessors
+ the multi-block array, all of which are now covered.

**[#723] -- Epic: constexpr support across Universal number systems -- CLOSED**

Final tally: all 32 sub-issues closed (5 Tier-1 primary types, 22 Tier-2
additional fixed-size types, 5 Tier-3 elastic types).

All cross-cutting prerequisites delivered:
- `blockbinary` arithmetic (#716)
- `blocksignificand` / `blocktriple` arithmetic (#718, #719)
- `blockdecimal` arithmetic (#729, #730)
- `floatcascade` (#728, #739, #742)
- error-free transforms `twoSum` / `twoProd` (#727, #738)
- `extractFields` via `BIT_CAST_CONSTEXPR` (upstream, used in #717)
- `std::is_constant_evaluated()` dispatch playbook (#716)
- `sw::math::constexpr_math` providing constexpr `log2` / `exp2` (Epic
  #763, the eventual fulfillment of #423)

The "plug-in" promise is now delivered across the library: any expression
in any fixed-size Universal type can be evaluated at compile time, drop-in
parity with `int` / `float` / `double` in any `constexpr` context.

## Validation

### gcc (build/) and clang (build_clang/) -- all 13 zfpblock + 5 array targets

For each PR, ran:

```bash
pgrep -a make                                  # safety check
cmake --build . --target <13 targets> -j4      # gcc
./<target>                                      # gcc test
cmake --build . --target <13 targets> -j4      # clang (build_clang/)
./<target>                                      # clang test
```

Results (both PRs):

| Target                       | gcc build | gcc test | clang build | clang test |
|------------------------------|-----------|----------|-------------|------------|
| zfpblock_api                 | OK | PASS (byte-identical compressed output preserved) | OK | PASS |
| zfpblock_constexpr           | OK | PASS (all static_asserts) | OK | PASS |
| zfpblock_bitplane            | OK | PASS | OK | PASS |
| zfpblock_lifting             | OK | PASS | OK | PASS |
| zfpblock_negabinary          | OK | PASS | OK | PASS |
| zfpblock_roundtrip_1d        | OK | PASS | OK | PASS |
| zfpblock_roundtrip_2d        | OK | PASS | OK | PASS |
| zfpblock_roundtrip_3d        | OK | PASS | OK | PASS |
| zfpblock_fixed_rate          | OK | PASS | OK | PASS |
| zfpblock_array_api           | OK | PASS | OK | PASS |
| zfpblock_array_cache         | OK | PASS | OK | PASS |
| zfpblock_array_copy_move     | OK | PASS | OK | PASS |
| zfpblock_array_roundtrip     | OK | PASS | OK | PASS |
| zfpblock_array_constexpr     | OK | PASS | OK | PASS | *(NEW in PR #831)*

The `zfpblock_api` test's compressed byte sequence
(`[01 f1 1c c5 61 01 00 00 00 00 00 00 00 00 00 00 ...]`) is byte-identical
across gcc and clang and across this change and main, confirming no
functional drift.

### CI -- full heavy tier green for both PRs

Both PRs passed the full CI tier on first ready submission:
- 11 platforms: Linux x64 GCC/Clang (fast + full), macOS ARM64/x64,
  Windows MSVC + MinGW, Linux ARM64/RISC-V/POWER, Android ARM64
- ASan (~26 min), UBSan (~32 min), Coverage (~27 min), Clang-Tidy, CodeQL
- lint-pr-title green (zfpblock scope registered in PR #830)

### CodeRabbit review cycles

PR #830: two review passes, three findings resolved (write_bits no-op,
garbled comment, scope linter blocker addressed by registering the scope).

PR #831: one review pass, one finding (assert in operator()/set) skipped
with rationale.

## Files touched

### PR #830 (codec)
- `include/sw/universal/number/zfpblock/zfp_codec.hpp` -- main rewrite, +200 net lines
- `include/sw/universal/number/zfpblock/zfpblock_impl.hpp` -- compress/decompress + wrappers marked constexpr
- `static/block/zfpblock/api/constexpr.cpp` -- expanded with roundtrip tests
- `.github/workflows/conventional-commits.yml` -- add zfpblock scope

### PR #831 (zfparray)
- `include/sw/universal/number/zfpblock/zfparray_impl.hpp` -- 91 insertions, 89 deletions
- `static/block/zfpblock/array/array_constexpr.cpp` -- new, 194 lines

## Lessons / patterns

1. **Make implementation-detail state mutable when const methods need to
   update it.**  The `const_cast<zfparray*>(this)` pattern is a code smell
   that the constexpr work surfaces (because const_cast fails in const
   constant evaluation).  Marking the affected member `mutable` and the
   helper functions `const` is strictly safer and fixes the smell.

2. **Bit-cast + leading-zero scan covers all of `std::frexp` for IEEE
   754.**  The `cx_frexp_exp` helper handles both normal and subnormal
   binary32 and binary64 in 30 lines.  The subnormal formula
   `exp = k - (bias + mantissa_bits)` falls out from the IEEE 754
   subnormal definition once you scan to the highest set bit.

3. **Reader/writer constructor pairs eliminate const_cast in bitstream
   APIs.**  The pattern is reusable for any class that wraps a pointer
   to a buffer where the calling context's const-ness varies.

4. **Conventional-commits scope allowlist needs to be kept in sync with
   the number system inventory.**  An ad-hoc PR-title linter rejection
   surfaced the missing `zfpblock` scope.  Other unregistered scopes
   are likely lurking; worth a future audit pass.

5. **Naming collisions in test directories.**  The `compile_all` CMake
   helper derives target name from source basename, so two tests with
   the same base name in different directories collide.  Project
   convention: prefix directory-scoped tests with the directory name
   (`array_constexpr.cpp`, not `constexpr.cpp`).
