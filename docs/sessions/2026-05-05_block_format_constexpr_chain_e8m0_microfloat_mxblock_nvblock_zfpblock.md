# Session: OCP / NVIDIA block-format constexpr chain (e8m0, microfloat, mxblock, nvblock, zfpblock)

**Date:** 2026-05-05
**Branches merged:** `feat/issue-731-e8m0-constexpr` (PR #810), `feat/issue-733-microfloat-constexpr` (PR #811), `feat/issue-734-mxfloat-constexpr` (PR #812), `feat/issue-735-nvblock-constexpr` (PR #813), `feat/issue-745-zfpblock-constexpr` (PR #814)
**Build directories:** `build_ci/` (gcc), `build_ci_clang/` (clang), `build_ubsan/` (clang + UBSan)

## Objectives

Continue Epic [#723](https://github.com/stillwater-sc/universal/issues/723) (constexpr promotion across the library) by closing the entire OCP / NVIDIA block-format dependency chain in dependency order:

```
e8m0  ->  microfloat  ->  mxblock (OCP Microscaling)
              \
               '-->  nvblock (NVIDIA NVFP4)

zfpblock (independent; ZFP transform codec)
```

1. Resolve [#731](https://github.com/stillwater-sc/universal/issues/731) -- bring `e8m0` (8-bit OCP exponent-only scale) to full constexpr
2. Resolve [#733](https://github.com/stillwater-sc/universal/issues/733) -- bring `microfloat` (8-bit float family: e2m1, e3m2, e4m3, e5m2) to full constexpr
3. Resolve [#734](https://github.com/stillwater-sc/universal/issues/734) -- bring `mxblock` (OCP Microscaling block) to full constexpr
4. Resolve [#735](https://github.com/stillwater-sc/universal/issues/735) -- bring `nvblock` (NVIDIA NVFP4 two-level scaled block) to full constexpr
5. Address [#745](https://github.com/stillwater-sc/universal/issues/745) -- bring `zfpblock` to constexpr to whatever extent is feasible without a multi-day codec rewrite, file follow-ups for the rest

## Summary of Changes

### e8m0 full constexpr (#731, PR #810)

`e8m0` is an 8-bit exponent-only OCP scaling factor. The bit pattern encodes a
biased exponent (`bias = 127`); encoding 0 represents 2^-127, encoding 0xFF
represents NaN. It does not store a mantissa, sign, or fraction, so the
conversion math is just bit-shifts on the IEEE 754 binary32 representation.

**Files modified:**
- `include/sw/universal/number/e8m0/e8m0_impl.hpp` -- ~120 line diff
- `static/float/e8m0/api/constexpr.cpp` -- new (148 lines)

**Key transformations:**
- `to_float()` constructs IEEE 754 bit patterns directly via `sw::bit_cast`:
  - Encoding 0 (special: 2^-127) emits the subnormal pattern `0x00400000`
    (mantissa 1, exponent 0) so the value rounds correctly to a representable
    float
  - Other encodings construct `(rawExp << 23)` with `rawExp = bits + 1`
    (the +1 corrects for the e8m0 bias relative to IEEE 754)
- `from_float()` extracts `rawExp` via `(bits >> 23) & 0xFF`, with round-up
  at `frac >= 3474676` (~`(sqrt(2)-1) * 2^23`, the half-octave threshold)
- Infinity check moved BEFORE the `v <= 0` clamp so `-inf` encodes to
  maxpos rather than zero (CR Critical fix)
- `operator>=` rewritten as `operator> || operator==` for NaN-safety
  (CR Critical fix; the previous `!operator<` returned true for NaN)
- The unreachable `std::isinf` runtime branch (after the early infinity check)
  was removed (CR Nitpick fix)

**Tests added:**
- Round-trip 2^-127 through 2^127 in constant-eval
- NaN encoding (0xFF) propagation through `to_float()` (`x != x`)
- Infinity-input encoding to maxpos (positive AND negative)
- All comparison operators against NaN (each returns false except `!=`)
- `numeric_limits` invariants

### microfloat full constexpr (#733, PR #811)

`microfloat<nbits, ebits, hasSubnormals, hasSupernormals, isSaturating>` covers
the 8-bit float family used as the per-element type in mxblock and the
per-element / scale type in nvblock. The promotion is more invasive than e8m0
because the type stores a sign bit and a fraction, has subnormal handling, and
supports per-instantiation NaN / saturation policies.

**Files modified:**
- `include/sw/universal/number/microfloat/microfloat_impl.hpp` -- ~310 line diff
- `static/float/microfloat/api/constexpr.cpp` -- new (310 lines)

**Key transformations:**
- New private helpers replace `std::frexp/ldexp/isnan/isinf/signbit/fabs/copysign`:
  - `extract_float_fields(float v) -> {sign, rawExp, rawFrac}` via `sw::bit_cast`
  - `cx_ldexp(float x, int exp)` -- bounded power-of-2 multiplication loop
- IEEE 754 binary32 layout assumption made explicit at file scope:
  `static_assert(sizeof(float) == sizeof(uint32_t)
                 && std::numeric_limits<float>::is_iec559,
                 "microfloat constexpr conversion requires IEEE 754 binary32");`
  (CR Major fix Round 2; previously the assumption was implicit)
- Mixed-type comparison overloads (12 of them: `microfloat OP {float, double, int, ...}`
  and the symmetric reflections) short-circuit on float NaN BEFORE narrowing
  to microfloat:
  ```cpp
  if (rhs != rhs) return false;  // operator==/<=/<, return false on NaN
  if (rhs != rhs) return true;   // operator!=, return true on NaN
  ```
  (CR Major fix Round 2; without this, `microfloat<...,hasNaN=false>` would
  silently quantize float NaN to a finite value before comparing)
- Signed zero preservation: `_bits = s ? sign_mask : 0x00u` instead of
  `_bits = static_cast<uint8_t>(s ? sign_mask : 0)` -- the prior form would
  collapse `-0.0f` to `+0` on some toolchains under aggressive constant-eval
  folding (CR Major fix)
- `_bits` storage stays `uint8_t _bits;` (no in-class initializer) to keep
  the type trivially constructible

**Tests added:**
- Round-trip across the e2m1, e3m2, e4m3, e5m2 representable ranges
- NaN propagation in operator>=/<=/==/!= (mixed-type and pure)
- Subnormal handling for hasSubnormals=true configurations
- e2m1_no_nan default-ctor pinned via value-init: `e2m1_no_nan z{};`
  (the trivial form `e2m1_no_nan z;` left storage indeterminate)

### mxblock full constexpr (#734, PR #812)

`mxblock<ElementType, BlockSize>` pairs one e8m0 scale with `BlockSize`
microfloat elements per the OCP Microscaling v1.0 spec. The constexpr
promotion is the first one in this chain that uses the
`std::is_constant_evaluated()` dispatch pattern -- runtime keeps the fast
stdlib calls; constant-eval routes through new bit-twiddling helpers.

**Files modified:**
- `include/sw/universal/number/mxfloat/mxblock_impl.hpp` -- ~140 line diff
- `static/block/mxblock/api/constexpr.cpp` -- new (160 lines)

**Key transformations:**
- Three new private constexpr helpers (only used at constant evaluation):
  - `cx_fabs(float v)` -- `v < 0.0f ? -v : v`
  - `cx_floor_log2(float v)` -- IEEE 754 bit-extraction:
    - Normals: `floor(log2(v)) == rawExp - 127`
    - Subnormals: `-149 + leading-bit-position` of the fraction
  - `cx_ldexp(float x, int exp)` -- bounded loop multiply by 2.0 / 0.5
- `quantize`, `dequantize`, `operator[]`, `dot`, `clear`, `setbits` all
  marked `constexpr`
- `set_element_from_float` keeps `std::round` at runtime; constant-eval
  uses `static_cast<int>(v + (v < 0 ? -0.5f : 0.5f))` for half-away-from-zero
- All-zeros amax fast path, NaN-scale propagation, e8m0 bias clamping
  preserved bit-for-bit

**Tests added:**
- Constant-eval `quantize` of `{1.0f, 0.5f, 0.25f, 0.0f}` into `mxblock<e2m1, 4>`
- `dot()` between two non-zero blocks at constant evaluation
- NaN-scale propagation: `setbits(0xFF)` -> `operator[]` returns NaN
- All-zeros amax: scale clamps to `2^(-elemMaxExp + 127)`

### nvblock full constexpr (#735, PR #813)

`nvblock<ElementType, BlockSize, ScaleType>` is NVIDIA's NVFP4 two-level
scaled block format: per-block fractional scale (e4m3) plus external FP32
tensor_scale. Cleaner promotion than mxblock because `raw_scale = amax / elem_max`
is a fractional value -- no `log2/floor/ldexp` needed at all.

**Files modified:**
- `include/sw/universal/number/nvblock/nvblock_impl.hpp` -- ~80 line diff
- `static/block/nvblock/api/constexpr.cpp` -- new (218 lines)

**Key transformations:**
- `quantize/dequantize/operator[]/dot/clear/setscalebits` marked `constexpr`
- `std::fabs` replaced inline with `(x < 0) ? -x : x` (no helper needed)
- `compute_elem_max()` materializes via
  `ElementType(SpecificValue::maxpos).to_float()` -- constexpr after PR #811
  brought `microfloat::to_float()` into constexpr
- `_block_scale.from_float(raw_scale)` and `_block_scale.to_float()` are
  both constexpr (after #811 promoted the e4m3 conversion path); the
  underflow-to-minpos guard is still needed and works at constant evaluation

**Tests added:**
- Default-construction zero contract
- `quantize` from a zero input array -> all elements zero, scale cleared
- Round-trip `setscalebits(0x38u)` (e4m3 1.0) plus per-element assignment ->
  `operator[]` reads back exact values
- Explicit `dequantize()` coverage via a `four_floats` aggregate (CR Nitpick fix;
  `operator[]` alone doesn't exercise `dequantize`'s tensor_scale path)
- NaN block_scale (encoding `0x7Fu`) -> `operator[]` returns NaN, `dot` returns NaN
- `dot()` with dual tensor_scales at constant evaluation:
  `cx_dot_scaled == 6.0f * cx_dot` (linearity verified compile-time)

### zfpblock partial constexpr (#745, PR #814)

This was the most pragmatic call of the session. `zfpblock` combines:

| Component                                | Disposition          |
|------------------------------------------|----------------------|
| Static block buffer + integer accessors  | Promoted (this PR)   |
| `compute_limits` helper                  | Promoted (this PR)   |
| 564-line ZFP transform codec             | Deferred (#815)      |
| `std::vector`-backed `zfparray` array    | Deferred (#816)      |

The codec uses `std::frexp`, `std::ldexp`, `std::memset`, and bit-stream
packing across `zfp_codec.hpp` (564 lines). Promoting it would be a
multi-day effort that the issue does not require, and the per-element
accessors (which is what users actually need at constant evaluation today)
do not depend on it. The two deferred slices are tracked in:

- [#815](https://github.com/stillwater-sc/universal/issues/815) -- ZFP codec full constexpr (encode/decode pipeline)
- [#816](https://github.com/stillwater-sc/universal/issues/816) -- `zfparray` full constexpr (blocked by #815)

**Files modified:**
- `include/sw/universal/number/zfpblock/zfpblock_impl.hpp` -- accessor diff
- `static/block/zfpblock/api/constexpr.cpp` -- new (147 lines)

**Promoted accessors:**
```cpp
constexpr size_t compressed_bits() const noexcept;
constexpr size_t compressed_bytes() const noexcept;
constexpr double compression_ratio() const noexcept;  // returns 0.0 on _nbits == 0
constexpr const uint8_t* data() const noexcept;
constexpr zfp_mode mode() const noexcept;
constexpr double param() const noexcept;
static constexpr size_t block_size() noexcept;
static constexpr unsigned dim() noexcept;
static constexpr void compute_limits(zfp_mode, double, unsigned&, size_t&) noexcept;
```

**Tests added:**
- Empty-state contract: value-initialized `zfp_f1{}` has `compressed_bits() == 0`,
  `compressed_bytes() == 0`, `compression_ratio() == 0.0` (tightened from
  `(void)cx_*` to explicit `static_assert` per CR Nitpick)
- `BLOCK_SIZE`, `MAX_BYTES`, `dim()`, `block_size()` static asserts for
  zfp_f1, zfp_d2, zfp_d3
- `data()` returns valid (non-null) pointer at constant evaluation
- Default `mode()` and `param()` accessor smoke tests

**CR pattern this PR:** CodeRabbit reviewed the initial commit and produced
one nitpick (the empty-state contract assertions were too lax). The
follow-up commit `6e02b330` tightened them. CR's habit across this session
has been to skip re-reviewing test-only fix-up commits, so the merge
proceeded once core CMake CI was green and sanitizers + Coverage + CodeQL
finished clean.

## Test Results

All five PRs landed with both gcc and clang Release builds passing. Local
UBSan (clang `build_ubsan/`) clean for all five. CI on `main` after each merge:

| PR  | Issue | Targets | gcc Release | clang Release | UBSan local | CI Sanitizers | Codacy / CodeQL |
|-----|-------|---------|-------------|----------------|-------------|----------------|------------------|
| #810 | #731  | e8m0_constexpr + e8m0 suite | PASS | PASS | PASS | PASS | PASS |
| #811 | #733  | microfloat_constexpr + microfloat suite + downstream e8m0 / dd / qd | PASS | PASS | PASS | PASS | PASS |
| #812 | #734  | mxblock_constexpr + mxblock suite | PASS | PASS | PASS | PASS | PASS |
| #813 | #735  | nvblock_constexpr + nvblock suite | PASS | PASS | PASS | PASS | PASS |
| #814 | #745 (partial) | zfpblock_constexpr + zfpblock suite | PASS | PASS | PASS | PASS | PASS |

## CodeRabbit Findings Resolved

| PR  | Severity | Finding | Resolution |
|-----|----------|---------|------------|
| #810 | Critical | `to_float` for negative infinity returned 0 instead of maxpos | Reordered infinity check before `v <= 0` clamp |
| #810 | Critical | `operator>=` returned true for NaN | Replaced `!operator<` with `operator> || operator==` |
| #810 | Major    | `2.0f * fmax` not a constant expression | Replaced with `numeric_limits<float>::infinity()` |
| #810 | Nitpick  | Unreachable `std::isinf` runtime branch | Removed |
| #811 | Major (R2) | IEEE 754 binary32 layout assumption implicit | Added explicit `static_assert(sizeof(float) == sizeof(uint32_t) && is_iec559)` |
| #811 | Major (R2) | Mixed-type comparisons quantize float NaN before comparing | Added `if (rhs != rhs) return false/true` short-circuits in 12 overloads |
| #811 | Major    | `-0.0f` collapsed to `+0` | `_bits = s ? sign_mask : 0x00u` to preserve signed zero |
| #813 | Nitpick  | `dequantize()` constexpr coverage missing | Added direct `dequantize()` assertion via `four_floats` aggregate |
| #814 | Nitpick  | Empty-state contract assertions used `(void)cx_*` | Tightened to explicit `static_assert(cx_nbits == 0u, ...)` |

## Loose ends and follow-ups

- [#815](https://github.com/stillwater-sc/universal/issues/815) -- full constexpr promotion of ZFP transform codec (encode/decode pipeline)
- [#816](https://github.com/stillwater-sc/universal/issues/816) -- full constexpr promotion of `zfparray` multi-block container (blocked by #815)
- Roadmap comment posted on umbrella issue [#745](https://github.com/stillwater-sc/universal/issues/745) explaining the partial close

## Files Changed (cumulative across the five PRs)

```
include/sw/universal/number/e8m0/e8m0_impl.hpp
include/sw/universal/number/microfloat/microfloat_impl.hpp
include/sw/universal/number/mxfloat/mxblock_impl.hpp
include/sw/universal/number/nvblock/nvblock_impl.hpp
include/sw/universal/number/zfpblock/zfpblock_impl.hpp
static/float/e8m0/api/constexpr.cpp           (new)
static/float/microfloat/api/constexpr.cpp     (new)
static/block/mxblock/api/constexpr.cpp        (new)
static/block/nvblock/api/constexpr.cpp        (new)
static/block/zfpblock/api/constexpr.cpp       (new)
CHANGELOG.md
```

## Status

Epic [#723](https://github.com/stillwater-sc/universal/issues/723) constexpr
coverage at end of session:

| Number system     | Status              | Closing PR |
|-------------------|---------------------|------------|
| qd / qd_cascade / td_cascade | Done    | #800 / #799 / #798 |
| dfixpnt           | Done                | #803 |
| dfloat            | Done                | #805 |
| hfloat            | Done                | #806 |
| dfixpnt overflow fix | Done             | #807 |
| qd / dd / floatcascade to_digits | Done | #808 |
| e8m0              | Done                | #810 |
| microfloat        | Done                | #811 |
| mxfloat / mxblock | Done                | #812 |
| nvblock           | Done                | #813 |
| zfpblock          | Partial (accessors) | #814 |
| zfp transform codec | Tracked            | #815 (open) |
| zfparray          | Tracked, blocked    | #816 (open) |
