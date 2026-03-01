# Decimal fixpnt

## Context

The library needs a decimal fixed-point type (`dfixpnt`) for exact decimal arithmetic (financial, COBOL-style, BCD applications). The existing `blockdecimal` is a 15-line alias over `blockdigit` (1 byte/digit, encoding-unaware) — unsuitable as a building block. This plan refactors `blockdecimal` into a standalone class with compact BCD/BID/DPD encoding backed by `blockbinary`, then builds `dfixpnt` on top of it.

Template signatures:
```cpp
template<unsigned ndigits, DecimalEncoding encoding = DecimalEncoding::BCD, typename bt = uint8_t>
class blockdecimal;  // unsigned decimal integer building block

template<unsigned ndigits, unsigned radix, DecimalEncoding encoding = DecimalEncoding::BCD,
         bool arithmetic = Modulo, typename bt = uint8_t>
class dfixpnt;  // signed decimal fixed-point number type
```

Encoding Options for decimal fixpnt: dfixpnt<ndigits, radix, arithmetic>

Where ndigits = total decimal digits, radix = fractional digits, so integer digits = ndigits - radix.

---
Option A: BCD (Binary-Coded Decimal) — 4 bits per digit

```text
  [sign][d_{n-1}][d_{n-2}]...[d_1][d_0]
    1b     4b      4b          4b   4b

  nbits = 1 + 4 * ndigits

  Each nibble holds one decimal digit (0–9). Storage: blockbinary<1 + 4*ndigits, bt, Unsigned>.
```

|       Aspect       |                   Assessment                    |
|--------------------|-------------------------------------------------|
| Storage efficiency | 83% (10 of 16 codes used per nibble)            |
| Decimal I/O        | Trivial — each nibble IS a digit                |
| Addition           | Nibble-wise add + decimal-adjust (carry if >9)  |
| Multiplication     | Digit-by-digit schoolbook or BCD multiply       |
| Modulo/saturating  | Natural — clamp or wrap per-digit at boundaries |
| Rounding           | Exact — no binary↔decimal conversion artifacts  |

---
Option B: Binary Integer Decimal (BID) — binary encoding of scaled value

```text
  [sign][binary magnitude of value × 10^radix]
    1b    ceil(ndigits × 3.3219) bits

  nbits = 1 + ceil(ndigits × log2(10))

  Store the value as a plain binary integer scaled by 10^radix. Storage: blockbinary<nbits, bt, Unsigned>.
```

|       Aspect       |                        Assessment                        |
|--------------------|----------------------------------------------------------|
| Storage efficiency | ~100% (minimal bits)                                     |
| Decimal I/O        | Requires repeated div/mod by 10                          |
| Addition           | Binary add — fast, delegate to blockbinary               |
| Multiplication     | Binary multiply then divide by 10^radix to rescale       |
| Modulo/saturating  | Check magnitude >= 10^ndigits after each operation       |
| Rounding           | Rescaling after multiply introduces truncation decisions |

---
  Option C: Densely Packed Decimal (DPD) — 10 bits per 3 digits

```text
  [sign][declet_{k-1}]...[declet_1][declet_0][remainder digits]
    1b      10b              10b       10b       0/4/8 bits

  nbits = 1 + 10 * (ndigits/3) + 4 * (ndigits%3)
```

|       Aspect       |                       Assessment                        |
|--------------------|---------------------------------------------------------|
| Storage efficiency | 97% (1000 of 1024 codes per declet)                     |
| Decimal I/O        | Moderate — encode/decode 3 digits at a time             |
| Arithmetic         | Must decode → compute → re-encode                       |
| Complexity         | High — already complex in dfloat, worse for fixed-point |

  ---

