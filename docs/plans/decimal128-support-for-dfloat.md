# Implementation Plan: decimal128 Support for dfloat

## Context

The `dfloat` IEEE 754-2008 decimal floating-point type currently supports `decimal32` (7 digits) and `decimal64` (16 digits) but **not** `decimal128` (34 digits). This is because all significand arithmetic uses `uint64_t`, which can only hold up to 10^19 (19 digits). A `static_assert(ndigits <= 19)` blocks instantiation with 34 digits.

decimal128 is a standard IEEE 754-2008 format (1+5+12+110 = 128 bits). It must be supported. The fix: introduce a conditional `significand_t` type alias that uses `uint64_t` for ndigits <= 19 and `__uint128_t` for ndigits <= 38, then update all significand-handling code to use `significand_t`.

## Design

```cpp
// In dfloat class body:
#ifdef __SIZEOF_INT128__
static constexpr unsigned max_ndigits = 38;  // __uint128_t holds up to ~3.4×10^38
using significand_t = std::conditional_t<(ndigits <= 19), uint64_t, __uint128_t>;
#else
static constexpr unsigned max_ndigits = 19;
using significand_t = uint64_t;
#endif
static_assert(ndigits <= max_ndigits, "dfloat: ndigits exceeds significand capacity");
```

Key constraint: `__uint128_t` max ≈ 3.4×10^38, so 10^34 fits comfortably (34 < 38).

---

## Step 1: Add `significand_t` type alias and `pow10_s()` helper

**File:** `dfloat_impl.hpp` (lines 55-83, 124-144)

- Add `__uint128_t` power-of-10 table (`_pow10_128_table[]`) covering 10^0 through 10^38
- Add `pow10_128(unsigned n)` returning `__uint128_t`
- In class: add `significand_t` type alias (conditional on ndigits)
- Add `pow10_s(unsigned n)` class-level helper returning `significand_t`, dispatching to `pow10_64` or `pow10_128`
- Change `static_assert` from `ndigits <= 19` to `ndigits <= max_ndigits`

## Step 2: Update `unpack()` signature and body

**File:** `dfloat_impl.hpp` (lines 654-711)

- Change `void unpack(bool& s, int& exponent, uint64_t& significand)` → use `significand_t&`
- In BID branch: `significand = significand_t(msd) * pow10_s(ndigits - 1) + significand_t(trailing_lo) + (significand_t(trailing_hi) << 64)` — read trailing bits into `significand_t` (needs 2-pass for >64 bit trailing field)
- For 110-bit trailing significand in decimal128: read low 64 bits, then remaining 46 bits

## Step 3: Update `pack()` signature and body

**File:** `dfloat_impl.hpp` (lines 732-783)

- Change `void pack(bool s, int exponent, uint64_t significand)` → use `significand_t`
- `msd = significand / pow10_s(ndigits - 1)`, `trailing = significand % pow10_s(ndigits - 1)`
- Write trailing bits from `significand_t`: low 64 via existing loop, remaining high bits via extended loop

## Step 4: Update `normalize_and_pack()`

**File:** `dfloat_impl.hpp` (lines 787-812)

- Change `void normalize_and_pack(bool s, int exponent, uint64_t significand)` → use `significand_t`
- `count_decimal_digits()` needs a wider overload for `__uint128_t`

## Step 5: Update `maxpos()` / `maxneg()`

**File:** `dfloat_impl.hpp` (lines 509-535)

- Change `pow10_64(ndigits) - 1` → `pow10_s(ndigits) - 1`
- These already call `pack()` which will accept `significand_t` after Step 3

## Step 6: Update `operator+=()` (addition)

**File:** `dfloat_impl.hpp` (lines 253-349)

