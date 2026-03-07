# Implement Flag-Honoring operator<<() for posit via blocktriple

## Context

The posit `operator<<()` (`posit_impl.hpp:1634`) currently converts to `long double` for output, which:
1. Loses precision for large posits (posit<64,3> has 58 fraction bits > long double's 63)
2. Fails for extreme regime values beyond long double range
3. Does NOT honor `std::fixed`, `std::scientific`, `std::showpos`, `std::uppercase`, `std::internal`, `std::left` flags

The user's architecture: implement `to_string()` on blocktriple (the internal sign+scale+significand marshalling type), then have posit's `operator<<()` convert to blocktriple and delegate. This avoids dividing posit values by 10 in the extreme regimes where no representable posit values exist.

The decimal digit extraction uses `support::decimal` (exact arbitrary-precision decimal arithmetic) to convert the blocktriple's binary (sign, scale, significand) triple directly to decimal digits. This avoids all floating-point intermediate conversions.

---

## Files to Modify

1. **`include/sw/universal/internal/blocktriple/blocktriple.hpp`** — Add `to_string()`, helpers, rewrite `operator<<()`
2. **`include/sw/universal/number/posit/posit_impl.hpp`** — Rewrite `operator<<()` and `to_string()` to delegate via blocktriple
3. **`static/tapered/posit/api/ostream_formatting.cpp`** — New test file
4. **`internal/blocktriple/api/ostream_formatting.cpp`** — New test file

## Reference Implementations

- `ereal_impl.hpp:390-539` — `to_string()` method (formatting pattern to follow)
- `ereal_impl.hpp:812-825` — `operator<<()` (flag extraction pattern)
- `cfloat_impl.hpp:3161-3231` — `to_decimal_fixpnt_string()` (uses `support::decimal` for exact binary→decimal)
- `support/decimal.hpp` — `support::decimal` class: `add()`, `mul()`, `powerOf2()`, `findMsd()`, `unpad()`

## Dependencies (already exist)

- `support/decimal.hpp` — Exact decimal arithmetic (vector<uint8_t> of digits)
- `blocktriple.hpp` — `_sign`, `_scale`, `_significand`, `_nan`, `_inf`, `_zero`, `at(i)`, `bfbits`, `radix`
- `posit_impl.hpp:979` — `to_value<BlockTripleOperator::REP>()` extracts exact (sign, scale, fraction) into blocktriple

---

## Changes

### Step 1: Add `#include <universal/number/support/decimal.hpp>` to blocktriple.hpp

**Location**: After line 18 (`#include <universal/internal/blocksignificand/blocksignificand.hpp>`), before trace_constants.

`decimal.hpp` only includes standard library headers — no circular dependency risk.

### Step 2: Add `to_string()` and helpers to blocktriple class

Add as public methods inside the `blocktriple` class (before the friend declarations at line ~931).

#### 2a: `power_of_five()` — private static helper

Computes `5^n` using repeated squaring with `support::decimal`:
```cpp
static support::decimal power_of_five(size_t n) {
    support::decimal result; result.setdigit(1);
    support::decimal base; base.setdigit(5);
    while (n > 0) {
        if (n & 1) support::mul(result, base);
        if (n > 1) support::mul(base, base);
        n >>= 1;
    }
    return result;
}
```

#### 2b: `append_exponent()` — private static helper

Formats exponent as `±NNN` (handles arbitrarily large exponents, unlike dd/ereal's 3-digit limit):
```cpp
static void append_exponent(std::string& str, long long e) {
    str += (e < 0 ? '-' : '+');
    e = (e < 0) ? -e : e;
    std::string digits;
    if (e == 0) { digits = "00"; }
    else { while (e > 0) { digits += char('0' + e % 10); e /= 10; } }
    while (digits.length() < 2) digits += '0';
    std::reverse(digits.begin(), digits.end());
    str += digits;
}
```

#### 2c: `to_string()` — public method

Signature (matches dd/ereal pattern):
```cpp
std::string to_string(std::streamsize precision = 7, std::streamsize width = 15,
    bool fixed = false, bool scientific = true, bool internal = false,
    bool left = false, bool showpos = false, bool uppercase = false,
    char fill = ' ') const
```

Algorithm:
1. `if (fixed && scientific) fixed = false;` — scientific takes precedence
2. **NaN**: `"nan"/"NAN"` (blocktriple uses sign bit for signaling: `"snan"/"SNAN"` if `_sign`)
3. **Sign**: `'-'` if `_sign`, `'+'` if `showpos`, for inf/zero/normal
4. **Inf**: append `"INF"/"inf"`
5. **Zero**: `"0."` + precision zeros + exponent if scientific
6. **Normal values** — exact binary→decimal via `support::decimal`:
   a. Walk `_significand` bits 0..bfbits-1, accumulate into `support::decimal sig` using `add(sig, bitWeight); add(bitWeight, bitWeight);`
   b. Compute `effExp = (long long)_scale - (long long)radix`
   c. If `effExp > 0`: `sig *= 2^effExp` via `powerOf2()` + `mul()`
   d. If `effExp < 0`: `sig *= 5^|effExp|` via `power_of_five()` + `mul()`. This gives exact digits with `|effExp|` implied fractional digits. (Because `sig * 2^(-e) = sig * 5^e / 10^e`)
   e. If `|effExp| > 100000`: fallback to `to_native<long double>()` via stringstream (extreme posit<256,5> edge case where `int _scale` already truncates)
   f. `sig.unpad()` — remove leading zeros
   g. Determine `sciExponent = (totalDigits - 1) - impliedFracDigits`
   h. Extract digits MSD-first from `sig` (index `size()-1` down to `0`)
   i. **Round** at precision boundary: examine digit beyond requested precision, round up if >= 5, propagate carry
   j. **Format**: scientific → `d.dddddde±EE`; fixed → `ddddd.ddddd`
7. **Width/fill/alignment**: internal (sign then fill then digits), left (trailing fill), right (leading fill)
   - Guard: `width > 0 && s.length() < size_t(width)`
   - Internal: check `s[0] == '-' || s[0] == '+'` for sign position

### Step 3: Rewrite blocktriple `operator<<()` (replace lines 958-982)

Extract all iostream flags, delegate to `to_string()`:
```cpp
template<unsigned fbits, BlockTripleOperator op, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const blocktriple<fbits, op, bt>& a) {
    std::ios_base::fmtflags fmt = ostr.flags();
    std::streamsize precision = ostr.precision();
    std::streamsize width = ostr.width();
    char fillChar = ostr.fill();
    bool showpos    = fmt & std::ios_base::showpos;
    bool uppercase  = fmt & std::ios_base::uppercase;
    bool fixed      = fmt & std::ios_base::fixed;
    bool scientific = fmt & std::ios_base::scientific;
    bool internal   = fmt & std::ios_base::internal;
    bool left       = fmt & std::ios_base::left;
    return ostr << a.to_string(precision, width, fixed, scientific,
                               internal, left, showpos, uppercase, fillChar);
}
```

### Step 4: Rewrite posit `operator<<()` (replace lines 1634-1653)

Handle NaR specially (posit-specific "nar" not "nan"), then delegate:
```cpp
template<unsigned nbits, unsigned es, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const posit<nbits, es, bt>& p) {
#if POSIT_ERROR_FREE_IO_FORMAT
    // ... existing hex format unchanged ...
#else
    // Extract all iostream flags
    std::ios_base::fmtflags fmt = ostr.flags();
    // ... extract precision, width, fill, showpos, uppercase, fixed, scientific, internal, left ...

    if (p.isnar()) {
        std::string s = uppercase ? "NAR" : "nar";
        // apply width/fill padding
        return ostr << s;
    }

    auto v = p.template to_value<BlockTripleOperator::REP>();
    return ostr << v.to_string(prec, width, bFixed, bScientific,
                               internal, left, showpos, uppercase, fillChar);
#endif
}
```

### Step 5: Rewrite posit `to_string()` (replace lines 1684-1691)

Delegate via blocktriple:
```cpp
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_string(const posit<nbits, es, bt>& p, std::streamsize precision = 17) {
    if (p.isnar()) return std::string("nar");
    auto v = p.template to_value<BlockTripleOperator::REP>();
    return v.to_string(precision, 0, false, true, false, false, false, false, ' ');
}
```

### Step 6: Add test files

**`static/tapered/posit/api/ostream_formatting.cpp`** — modeled on `elastic/ereal/api/ostream_formatting.cpp`:
- `capture()` helper with stream flags
- `test_default_output()` — 0, 1, -1, 3.14, -42.5
- `test_scientific_output()` — exponent notation
- `test_fixed_output()` — decimal point, sub-unit values, precision control
- `test_showpos()` — '+' prefix
- `test_uppercase()` — 'E' vs 'e'
- `test_width_alignment()` — right, left, internal with custom fill
- `test_nar_output()` — prints "nar"/"NAR", not "nan"
- `test_special_values()` — maxpos, minpos, NaR
- `test_multiple_sizes()` — posit<8,0>, posit<16,1>, posit<32,2>, posit<64,3>
- Inline demo blocks showing formatted output for visual inspection

**`internal/blocktriple/api/ostream_formatting.cpp`** — similar structure for blocktriple directly.

Both CMakeLists.txt use `file(GLOB API_SRC "./api/*.cpp")` — new files auto-discovered.

---

## Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| Exact `support::decimal` conversion, not `to_native<long double>()` | Avoids precision loss and range overflow for large posit configs |
| `5^n` trick for negative exponents (`sig * 2^-e = sig * 5^e / 10^e`) | Avoids decimal division entirely — integer multiplication only |
| Repeated squaring for `5^n` | O(log n) multiplications vs O(n) naive; critical for posit<64,3> where n~32698 |
| NaR handled in posit before blocktriple delegation | Posit NaR is distinct from IEEE NaN; must print "nar" not "nan" |
| `long long` for exponent arithmetic in `to_string()` locals | `int _scale` in blocktriple can reach ±2^31 for large posits |
| Fallback to long double for `|effExp| > 100000` | Prevents impractical `5^n` computation for extreme posit<256+> |
| `append_exponent()` handles arbitrary exponent size | dd/ereal version limited to 3 digits; posit<64,3> needs 5+ |

## Verification

1. Build blocktriple tests: `cd build_ci && cmake .. && cmake --build . --target bt_api_ostream_formatting -j4`
2. Run blocktriple test, verify PASS
3. Build posit tests: `cmake --build . --target posit_api_ostream_formatting -j4`
4. Run posit test, verify PASS
5. Build existing posit tests to check for regressions:
   ```bash
   cmake --build . --target posit_api_api posit_api_traits posit_api_manipulators -j4
   ```
6. Run each, verify no regressions
7. Build with clang (`build_ci_clang`) to verify portability
8. Spot-check: `std::fixed`, `std::scientific`, `std::setprecision`, `std::showpos`, `std::setw`, `std::left` produce correct output for posit<32,2> and posit<64,3>
