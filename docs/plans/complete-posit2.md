# Plan: Complete posit2 Arithmetic Operations

## Context

The attention head benchmark revealed that `posit<16,1>` and `posit<8,0>` use 12 bytes per value (due to `std::bitset`-based `bitblock` storage in the original posit), when they should use 2 bytes and 1 byte respectively. The posit2 type was started to solve this — it uses `blockbinary<nbits, bt>` for limb-based storage (matching cfloat's pattern), giving `sizeof(posit2<16,1,uint16_t>) == 2`. However, posit2 was left incomplete: **addition works, but subtraction, multiplication, and division are stubbed out**.

Completing posit2 enables swapping it into the attention benchmark to demonstrate correct KV cache memory savings, and more broadly makes posit2 a viable drop-in replacement for posit throughout the library.

## Current State of posit2

**Working:**
- Storage: `blockbinary<nbits, bt, BinaryNumberType::Signed> _block` (line 1104 of posit_impl.hpp)
- Constructors: all native types (int, float, double, etc.) via `blocktriple<> → convert()`
- Assignment from native types: complete
- Conversion to native types: `operator float/double/int()` via `to_native<>()`
- Comparison operators: `== != < > <= >=` via `twosComplementLessThan`
- Addition (`operator+=`): works via `normalizeAddition → blocktriple::add → convert`
- Decode/encode: `decode_regime`, `extract_fields`, `decode`, `convert_`, `convert`
- I/O: `operator<<`, `operator>>`, `to_binary`, `to_string`, `hex_format`
- Selectors: `sign()`, `scale()`, `isnar()`, `iszero()`, `isone()`, etc.
- Modifiers: `clear()`, `setzero()`, `setnar()`, `maxpos()`, `minpos()`, etc.
- Increment/decrement: `++` and `--`

**Broken / Stubbed:**
- **Subtraction** (`operator-=`): special-case handling present, but arithmetic body is commented out (lines 704-723)
- **Multiplication** (`operator*=`): special-case handling present, but arithmetic body uses old `internal::value<>` path with calls commented out (lines 749-755)
- **Division** (`operator/=`): same — old `internal::value<>` path, calls commented out (lines 801-807)
- **abs()**: body commented out (line 907)
- **normalizeAddition**: has a hardcoded `FSU_MASK = 0x07FFu` (line 1069) that only works for `posit<16,2,uint16_t>` — not generic
- **normalizeMultiplication**: missing entirely
- **normalizeDivision**: missing entirely

## Approach

Follow the cfloat pattern exactly. cfloat's arithmetic cycle is:
1. `normalizeXxx()` — extract posit fields into a `blocktriple<fbits, BlockTripleOperator::XXX, bt>`
2. `blocktriple::add()/mul()/div()` — perform arithmetic
3. `convert(blocktriple, posit)` — encode result back

The `convert()` function already exists and works (it's what addition uses). We just need to:
1. Fix `normalizeAddition` to be generic (remove the hardcoded mask)
2. Write `normalizeMultiplication` and `normalizeDivision`
3. Rewrite `operator-=` to use blocktriple (negate + add, matching cfloat's pattern)
4. Rewrite `operator*=` to use blocktriple (`normalizeMultiplication → mul → convert`)
5. Rewrite `operator/=` to use blocktriple (`normalizeDivision → div → convert`)
6. Fix `abs()`
7. Add test files for subtraction, multiplication, division
8. Update the attention benchmark to use posit2

## File Changes

### 1. `include/sw/universal/number/posit2/posit_impl.hpp` (main work)

#### Fix `normalizeAddition` (line 1033-1079)
Replace the hardcoded `FSU_MASK = 0x07FFu` with generic extraction:
- Decode the posit to get the fraction bits
- Copy fraction bits into the blocktriple significant, add hidden bit
- Shift by `rbits` for rounding
- Pattern: decode `_block` → extract fraction → set into blocktriple with hidden bit

The proper approach (matching cfloat) is to decode the posit, extract the raw fraction bits, and set them into the blocktriple's significant. The current commented-out code at lines 1051-1066 had the right idea but referenced undefined `fraction_ull()` and `blockcopy()`. We'll implement a clean version using `extract_fields()` + block-level copy.

#### Add `normalizeMultiplication` (new method, after normalizeAddition)
```cpp
constexpr void normalizeMultiplication(blocktriple<fbits, BlockTripleOperator::MUL, bt>& tgt) const {
    // special cases: nar → nan, zero → zero
    // normal: decode fields, copy fraction into blocktriple, add hidden bit
    // no rounding shift needed for MUL (product.mul handles it)
}
```
Follow cfloat's pattern at cfloat_impl.hpp:2076-2132, but simplified:
- No subnormals/max-exponent values (posits don't have them)
- Always: extract sign, scale, fraction → setbits with hidden bit

#### Add `normalizeDivision` (new method)
```cpp
constexpr void normalizeDivision(blocktriple<fbits, BlockTripleOperator::DIV, bt>& tgt) const {
    // same as normalizeMultiplication but with divshift applied
}
```
Follow cfloat's pattern at cfloat_impl.hpp:2138-2192.

#### Fix `operator-=` (line 684-725)
Replace the stubbed body with:
```cpp
posit& operator-=(const posit& rhs) {
    return *this += (-rhs);
}
```
This is the simplest correct approach — negate and add — matching what cfloat does at line 563.

#### Fix `operator*=` (line 729-768)
Replace the old `internal::value` approach with blocktriple:
```cpp
blocktriple<fbits, BlockTripleOperator::MUL, bt> a, b, product;
normalizeMultiplication(a);
rhs.normalizeMultiplication(b);
product.mul(a, b);
convert(product, *this);
```

#### Fix `operator/=` (line 772-836)
Same pattern:
```cpp
blocktriple<fbits, BlockTripleOperator::DIV, bt> a, b, quotient;
normalizeDivision(a);
rhs.normalizeDivision(b);
quotient.div(a, b);
convert(quotient, *this);
```

#### Fix `abs()` (line 904-913)
```cpp
posit abs() const noexcept {
    posit p(*this);
    if (isneg()) p.setbits(twosComplement(_block));
    return p;
}
```

#### Fix `reciprocal()` (line 838-902)
The `setBitblock` call at line 856 needs to be replaced with `setbits()`:
```cpp
p.setbits(raw_bits);  // was: p.setBitblock(raw_bits)
```

### 2. Fix existing test + add new test files

Both posit and posit2 define `posit<nbits,es,bt>` in `sw::universal` — they cannot coexist in one TU. The existing posit2 addition test wrongly includes `posit/posit.hpp`. All posit2 tests must include `posit2/posit.hpp`.

#### Fix `static/posit2/arithmetic/addition.cpp`
Change `#include <universal/number/posit/posit.hpp>` → `#include <universal/number/posit2/posit.hpp>`

#### `static/posit2/arithmetic/subtraction.cpp` (new)
Follow the pattern from `static/posit/arithmetic/subtraction.cpp`, but include `posit2/posit.hpp`. Use `VerifySubtraction<posit<N,E>>`.

#### `static/posit2/arithmetic/multiplication.cpp` (new)
Follow `static/posit/arithmetic/multiplication.cpp`, but include `posit2/posit.hpp`. Use `VerifyMultiplication<posit<N,E>>`.

#### `static/posit2/arithmetic/division.cpp` (new)
Follow `static/posit/arithmetic/division.cpp`, but include `posit2/posit.hpp`. Use `VerifyDivision<posit<N,E>>`.

### 3. `applications/mixed-precision/attention/attention.cpp`

Replace `#include <universal/number/posit/posit.hpp>` with `#include <universal/number/posit2/posit.hpp>`. The posit<16,1> and posit<8,0> configs will now use posit2 with proper limb-based storage (2 bytes and 1 byte respectively), fixing the KV cache size reporting.

## Key Files

| File | Purpose |
|------|---------|
| `include/sw/universal/number/posit2/posit_impl.hpp` | Main implementation — all arithmetic fixes go here |
| `include/sw/universal/number/cfloat/cfloat_impl.hpp` | Reference for normalize/arithmetic patterns (lines 494-718, 2020-2192) |
| `include/sw/universal/internal/blocktriple/blocktriple.hpp` | `add()`, `mul()`, `div()` methods we delegate to |
| `include/sw/universal/number/posit2/positFraction.hpp` | Fraction field extraction |
| `include/sw/universal/number/posit2/positExponent.hpp` | Exponent field extraction |
| `include/sw/universal/number/posit2/positRegime.hpp` | Regime encoding/decoding |
| `static/posit2/arithmetic/addition.cpp` | Existing test (reference for new test patterns) |
| `applications/mixed-precision/attention/attention.cpp` | Benchmark to update |

## Verification

### Step 1: Build posit2 tests
```bash
cd build_attention
cmake -DUNIVERSAL_BUILD_NUMBER_POSIT2=ON ..
make -j4 posit2_addition  # verify existing test still passes
```

### Step 2: Run existing addition test
```bash
./static/posit2/posit2_addition
```
Should pass — confirms normalizeAddition fix didn't break anything.

### Step 3: Build and run new arithmetic tests
```bash
make -j4 posit2_subtraction posit2_multiplication posit2_division
./static/posit2/posit2_subtraction
./static/posit2/posit2_multiplication
./static/posit2/posit2_division
```
These exhaustively verify correctness for small posit sizes (2-8 bits).

### Step 4: Run updated attention benchmark
```bash
make -j4 attention_attention
./applications/mixed-precision/attention/attention_attention
```
Verify:
- posit2<16,1,uint16_t> shows KV cache = 49152 B (same as fp16, = 96 tokens × 128 × 2 × 2 bytes)
- posit2<8,0,uint8_t> shows KV cache = 24576 B (same as fp8, = 96 × 128 × 2 × 1 byte)
- Accuracy results are similar to original posit results
- Energy estimates now correctly show 16-bit and 8-bit energy costs for posit2

## Implementation Order

1. Fix `normalizeAddition` (generic mask) — verify existing addition test still passes
2. Implement `operator-=` via negate+add — simple, test with subtraction test
3. Implement `normalizeMultiplication` + fix `operator*=` — test with multiplication test
4. Implement `normalizeDivision` + fix `operator/=` — test with division test
5. Fix `abs()` and `reciprocal()`
6. Update attention benchmark to use posit2