- Unpack into `significand_t` (already using `__uint128_t` intermediates, but unpacked vars are `uint64_t`)
- Change `uint64_t lhs_sig, rhs_sig` → `significand_t lhs_sig, rhs_sig`
- For ndigits <= 19: existing `__uint128_t` scale-up approach works (max intermediate ≈ 2×10^19, fits)
- For ndigits > 19 (decimal128): scale-up overflows (10^34 × 10^33 = 10^67 >> 10^38). **Use scale-DOWN approach**: divide smaller-exponent operand's significand by 10^shift instead. Max sum ≈ 2×10^34, fits `__uint128_t`
- Branch: `if constexpr (ndigits <= 19)` uses scale-up, else uses scale-down
- Remove the old `#ifdef __SIZEOF_INT128__` / `#else` split; guard the whole class with `__SIZEOF_INT128__` for ndigits > 19

## Step 7: Update `operator*=()` (multiplication)

**File:** `dfloat_impl.hpp` (lines 355-392)

- Change to `significand_t lhs_sig, rhs_sig`
- For ndigits > 19: `__uint128_t` wide multiply overflows (10^34 × 10^34 = 10^68 >> 10^38)
- **Split multiplication**: split each 34-digit significand into two 17-digit halves (hi/lo), use schoolbook with `__uint128_t` partials:
  ```
  a = a_hi * 10^17 + a_lo
  b = b_hi * 10^17 + b_lo
  product = a_hi*b_hi * 10^34 + (a_hi*b_lo + a_lo*b_hi) * 10^17 + a_lo*b_lo
  ```
  Each partial fits `__uint128_t` (10^17 × 10^17 = 10^34 < 10^38). Accumulate by keeping only the top 34 significant digits, adjusting exponent.

## Step 8: Update `operator/=()` (division)

**File:** `dfloat_impl.hpp` (lines 393-436)

- Change to `significand_t lhs_sig, rhs_sig`
- For ndigits > 19: `lhs_sig * pow10_s(ndigits)` overflows (10^34 × 10^34 = 10^68)
- **Iterative long division**: compute quotient digit-by-digit:
  ```
  remainder = lhs_sig
  quotient = 0
  for i in range(ndigits):
      remainder *= 10       // max ≈ 10^35, fits __uint128_t
      quotient = quotient * 10 + remainder / rhs_sig
      remainder = remainder % rhs_sig
  ```
  Adjust exponent by `-ndigits`.

## Step 9: Update `operator==()` and other comparisons

**File:** `dfloat_impl.hpp` (lines 1040-1050, 1057-1070)

- Change `uint64_t lsig, rsig` → `significand_t lsig, rsig` in `operator==`
- `operator<`: currently delegates through `double` — for decimal128, `double` loses precision. Implement native comparison: compare signs, then exponents, then significands directly.

## Step 10: Update `isone()`, `scale()`, `str()`

**File:** `dfloat_impl.hpp` (lines 559-639)

- `isone()`: change `uint64_t sig` → `significand_t sig`
- `scale()`: change `uint64_t sig` → `significand_t sig`; `count_decimal_digits()` needs wider overload
- `str()`: change `uint64_t sig` → `significand_t sig`; `std::to_string()` doesn't work with `__uint128_t`. Add `sig_to_string(significand_t)` helper.

## Step 11: Update `convert_ieee754()` and `convert_to_double()`

**File:** `dfloat_impl.hpp` (lines 830-887)

- `convert_ieee754(double)`: double has ~15-17 significant digits, so significand from double always fits uint64_t. For ndigits > 19, cast result to `significand_t` before calling `normalize_and_pack`.
- `convert_to_double()`: unpack into `significand_t`, convert to double via `static_cast<double>(sig)` which works for `__uint128_t` on gcc/clang.

## Step 12: Update `convert_signed()` / `convert_unsigned()`

**File:** `dfloat_impl.hpp` (lines 889-922)

- Input types are at most `uint64_t`, so no overflow. Cast to `significand_t` before calling `normalize_and_pack`.

## Step 13: Update DPD codec for wider significands

**File:** `dpd_codec.hpp` (lines 272-336)

