# Session: decimal128 Support for dfloat

**Date:** 2026-02-26
**Branch:** `v3.104`
**Build directories:** `build_dfloat/` (gcc), `build_clang_dfloat/` (clang)

## Objective

Add IEEE 754-2008 decimal128 (34-digit) support to the `dfloat` number system. The existing implementation used `uint64_t` for all significand arithmetic, which can only hold 19 digits. A `static_assert(ndigits <= 19)` blocked instantiation of `dfloat<34, 12>`. The fix: introduce a conditional `significand_t` type alias using `__uint128_t` for wide significands, then update all significand-handling code.

## Changes Made (1 commit on v3.104)

### `0c777d87` — Add decimal128 support via __uint128_t significand

**4 files changed, 896 insertions(+), 149 deletions(-)**

#### Core Type System Changes (`dfloat_impl.hpp`)

**New free functions:**
- `pow10_128(unsigned n)` — constexpr `__uint128_t` power-of-10 (iterative multiply)
- `count_decimal_digits_wide(__uint128_t v)` — decimal digit counter for wide values
- `uint128_to_string(__uint128_t v)` — decimal string conversion (no `std::to_string` for `__uint128_t`)

**New class members:**
- `significand_t` — conditional type alias: `uint64_t` for ndigits <= 19, `__uint128_t` for ndigits <= 38
- `pow10_s(unsigned n)` — returns `significand_t`, dispatches to `pow10_64` or `pow10_128`
- `count_digits_s(significand_t v)` — decimal digit counter dispatching on width
- `sig_to_string(significand_t v)` — string conversion dispatching on width
- `max_ndigits` — 38 with `__uint128_t`, 19 without

**Updated methods (all changed from `uint64_t` to `significand_t`):**

| Method | Key change |
|--------|-----------|
| `unpack()` | Two-pass BID trailing read for t > 64 bits (low 64 + high 46); declet-by-declet DPD read |
| `pack()` | Two-pass BID trailing write; declet-by-declet DPD write |
| `normalize_and_pack()` | Uses `count_digits_s()` |
| `operator+=` | `if constexpr (ndigits <= 19)`: scale-up into `__uint128_t`; else: mixed scale-up/scale-down (safe_scale_up = 3) |
| `operator*=` | `if constexpr (ndigits <= 19)`: direct wide multiply; else: schoolbook split with 17-digit halves |
| `operator/=` | `if constexpr (ndigits <= 19)`: scaled numerator; else: iterative long division |
| `maxpos()`/`maxneg()` | `pow10_s(ndigits) - 1` |
| `isone()`, `scale()`, `str()` | Use `significand_t` and helpers |
| `setbits()` | Added `__uint128_t` overload |
| `convert_ieee754()` | Caps effective_digits at 17 (double precision limit), casts to `significand_t` |
| `convert_to_double()` | Unpacks to `significand_t`, `static_cast<double>` works for `__uint128_t` |
| `convert_signed()`/`convert_unsigned()` | Cast to `significand_t` before `normalize_and_pack` |

**Updated comparison operators:**
- `operator==` — uses `typename Dfloat::significand_t` for unpacked values
- `operator<` — fully native comparison (sign, scale, significand alignment) instead of double delegation

**DPD codec additions:**
- `dpd_decode_trailing_wide()` — reads declets bit-by-bit from `_block[]` for t > 64
- `dpd_encode_trailing_wide()` — writes declets bit-by-bit for t > 64
- Existing `dpd_encode_trailing()`/`dpd_decode_trailing()` updated for `significand_t`

#### Other Files

- **`manipulators.hpp`** — `components()` uses `typename Dfloat::significand_t` and `sig_to_string()`
- **`dfloat.hpp`** — Uncommented `decimal128` and `decimal128_dpd` aliases, guarded with `#ifdef __SIZEOF_INT128__`
- **`static/float/dfloat/standard/decimal128.cpp`** (new) — Regression test

## Bugs Found and Fixed

1. **`count_decimal_digits(0)` returns 1, not 0**: The split multiply checked `hh_digits > 0` but `count_decimal_digits(0)` returns 1, causing the code to enter the wrong branch and produce garbage results. Fix: check `p_hh > 0` explicitly before using digit count.

2. **Addition scale-down loses small operands**: Initial implementation for decimal128 divided the smaller-exponent operand by 10^shift, which zeroed it for small shifts (e.g., `100 + 42`: dividing 42 by 10^2 = 0). Fix: use mixed strategy — scale UP by `safe_scale_up` digits (3 for decimal128), then scale DOWN the remainder.

## Design Decisions

### Why mixed scale-up/scale-down for addition?

For ndigits <= 19, scaling up into `__uint128_t` always works: max intermediate is (10^19) * 10^18 = 10^37 < 10^38. For ndigits = 34, scaling up by the full shift would produce 10^34 * 10^33 = 10^67, far exceeding `__uint128_t` capacity (3.4 * 10^38). The compromise: scale up by at most 3 digits (10^34 * 10^3 = 10^37, safe), then scale down the other operand for the rest.

### Why schoolbook split multiply?

The product of two 34-digit numbers has 67-68 digits, far exceeding `__uint128_t`. Splitting each operand into two 17-digit halves gives four partial products, each at most 10^34 which fits `__uint128_t`. The partial products are combined with appropriate exponent adjustments, keeping only the top 34 significant digits.

### Why iterative long division?

The direct approach (numerator * 10^ndigits / denominator) overflows for decimal128: 10^34 * 10^34 = 10^68. Iterative long division computes one quotient digit per iteration: `remainder = remainder * 10; digit = remainder / divisor; remainder = remainder % divisor`. Each `remainder * 10` is at most 10^35, which fits `__uint128_t`.

### Why native operator< instead of double delegation?

`double` has only ~15-17 significant digits. Two different 34-digit decimal128 values could map to the same double, making double-based comparison incorrect. The native comparison compares signs, then decimal scales, then aligned significands.

## Test Results

```
100% tests passed, 0 tests failed out of 18 (gcc)
100% tests passed, 0 tests failed out of 18 (clang)
```

Tests (18 total):
1. dfloat_api, dfloat_logic, dfloat_assignment
2. dfloat_addition, dfloat_subtraction, dfloat_multiplication, dfloat_division
3. dfloat_decimal32, dfloat_decimal128, dfloat_dpd_codec
4. hfloat_api, hfloat_logic, hfloat_assignment
5. hfloat_addition, hfloat_subtraction, hfloat_multiplication, hfloat_division
6. hfloat_short

## Key Constraints

- `__uint128_t` is a GCC/Clang extension, not available on MSVC. All decimal128 code guarded with `#ifdef __SIZEOF_INT128__`.
- `__uint128_t` max ~ 3.4 * 10^38, so 10^34 fits comfortably but intermediate products of two 34-digit numbers (10^68) do not — necessitating split multiply.
- `std::to_string()` and `operator<<` do not work with `__uint128_t` — custom `uint128_to_string()` used throughout.
