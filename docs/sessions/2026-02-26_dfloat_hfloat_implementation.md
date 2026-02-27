# Session: dfloat (IEEE 754-2008 Decimal FP) and hfloat (IBM System/360 Hex FP)

**Date:** 2026-02-26
**Branch:** `v3.104`
**Build directories:** `build_dfloat/` (gcc), `build_clang_dfloat/` (clang)

## Objective

Implement two new number systems completing the floating-point radix family: `dfloat` (IEEE 754-2008 decimal floating-point with BID and DPD encodings) and `hfloat` (IBM System/360 hexadecimal floating-point). IBM mainframes historically provided all three in hardware: binary (IEEE 754), hexadecimal (System/360, 1964), and decimal (financial industry). Adding these serves as an educational resource for comparing encoding tradeoffs.

## Changes Made (2 commits on v3.104)

### 1. `c618d701` — Implement dfloat and hfloat core infrastructure, arithmetic, math libraries

**49 files changed, 4082 insertions(+), 534 deletions(-)**

#### dfloat: IEEE 754-2008 Decimal Floating-Point

**Design:** Template `dfloat<ndigits, es, Encoding, bt>` with `DecimalEncoding` enum `{BID, DPD}`. Both encodings in one type for educational comparison.

**Storage layout:** `[sign(1)] [combination(5)] [exponent_continuation(w)] [trailing_significand(t)]`

| Alias | Config | Bits |
|-------|--------|------|
| decimal32 | `dfloat<7, 6, BID>` | 1+5+6+20 = 32 |
| decimal64 | `dfloat<16, 8, BID>` | 1+5+8+50 = 64 |
| decimal128 | `dfloat<34, 12, BID>` | 1+5+12+110 = 128 |

**Key implementation details:**
- `nbits` computed via constexpr `bid_trailing_bits()` using integer approximation `ceil(n * log2(10)) = ceil(n * 3322 / 1000)` — avoids non-constexpr `std::ceil/std::log2`
- Combination field: `ab!=11` → exp_msbs=ab, MSD=0cde; `ab==11,c!=1` → exp_msbs=cd, MSD=100e; `11110`=inf; `11111`=NaN
- Addition aligns by scaling UP the larger-exponent operand (not dividing down smaller — prevents precision loss)
- Multiplication/division use `__uint128_t` for wide intermediate results
- DPD branches via `if constexpr (encoding == DecimalEncoding::DPD)` in significand pack/unpack
- DPD codec: canonical IEEE 754-2008 Table 3.3 truth table with 8-case encode/decode, all 1000 round-trips verified

**Files modified:**
- `dfloat_fwd.hpp` — Added `DecimalEncoding` enum, 4-param forward declaration (defaults removed to avoid redefinition error)
- `dfloat_impl.hpp` — Complete rewrite (~900 lines): storage, combination field, arithmetic, conversions
- `dfloat.hpp` — Umbrella header with standard aliases
- `manipulators.hpp` — Updated for 4-param template, added `color_print`, `components`
- `attributes.hpp` — Updated for 4-param template

**Files created:**
- `dpd_codec.hpp` — DPD encode/decode using IEEE 754-2008 truth table
- `numeric_limits.hpp` — `radix=10`, `is_exact=true`
- `dfloat_traits.hpp` — `is_dfloat` trait
- `mathlib.hpp` + 13 function files — Math library delegating through double

#### hfloat: IBM System/360 Hexadecimal Floating-Point

**Design:** Template `hfloat<ndigits, es, bt>` where ndigits = hex fraction digits, es = exponent bits (7 for standard).

**Storage layout:** `[sign(1)] [exponent(7)] [hex_fraction(ndigits*4)]`

**Key behaviors:** No hidden bit, no NaN, no infinity, no subnormals. Truncation rounding only. Overflow saturates. Wobbling precision (0-3 leading zero bits in MSB hex digit). `SpecificValue::qnan/snan` → zero, `infpos/infneg` → maxpos/maxneg.

| Alias | Config | Bits |
|-------|--------|------|
| hfloat_short | `hfloat<6, 7>` | 1+7+24 = 32 |
| hfloat_long | `hfloat<14, 7>` | 1+7+56 = 64 |
| hfloat_extended | `hfloat<28, 7>` | 1+7+112 = 120 |

**All files created from scratch:** `hfloat_fwd.hpp`, `exceptions.hpp`, `hfloat_impl.hpp`, `hfloat.hpp`, `manipulators.hpp`, `attributes.hpp`, `numeric_limits.hpp`, `hfloat_traits.hpp`, `mathlib.hpp` + 13 function files, `CMakeLists.txt`, `api/api.cpp`

