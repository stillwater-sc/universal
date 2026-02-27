# Implementation Plan: dfloat (Decimal FP) and hfloat (Hexadecimal FP)

## Context

Universal is an educational C++ template library for custom arithmetic types. It currently has binary floating-point (`cfloat`) but lacks decimal and hexadecimal floating-point systems. IBM mainframes historically provided all three in hardware: binary (IEEE 754), hexadecimal (System/360, 1964 -- hardware benefits from reduced alignment shifts), and decimal (financial industry, exact base-10 representation). Adding `dfloat` (IEEE 754-2008 decimal) and `hfloat` (IBM HFP) completes the floating-point radix family and serves as an educational resource for comparing encoding tradeoffs.

A `dfloat` skeleton already exists but is entirely stubbed. `hfloat` is greenfield.

## Design Decisions (User-Confirmed)

- **dfloat encoding**: Template parameter `DecimalEncoding` enum `{BID, DPD}` -- both encodings in one type for educational comparison
- **dfloat template**: `dfloat<ndigits, es, Encoding, bt>` (keep existing param names, add Encoding)
- **hfloat behavior**: Classic IBM System/360 -- no NaN, no infinity, no subnormals, truncation rounding, overflow saturates
- **hfloat template**: `hfloat<ndigits, es, bt>` where ndigits = hex fraction digits, es = exponent bits (7 for standard)
- **Scope**: Full implementation including math library

---

## Phase 1: dfloat Core Infrastructure (BID)

Rewrite the dfloat skeleton with correct IEEE 754-2008 decimal storage layout and BID encoding.

### Storage Layout

IEEE 754-2008 format: `[sign(1)] [combination(5)] [exponent_continuation(w)] [trailing_significand(t)]`

| Alias | Config | Bits |
|-------|--------|------|
| decimal32 | `dfloat<7, 6, BID>` | 1+5+6+20 = 32 |
| decimal64 | `dfloat<16, 8, BID>` | 1+5+8+50 = 64 |
| decimal128 | `dfloat<34, 12, BID>` | 1+5+12+110 = 128 |

Key static constexprs:
```cpp
static constexpr unsigned p = ndigits;              // precision digits
static constexpr unsigned w = es;                   // exponent continuation bits
static constexpr unsigned t = nbits - 1 - 5 - w;   // trailing significand bits
static constexpr int bias = (3 << (w - 1)) + p - 2;
```

Combination field (5 bits `abcde`):
- `ab != 11`: exp MSBs = `ab`, MSD = `0cde` (digit 0-7)
- `ab == 11, c != 1`: exp MSBs = `cd`, MSD = `100e` (digit 8-9)
- `11110`: infinity; `11111`: NaN

### Files to Modify

| File | Changes |
|------|---------|
| `include/sw/universal/number/dfloat/dfloat_fwd.hpp` | Add `DecimalEncoding` enum, 4-param forward decl |
| `include/sw/universal/number/dfloat/dfloat_impl.hpp` | Complete rewrite: fix storage calc, combination field encode/decode, BID significand pack/unpack, `clear/setzero/setinf/setnan/setsign`, `iszero/isinf/isnan/sign/scale`, `maxpos/minpos/zero/minneg/maxneg`, `convert_ieee754` (double->dfloat), `convert_to_ieee754` (dfloat->double), `convert_signed/convert_unsigned`, comparison operators |
| `include/sw/universal/number/dfloat/dfloat.hpp` | Add Encoding template param to aliases, uncomment traits/numeric_limits includes |
| `include/sw/universal/number/dfloat/manipulators.hpp` | Implement `to_binary()` showing field boundaries, `type_tag()` with encoding name |
| `include/sw/universal/number/dfloat/attributes.hpp` | Fix template params, implement `dynamic_range()` |
| `static/float/dfloat/api/api.cpp` | Rewrite for 4-param template, test BID `dfloat<7,6>` and `dfloat<16,8>` |

### Files to Create

| File | Purpose |
|------|---------|
| `include/sw/universal/traits/dfloat_traits.hpp` | `is_dfloat_trait`, `is_dfloat`, `enable_if_dfloat` (pattern: `traits/cfloat_traits.hpp`) |
| `include/sw/universal/number/dfloat/numeric_limits.hpp` | `std::numeric_limits<dfloat>` specialization (radix=10) |

### Reference Files
- `include/sw/universal/number/cfloat/cfloat_impl.hpp` -- class structure pattern
- `include/sw/universal/traits/cfloat_traits.hpp` -- traits pattern