For a fixed-point type, BCD is the strongest fit:

  1. Fixed-point arithmetic is digit-aligned — addition never changes the radix position, so nibble-wise add with decimal-adjust is the natural operation. No
   rescaling, no range validation.
  2. Modulo/saturating are clean — modulo wraps at 10^ndigits (just discard carry out); saturating clamps to 10^ndigits − 1 (all 9s). Both are trivial to
  detect from the carry chain.
  3. No precision surprises — BID requires a magnitude < 10^ndigits check after every operation (the binary range exceeds the decimal range). BCD can't
  represent invalid values at all if you enforce 0–9 per nibble.
  4. String conversion is free — critical for financial/COBOL use cases where decimal exactness is the whole point.
  5. Multiplication cost is acceptable — schoolbook digit-by-digit multiply is O(ndigits²), but for typical sizes (8–34 digits) this is fine. The BID
  advantage (fast binary multiply) is offset by the expensive rescaling division.
  6. The library already has blockdecimal — this may provide BCD building blocks.

The storage overhead vs BID is modest: a dfixpnt<16,6> (16 digits, 6 fractional) needs 65 bits in BCD vs 55 in BID — both fit in a single 64-bit block plus one extra byte.

## Analysis: blockdecimal as dfixpnt Building Block

Current State

|          Component           |               Storage               |       Sign       |        Arithmetic         |
|------------------------------|-------------------------------------|------------------|---------------------------|
| blockdigit<N,R>              | uint8_t _digit[N] + bool _negative  | Sign-magnitude   | Digit-by-digit schoolbook |
| blockdecimal<N>              | Alias for blockdigit<N,10>          | (same)           | (same)                    |
| blockbinary<N,bt>            | bt _block[nrBlocks]                 | Two's complement | Binary with carry chains  |
| fixpnt<nbits,rbits,arith,bt> | blockbinary<nbits,bt,Signed> _block | Two's complement | Binary, then radix-shift  |

### The Problem with blockdigit as-is

blockdigit stores one digit per byte — a blockdecimal<16> uses 16 bytes + 1 sign byte = 17 bytes. This is a pure radix representation, 
not BCD, BID, or DPD. It's fine as a pedagogical integer type, but it's:

  1. Memory-wasteful — 8 bits to store a value 0–9 (only 3.3 bits of information per byte)
  2. Encoding-unaware — no concept of BCD (4 bits/digit), BID (binary integer), or DPD (10 bits/3 digits)
  3. Not parameterized by encoding — always uses the same 1-byte-per-digit layout
  4. No fixed-point semantics — purely integer, no radix point

### Evaluation: Separate blockdecimal vs Keep as blockdigit Alias

Option 1: Keep blockdecimal as blockdigit alias, build dfixpnt on new storage

Create a new internal building block (e.g., decimal_storage<ndigits, encoding, bt>) that handles BCD/BID/DPD. 
Leave blockdigit/blockdecimal untouched for their current users (positional, rational).

|               Pro                |                 Con                  |
|----------------------------------|--------------------------------------|
| No risk to existing users        | Another parallel storage abstraction |
| Clean slate for encoding support | Can't share code with blockdigit     |

Option 2: Refactor blockdecimal into a standalone class with encoding support

Break blockdecimal away from the blockdigit alias. Make it a real class parameterized by encoding, backed by blockbinary for compact bit storage:

```cpp
template<unsigned ndigits, DecimalEncoding encoding = DecimalEncoding::BCD, typename bt = uint8_t>
class blockdecimal {
      // BCD:  blockbinary<4 * ndigits, bt, Unsigned>  — 4 bits per digit
      // BID:  blockbinary<bid_bits(ndigits), bt, Unsigned>  — binary integer
      // DPD:  blockbinary<dpd_bits(ndigits), bt, Unsigned>  — 10 bits per 3 digits
};
```

|                           Pro                           |                               Con                               |
|---------------------------------------------------------|-----------------------------------------------------------------|
| Reuses blockbinary for compact storage                  | Must preserve old blockdecimal<N> API or migrate users          |
| Single building block for dfixpnt                       | positional and rational use blockdigit directly (not via alias) |
| DPD codec already exists in dfloat                      | More complex class                                              |
| Natural name — blockdecimal should mean "decimal block" |                                                                 |

**Key insight**: positional and rational use blockdigit<N,R> directly (not the blockdecimal alias), so breaking the alias has zero impact on existing users.
The current blockdecimal.hpp is just a 16-line alias file that nobody imports directly.

### Recommendation: Option 2 — Refactor blockdecimal

The architecture would be:

```text
  blockbinary<nbits, bt>          ← binary storage engine (existing)
      ↑                               ↑
      │                               │
  fixpnt<nbits, rbits, arith, bt>   blockdecimal<ndigits, encoding, bt>  ← NEW
      (binary fixed-point)              ↑
                                        │
                                    dfixpnt<ndigits, radix, encoding, arith>  ← NEW
                                        (decimal fixed-point)
```

blockdecimal handles:

  - Encoding/decoding digits ↔ bits (BCD, BID, DPD)
  - Unsigned decimal integer arithmetic in the chosen encoding
  - digit(i) accessor, setdigit(i, d), digit iteration
  - Overflow detection (value ≥ 10^ndigits)

dfixpnt handles:

  - Sign (sign-magnitude, like dfloat)
  - Radix point placement
  - Modulo vs Saturating overflow policy
  - Conversions to/from native types
  - String I/O with decimal point

Storage sizes for blockdecimal<16, encoding>

|      Encoding      | Bits | Bytes |           Efficiency           |
|--------------------|------|-------|--------------------------------|
| BCD                | 64   | 8     | 83% (40 of 48 bits carry info) |
| BID                | 54   | 7     | 99%                            |
| DPD                | 54   | 7     | 97%                            |
| Current blockdigit | 128  | 16    | 41%                            |

All three encodings are 2× more compact than the current 1-byte-per-digit layout.

## Plan: blockdecimal Refactor + dfixpnt Decimal Fixed-Point Type

### Critical Files

| File | Action |
|------|--------|
| `include/sw/universal/number/shared/decimal_encoding.hpp` | NEW — shared `DecimalEncoding` enum (BCD, BID, DPD) |
| `include/sw/universal/number/shared/decimal_bits.hpp` | NEW — `bcd_bits()`, `bid_bits()`, `dpd_bits()` helpers |
| `include/sw/universal/internal/blockdecimal/blockdecimal.hpp` | REWRITE — standalone class with encoding support |
| `include/sw/universal/number/dfixpnt/dfixpnt.hpp` | NEW — umbrella header |
| `include/sw/universal/number/dfixpnt/dfixpnt_fwd.hpp` | NEW — forward declarations |
| `include/sw/universal/number/dfixpnt/dfixpnt_impl.hpp` | NEW — main implementation |
| `include/sw/universal/number/dfixpnt/exceptions.hpp` | NEW |
| `include/sw/universal/number/dfixpnt/numeric_limits.hpp` | NEW |
| `include/sw/universal/number/dfixpnt/attributes.hpp` | NEW |
| `include/sw/universal/number/dfixpnt/manipulators.hpp` | NEW |
| `include/sw/universal/number/dfixpnt/mathlib.hpp` | NEW |
| `include/sw/universal/traits/dfixpnt_traits.hpp` | NEW |
| `include/sw/universal/number/dfloat/dfloat_fwd.hpp` | UPDATE — use shared DecimalEncoding |
| `include/sw/universal/number/fixpnt/fixpnt_impl.hpp` | UPDATE — use shared Modulo/Saturate |
| `static/fixpnt/decimal/` | UPDATE — repurpose test stubs for dfixpnt |
| `CMakeLists.txt` | UPDATE — add `UNIVERSAL_BUILD_NUMBER_DFIXPNTS` option |

Unchanged: `blockdigit.hpp`, `positional`, `rational` (they use `blockdigit` directly, not the `blockdecimal` alias).

## Implementation Steps

### Step 1: Shared Infrastructure

**1a. `include/sw/universal/number/shared/decimal_encoding.hpp`** (new):
```cpp
enum class DecimalEncoding { BCD, BID, DPD };
```
Add `BCD` to the existing enum (currently only `BID`/`DPD` in `dfloat_fwd.hpp`).

**1b. `include/sw/universal/number/shared/decimal_bits.hpp`** (new):
```cpp
static constexpr unsigned bcd_bits(unsigned n) { return 4u * n; }
static constexpr unsigned bid_bits(unsigned n) {
    if (n == 0) return 0;
    return static_cast<unsigned>((static_cast<uint64_t>(n) * 3322u + 999u) / 1000u);
}
static constexpr unsigned dpd_bits(unsigned n) {
    return (n / 3) * 10 + (n % 3 == 1 ? 4 : n % 3 == 2 ? 7 : 0);
}
```