**CMake wiring:** 3 insertion points in root `CMakeLists.txt` — option, STATICS cascade, add_subdirectory.

### 2. `f3384524` — Add regression tests for dfloat and hfloat

**14 files created, 1845 insertions(+)**

#### dfloat regression tests (7 files):
- `conversion/assignment.cpp` — Integer/float round-trip, SpecificValue, unsigned types, DPD encoding, decimal exactness (10 * 0.1 = 1.0)
- `logic/logic.cpp` — ==, !=, <, >, <=, >= with positive, negative, and zero values
- `arithmetic/addition.cpp` — Basic, fractional (powers of 2), different scales, negatives, commutativity, inf/NaN
- `arithmetic/subtraction.cpp` — Basic, anti-commutativity
- `arithmetic/multiplication.cpp` — Basic, commutativity, multiplicative identity, inf/NaN
- `arithmetic/division.cpp` — Basic, self-division, division by zero, inf/NaN
- `standard/decimal32.cpp` — Field width static_asserts, special values, BID/DPD value and arithmetic agreement

#### hfloat regression tests (7 files):
- `conversion/assignment.cpp` — Integer/float round-trip, powers of 2, powers of 16, no-NaN/no-inf SpecificValue mapping, unsigned types
- `logic/logic.cpp` — All comparison operators with positive, negative, and zero values
- `arithmetic/addition.cpp` — Basic, powers of 2, commutativity, truncation rounding verification
- `arithmetic/subtraction.cpp` — Basic, anti-commutativity
- `arithmetic/multiplication.cpp` — Basic, commutativity, multiplicative identity, powers of 16
- `arithmetic/division.cpp` — Basic, self-division, truncation rounding (1/3), division by zero saturation
- `standard/short.cpp` — Field width static_asserts, no NaN/inf, trivially constructible, wobbling precision, type_tag, dynamic range

## Bugs Found and Fixed

1. **Default template argument redefinition**: `dfloat_fwd.hpp` and `dfloat_impl.hpp` both declared defaults for `DecimalEncoding`. Fix: removed defaults from forward declaration.

2. **Non-constexpr `nbits`**: Used `std::ceil(std::log2(std::pow(10.0, ...)))`. Fix: created `bid_trailing_bits()` with integer approximation.

3. **Protected `getbit()` access**: Free functions (`to_binary`, `color_print`) couldn't access protected member. Fix: moved to public section.

4. **Addition alignment bug** (100+3=100): Dividing rhs significand by 10^shift lost precision. Fix: scale UP the larger-exponent operand instead.

5. **hfloat quarter (0.25) = 0**: Hex exponent calculation for negative binary exponents was wrong. Fix: use `bin_exp / 4` (C integer division truncates toward zero = ceil for negatives).

6. **Test: 0.1 + 0.2 != 0.3**: C++ double literal `0.1` is already binary-imprecise; after round-trip through dfloat, `0.100000000000000005551...` + `0.200000000000000011102...` != `double(0.3)`. Fix: changed test to use powers-of-2 fractions that are exactly representable in both binary and decimal.

## Test Results

```
100% tests passed, 0 tests failed out of 17 (gcc)
100% tests passed, 0 tests failed out of 17 (clang)
```

Tests:
1. dfloat_api, dfloat_logic, dfloat_assignment
2. dfloat_addition, dfloat_subtraction, dfloat_multiplication, dfloat_division
3. dfloat_decimal32, dfloat_dpd_codec
4. hfloat_api, hfloat_logic, hfloat_assignment
5. hfloat_addition, hfloat_subtraction, hfloat_multiplication, hfloat_division
6. hfloat_short

## Architecture Notes

- **BID vs DPD tradeoff**: BID stores trailing significand as binary integer (simple arithmetic, used by Intel), DPD stores as 10-bit declets (compact display, used by IBM/IEEE hardware). Same combination field for both.
- **hfloat wobbling precision**: A value like 1.0 (binary 0.0001 in hex) has 3 wasted leading zero bits in its first hex digit, giving only 21 effective fraction bits instead of 24. Value 8.0 (binary 0.1000) uses all 24 bits. This is a fundamental tradeoff of base-16 representation.
- **Truncation vs IEEE rounding**: hfloat always truncates (chops toward zero). This is simpler to implement in hardware but introduces systematic negative bias. IEEE 754 round-to-nearest-even eliminates this bias.