---

## Phase 2: dfloat Arithmetic (BID)

Implement all four arithmetic operations for BID encoding.

### Algorithm Outlines

**Addition**: Unpack both operands to `(sign, exponent, significand_integer)`. Align by dividing smaller-exponent significand by `10^shift`. Add/subtract based on signs. Normalize result to p digits. Round per IEEE 754-2008.

**Multiplication**: `result_sig = sig_a * sig_b` (needs 2p-digit intermediate via `__uint128_t` or custom wide int). `result_exp = exp_a + exp_b`. Normalize to p digits.

**Division**: `result_sig = (sig_a * 10^p) / sig_b`. `result_exp = exp_a - exp_b`. Remainder determines rounding.

### Files to Modify
- `dfloat_impl.hpp`: Implement `operator+=`, `-=`, `*=`, `/=`, `operator-()`, `operator++/--`

### Files to Create
- `static/float/dfloat/conversion/assignment.cpp` -- native type round-trip tests
- `static/float/dfloat/conversion/decimal_conversion.cpp` -- string conversion tests (verify 0.1 exact)
- `static/float/dfloat/logic/logic.cpp` -- comparison tests including NaN semantics
- `static/float/dfloat/arithmetic/addition.cpp`
- `static/float/dfloat/arithmetic/subtraction.cpp`
- `static/float/dfloat/arithmetic/multiplication.cpp`
- `static/float/dfloat/arithmetic/division.cpp`

---

## Phase 3: dfloat DPD Encoding

Add DPD (Densely Packed Decimal) as alternate encoding, branching via `if constexpr`.

DPD maps 3 BCD digits to a 10-bit declet. Each declet classifies digits as "small" (0-7) or "large" (8-9), giving 8 encoding patterns. Encode/decode via constexpr lookup tables (1000-entry encode, 1024-entry decode).

### Files to Create
- `include/sw/universal/number/dfloat/dpd_codec.hpp` -- encode/decode tables + functions
- `static/float/dfloat/standard/dpd_codec.cpp` -- exhaustive verification of all 1000 encodings

### Files to Modify
- `dfloat_impl.hpp`: Add `if constexpr (Encoding == DPD)` branches in significand pack/unpack

---

## Phase 4: dfloat Standard Aliases and Polish

### Files to Modify
- `dfloat.hpp`: Add aliases:
  ```cpp
  using decimal32  = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;
  using decimal64  = dfloat<16, 8, DecimalEncoding::BID, uint32_t>;
  using decimal128 = dfloat<34, 12, DecimalEncoding::BID, uint32_t>;
  using decimal32_dpd  = dfloat<7, 6, DecimalEncoding::DPD, uint32_t>;
  // etc.
  ```
- `manipulators.hpp`: Add `color_print`, `pretty_print`, `components`

### Files to Create
- `static/float/dfloat/standard/decimal32.cpp` -- field width verification, known bit patterns
- `static/float/dfloat/standard/decimal64.cpp`
- `static/float/dfloat/standard/decimal128.cpp`

---

## Phase 5: hfloat Core Infrastructure

Create IBM System/360 hexadecimal floating-point from scratch.

### Storage Layout

Format: `[sign(1)] [exponent(7)] [hex_fraction(ndigits*4)]`

Value: `(-1)^sign * 16^(exponent - 64) * 0.f1f2...fn`

| Alias | Config | Bits |
|-------|--------|------|
| hfloat_short | `hfloat<6, 7>` | 1+7+24 = 32 |
| hfloat_long | `hfloat<14, 7>` | 1+7+56 = 64 |
| hfloat_extended | `hfloat<28, 7>` | 1+7+112 = 120 (stored in 128) |

Key behaviors:
- No hidden bit, no NaN, no infinity, no subnormals
- Truncation rounding only
- Overflow saturates to maxpos/maxneg
- Wobbling precision: 0-3 leading zero bits in MSB hex digit
- Zero: fraction all zeros

### Files to Create (all new)

**Headers** (`include/sw/universal/number/hfloat/`):
- `hfloat.hpp` -- umbrella header
- `hfloat_fwd.hpp` -- forward declarations + aliases
- `exceptions.hpp` -- exception hierarchy (no NaN exceptions; overflow/underflow)
- `hfloat_impl.hpp` -- main class: template `<ndigits, es, bt>`, storage, constructors, operators, conversions, comparisons. `setinf()` maps to `maxpos()`. SpecificValue `qnan/snan` maps to zero.
- `numeric_limits.hpp` -- `radix=16`, `has_infinity=false`, `has_quiet_NaN=false`
- `manipulators.hpp` -- `type_tag`, `to_binary` (show hex digit boundaries), `to_hex`
- `attributes.hpp` -- `dynamic_range`, `sign`, `scale` (returns `4*(exp-64)`)