**1c. Update `dfloat_fwd.hpp`**: Replace local `DecimalEncoding` enum with `#include <universal/number/shared/decimal_encoding.hpp>`.

**1d. Update `fixpnt_impl.hpp`**: Keep `Modulo`/`Saturate` constants in place (they're already in `sw::universal` namespace, dfixpnt will include fixpnt or redefine — see note below). Alternative: extract to `include/sw/universal/number/shared/arithmetic_policy.hpp` and include from both. Use the simpler approach: just define them again in dfixpnt_impl.hpp with a guard, or include from fixpnt.

**Decision**: Define `Modulo`/`Saturate` in dfixpnt_impl.hpp identically (they're `constexpr bool` with internal linkage — no ODR issue). No shared header needed for two constants.

### Step 2: blockdecimal Refactor

Rewrite `include/sw/universal/internal/blockdecimal/blockdecimal.hpp` as a standalone class.

**Storage**:
```cpp
static constexpr unsigned nbits =
    (encoding == DecimalEncoding::BCD) ? bcd_bits(ndigits) :
    (encoding == DecimalEncoding::BID) ? bid_bits(ndigits) :
                                         dpd_bits(ndigits);
blockbinary<nbits, bt, BinaryNumberType::Unsigned> _block;
```

**Key interface** (unsigned — sign is in dfixpnt):
- `digit(i)` / `setdigit(i, d)` — encoding-aware digit access
- `operator+=`, `-=`, `*=`, `/=`, `%=` — arithmetic
- `operator==`, `<`, etc. — comparison
- `iszero()`, `clear()`, `maxpos()`, `setbits()`
- `to_bid()` / `from_bid()` — convert to/from binary integer for mixed-encoding arithmetic
- Trivial default constructor (`blockbinary _block;` — no initializer)

**Arithmetic strategy by encoding**:

| Operation | BCD | BID | DPD |
|-----------|-----|-----|-----|
| Add/Sub | Nibble-wise + DAA (Decimal Adjust) | Binary add, check < 10^n | Decode→BID→add→encode |
| Mul/Div | Convert to BID, compute, convert back | Binary multiply/divide | Decode→BID→compute→encode |
| Compare | Digit-by-digit from MSD | Binary compare | Decode→BID→compare |
| Digit access | Extract nibble `_block[4i:4i+3]` | `(value / 10^i) % 10` | Decode relevant declet |

**BCD Decimal Adjust after Addition (DAA)**:
```cpp
void decimal_adjust_add() {
    unsigned carry = 0;
    for (unsigned i = 0; i < ndigits; ++i) {
        unsigned nibble = extract_nibble(i) + carry;
        if (nibble > 9) { nibble += 6; carry = 1; nibble &= 0xF; }
        else { carry = 0; }
        set_nibble(i, nibble);
    }
}
```

**DPD codec**: Include from `<universal/number/dfloat/dpd_codec.hpp>` (keep codec in dfloat, cross-include).

### Step 3: dfixpnt Core Implementation

**Template and storage**:
```cpp
template<unsigned _ndigits, unsigned _radix,
         DecimalEncoding _encoding = DecimalEncoding::BCD,
         bool _arithmetic = Modulo, typename bt = uint8_t>
class dfixpnt {
    static constexpr unsigned ndigits = _ndigits;
    static constexpr unsigned radix   = _radix;    // fractional digits
    static constexpr unsigned idigits = ndigits - radix;  // integer digits
    // ...
    bool _sign;                                    // sign-magnitude
    blockdecimal<ndigits, _encoding, bt> _block;   // unsigned magnitude
};
```

**Value**: `(-1)^sign × _block × 10^(-radix)`

Example: `dfixpnt<8, 3>` → 8 digits, 3 fractional → range ±99999.999

**Constructors**: Trivial default, SpecificValue, native types (int, float, double).

**Arithmetic dispatch**: `if constexpr (arithmetic == Modulo)` at each operator.

**Addition** (`operator+=`): No radix alignment needed (both operands have same radix). Same-sign: add magnitudes. Different-sign: subtract smaller from larger, adjust sign. Modulo: let blockdecimal overflow wrap. Saturate: use wider blockdecimal, clamp.

**Multiplication** (`operator*=`): Product has `2*radix` fractional digits. Convert both to wide BID, multiply, divide by `10^radix` to rescale, convert back to encoding.

**Division** (`operator/=`): Upscale dividend by `10^radix` (multiply in BID), then divide by rhs magnitude, result is in correct scale.

**String I/O**: Insert decimal point at position `radix` from right. Parse: split on '.', combine integer + fractional digits.

### Step 4: Supporting Files

Follow the dfloat/fixpnt pattern exactly:

- **`exceptions.hpp`**: `dfixpnt_arithmetic_exception`, `dfixpnt_divide_by_zero`, `dfixpnt_overflow`
- **`numeric_limits.hpp`**: `radix = 10`, `is_exact = true`, `is_modulo = arithmetic`
- **`attributes.hpp`**: `dfixpnt_range()`, `sign()`, `scale()`
- **`manipulators.hpp`**: `type_tag()`, `to_binary()`, `color_print()`
- **`dfixpnt_traits.hpp`**: `is_dfixpnt_trait`, `is_dfixpnt`, `enable_if_dfixpnt`
- **`mathlib.hpp`**: `abs()`, `floor()`, `ceil()` (minimal initial set)

### Step 5: CMake Wiring

4 insertion points in root `CMakeLists.txt`:

1. **Option** (~line 196): `option(UNIVERSAL_BUILD_NUMBER_DFIXPNTS "..." OFF)`
2. **BUILD_ALL cascade** (~line 832): `set(UNIVERSAL_BUILD_NUMBER_DFIXPNTS ON)`
3. **CI_LITE cascade** (~line 764): `set(UNIVERSAL_BUILD_NUMBER_DFIXPNTS ON)`
4. **add_subdirectory** (~line 971): `if(UNIVERSAL_BUILD_NUMBER_DFIXPNTS) add_subdirectory("static/fixpnt/decimal") endif()`

Remove `add_subdirectory("decimal")` from `static/fixpnt/CMakeLists.txt` (decouple from binary fixpnt build flag).

Update `static/fixpnt/decimal/CMakeLists.txt`: Change prefix from `deci` to `dfixpnt`.

### Step 6: Test Population

Repurpose existing stubs in `static/fixpnt/decimal/`:

- **`api/api.cpp`**: Construction, `type_tag()`, `to_string()`, digit access, special values, stream I/O. Test all three encodings.
- **`conversion/assignment.cpp`**: From/to int, float, double. String parsing.
- **`arithmetic/addition.cpp`**: Same-sign, different-sign, overflow (Modulo wraps, Saturate clamps).
- **`logic/logic.cpp`**: All comparison operators, +0 == -0.

Add new files: `arithmetic/subtraction.cpp`, `arithmetic/multiplication.cpp`, `arithmetic/division.cpp`.

## Verification

1. **GCC**: `cmake -DUNIVERSAL_BUILD_NUMBER_DFIXPNTS=ON .. && make -j4 && ctest -R dfixpnt`
2. **Clang**: `CXX=clang++ cmake -DUNIVERSAL_BUILD_NUMBER_DFIXPNTS=ON .. && make -j4 && ctest -R dfixpnt`
3. **No regressions**: Build dfloat and fixpnt targets to verify shared infrastructure changes don't break them.
4. **Grep check**: `grep -r __SIZEOF_INT128__ include/sw/universal/internal/blockdecimal/` → no matches.
5. **Triviality**: `static_assert(std::is_trivially_constructible_v<dfixpnt<8,3>>)` in api test.

## Phasing Strategy

Start with **BCD encoding only** for the initial implementation (it's the most natural for decimal fixed-point and the recommended default). Get the full vertical slice working (blockdecimal BCD → dfixpnt → tests). Then add BID and DPD as incremental follow-ups — the `if constexpr (encoding == ...)` dispatch makes this clean.