- `dpd_encode_significand()` and `dpd_decode_significand()` currently use `uint64_t`
- decimal128 DPD has 11 declets (33 digits / 3 = 11) → 110 trailing bits, which exceeds 64 bits
- Change to template or overload accepting `__uint128_t`
- Also update `dpd_decode_trailing()` and `dpd_encode_trailing()` in `dfloat_impl.hpp` (lines 816-824) to use `significand_t`

## Step 14: Update `manipulators.hpp` — `components()`

**File:** `manipulators.hpp` (lines 68-84)

- Change `uint64_t sig` → `typename dfloat<...>::significand_t sig`
- Use `sig_to_string()` helper for output (since `<<` doesn't work for `__uint128_t`)

## Step 15: Uncomment `decimal128` aliases in `dfloat.hpp`

**File:** `dfloat.hpp` (lines 79-87)

- Uncomment `using decimal128 = dfloat<34, 12, DecimalEncoding::BID, uint32_t>;`
- Uncomment `using decimal128_dpd = dfloat<34, 12, DecimalEncoding::DPD, uint32_t>;`
- Guard with `#ifdef __SIZEOF_INT128__`

## Step 16: Add `setbits()` overload for wider values

**File:** `dfloat_impl.hpp` (line 498)

- Current `setbits(uint64_t)` can't set all 128 bits of decimal128
- Add overload or template accepting `__uint128_t`

## Step 17: Create regression test for decimal128

**File (new):** `static/float/dfloat/standard/decimal128.cpp`

- static_assert on field widths: `nbits==128, t==110, es==12, combBits==5`
- Special values: zero, inf, NaN, maxpos, minpos
- Integer round-trip through decimal128 (small and large values)
- Arithmetic: addition, subtraction, multiplication, division with 34-digit values
- Exactness test: `10 * 0.1 == 1.0` (decimal exactness)
- BID/DPD agreement: same values should produce same arithmetic results

**CMake:** add `dfloat_decimal128` target to `static/float/dfloat/CMakeLists.txt`

---

## Key Helper Functions to Add

```cpp
// Wide power-of-10 (free function, above class)
static constexpr __uint128_t pow10_128(unsigned n);

// In-class significand-width power-of-10
static constexpr significand_t pow10_s(unsigned n);

// Wide decimal digit counter
static constexpr unsigned count_decimal_digits_wide(__uint128_t v);

// __uint128_t to string (for str() and components())
static std::string sig_to_string(significand_t v);
```

---

## Files Modified (summary)

| File | Changes |
|------|---------|
| `dfloat_impl.hpp` | `significand_t` alias, wider pow10, wider unpack/pack/normalize, wider arithmetic, wider comparisons, wider helpers |
| `dpd_codec.hpp` | Wider `dpd_encode_significand()` / `dpd_decode_significand()` |
| `manipulators.hpp` | `components()` uses `significand_t` |
| `dfloat.hpp` | Uncomment decimal128/decimal128_dpd aliases |

## Files Created

| File | Purpose |
|------|---------|
| `static/float/dfloat/standard/decimal128.cpp` | Regression test for decimal128 |

---

## Verification

1. Build with gcc: `cd build_dfloat && cmake -DUNIVERSAL_BUILD_NUMBER_DFLOATS=ON .. && make -j4`
2. Run tests: `ctest` — all existing tests must still pass (decimal32, decimal64 unchanged)
3. New `dfloat_decimal128` test must pass
4. Build with clang: `cd build_clang_dfloat && cmake -DUNIVERSAL_BUILD_NUMBER_DFLOATS=ON .. && make -j4 && ctest`
5. Verify: `decimal128` instantiation compiles and basic operations work
6. Key test: large-precision arithmetic doesn't overflow intermediates

## Safety Reminders
- ONE build at a time, `make -j4` max
- Test with BOTH gcc and clang
- Never claim tests pass without running them
- `__uint128_t` (not `__int128`) to avoid `-Wpedantic` warnings