**Traits**: `include/sw/universal/traits/hfloat_traits.hpp`

**Tests** (`static/float/hfloat/`):
- `CMakeLists.txt`
- `api/api.cpp`
- `conversion/assignment.cpp`
- `conversion/hex_conversion.cpp`
- `logic/logic.cpp`
- `standard/short.cpp`, `standard/long.cpp`, `standard/extended.cpp`

### CMake Wiring (root `CMakeLists.txt`)

3 insertion points:
1. ~line 167: `option(UNIVERSAL_BUILD_NUMBER_HFLOATS "Set to ON to build static hfloat tests" OFF)`
2. ~line 802 in `STATICS` cascade: `set(UNIVERSAL_BUILD_NUMBER_HFLOATS ON)`
3. ~line 1027 after dfloat: `if(UNIVERSAL_BUILD_NUMBER_HFLOATS) add_subdirectory("static/float/hfloat") endif()`

---

## Phase 6: hfloat Arithmetic

Implement arithmetic with hex-digit alignment and truncation rounding.

**Addition**: Align by shifting fraction right by `4*(exp_large - exp_small)` bits. Add/subtract. Normalize by shifting hex digits until leading hex digit != 0. Truncate.

**Multiplication**: `result_frac = frac_a * frac_b` (wide multiply). `result_exp = exp_a + exp_b - 64`. Normalize to ndigits hex digits. Truncate.

**Division**: `result_frac = (frac_a << ndigits*4) / frac_b`. `result_exp = exp_a - exp_b + 64`. Normalize. Truncate.

### Files to Create
- `static/float/hfloat/arithmetic/addition.cpp`
- `static/float/hfloat/arithmetic/subtraction.cpp`
- `static/float/hfloat/arithmetic/multiplication.cpp`
- `static/float/hfloat/arithmetic/division.cpp`
- `static/float/hfloat/performance/perf.cpp`

---

## Phase 7: Math Libraries (Both Types)

Initial implementation delegates through `double` for all math functions:
```cpp
template<...> TypeName func(TypeName x) {
    return TypeName(std::func(double(x)));
}
```

Functions: `exp/exp2/exp10/expm1`, `log/log2/log10/log1p`, `pow/sqrt/cbrt/hypot`, `sin/cos/tan/asin/acos/atan/atan2`, `sinh/cosh/tanh/asinh/acosh/atanh`, `trunc/floor/ceil/round`, `fmod/remainder`, `copysign/nextafter/fabs/abs`, `fmin/fmax/fdim`, `erf/erfc/tgamma/lgamma`, `fma`

For hfloat: `isnan()` always returns false, `isinf()` always returns false, overflow saturates.

### Files to Create (per type)
- `math/classify.hpp`, `math/exponent.hpp`, `math/logarithm.hpp`, `math/trigonometry.hpp`, `math/hyperbolic.hpp`, `math/sqrt.hpp`, `math/pow.hpp`, `math/minmax.hpp`, `math/next.hpp`, `math/truncate.hpp`, `math/fractional.hpp`, `math/hypot.hpp`, `math/error_and_gamma.hpp`
- `mathlib.hpp` umbrella
- Test files: `static/float/{dfloat,hfloat}/math/*.cpp`

---

## Phase 8: Integration and Testing

1. Build and test with gcc (`make -j4`)
2. Build and test with clang (critical -- clang is stricter on UB, uninitialized vars)
3. Verify `ReportTrivialityOfType` passes for all configurations
4. Verify `constexpr` correctness (no `std::frexp/ldexp` in constexpr paths)
5. Key test: `dfloat` represents 0.1 exactly (unlike binary float)
6. Key test: `hfloat` truncation rounding never rounds up

### Build Commands
```bash
mkdir build_dfloat && cd build_dfloat
cmake -DUNIVERSAL_BUILD_NUMBER_DFLOATS=ON -DUNIVERSAL_BUILD_NUMBER_HFLOATS=ON ..
make -j4
ctest
```

### Safety Reminders
- ONE build at a time, `make -j4` max (never `-j$(nproc)`)
- Test with BOTH gcc and clang before considering done
- Never claim tests pass without running them
