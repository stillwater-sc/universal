# SpecificValue Constructor Implementation for ereal Type

## Executive Summary

The `ereal` type is missing a constructor that accepts `SpecificValue` enum values. This constructor is required by `numeric_limits<ereal<maxLimbs>>` to construct special values (infinity, NaN, maxpos, minpos, etc.). The implementation is straightforward following the patterns established by `dd_cascade`, `td_cascade`, and `qd_cascade`.

## 1. SpecificValue Enum Definition

**File**: `/home/stillwater/dev/stillwater/clones/universal/include/sw/universal/number/shared/specific_value_encoding.hpp`

**Lines**: 12-14

```cpp
enum class SpecificValue {
    maxpos, minpos, zero, minneg, maxneg, infpos, infneg, qnan, snan, nar
};
```

**Enum Values**:
- `maxpos` - Maximum positive finite value
- `minpos` - Minimum positive normalized value (smallest positive non-zero)
- `zero` - Zero
- `minneg` - Minimum negative normalized value (largest negative non-zero)
- `maxneg` - Maximum negative finite value (most negative)
- `infpos` - Positive infinity
- `infneg` - Negative infinity
- `qnan` - Quiet NaN (non-signaling)
- `snan` - Signaling NaN
- `nar` - NaR (Not a Real) - approximated as qnan for types that don't have NaR

## 2. ereal Current Implementation

**Main File**: `/home/stillwater/dev/stillwater/clones/universal/include/sw/universal/number/ereal/ereal_impl.hpp`

**Current Status**:
- ereal already has `setnan()`, `setinf()`, and `setzero()` methods (lines 182-185)
- ereal has a `_limb` member that is a `std::vector<double>` (line 208)
- No `maxpos()`, `minpos()`, `maxneg()`, or `minneg()` methods exist
- No SpecificValue constructor exists

**Limb Structure**:
```cpp
std::vector<double> _limb;  // components of the real value
```

Each limb is a double (IEEE-754 double-precision) component. When multiple limbs are used, they form a multi-component representation using Shewchuk's expansion arithmetic.

**Existing Methods** (lines 182-185):
```cpp
void clear()                   noexcept { _limb.clear(); _limb.push_back(0.0); }
void setzero()                 noexcept { clear(); }
void setnan()                  noexcept { clear(); _limb[0] = std::numeric_limits<double>::quiet_NaN(); }
void setinf(bool sign = false) noexcept { clear(); _limb[0] = (sign ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity()); }
```

**Missing From ereal**:
- Constructor `ereal(const SpecificValue code)`
- Method `ereal& maxpos()`
- Method `ereal& minpos()`
- Method `ereal& maxneg()`
- Method `ereal& minneg()`
- Class constants: `MIN_EXP_NORMAL`, `MAX_EXP`, `MIN_EXP_SUBNORMAL`, `EXP_BIAS`

## 3. Reference Implementation: dd_cascade

**File**: `/home/stillwater/dev/stillwater/clones/universal/include/sw/universal/number/dd_cascade/dd_cascade_impl.hpp`

**Lines 82-114** - SpecificValue Constructor:
```cpp
constexpr dd_cascade(const SpecificValue code) noexcept : cascade{} {
    switch (code) {
    case SpecificValue::maxpos:
        maxpos();
        break;
    case SpecificValue::minpos:
        minpos();
        break;
    case SpecificValue::zero:
    default:
        zero();
        break;
    case SpecificValue::minneg:
        minneg();
        break;
    case SpecificValue::maxneg:
        maxneg();
        break;
    case SpecificValue::infpos:
        setinf(false);
        break;
    case SpecificValue::infneg:
        setinf(true);
        break;
    case SpecificValue::nar: // approximation as dds don't have a NaR
    case SpecificValue::qnan:
        setnan(NAN_TYPE_QUIET);
        break;
    case SpecificValue::snan:
        setnan(NAN_TYPE_SIGNALLING);
        break;
    }
}
```

**Lines 276-300** - Specific Value Methods:
```cpp
constexpr dd_cascade& maxpos() noexcept {
    cascade[0] = 1.7976931348623157e+308;
    cascade[1] = 9.9792015476735972e+291;
    return *this;
}
constexpr dd_cascade& minpos() noexcept {
    cascade[0] = std::numeric_limits<double>::min();
    cascade[1] = 0.0;
    return *this;
}
constexpr dd_cascade& zero() noexcept {
    clear();
    return *this;
}
constexpr dd_cascade& minneg() noexcept {
    cascade[0] = -std::numeric_limits<double>::min();
    cascade[1] = 0.0;
    return *this;
}
constexpr dd_cascade& maxneg() noexcept {
    cascade[0] = -1.7976931348623157e+308;
    cascade[1] = -9.9792015476735972e+291;
    return *this;
}
```

**Key Points**:
- dd_cascade has 2 components (hence "double-double")
- The highest component carries the main value
- Additional components carry error corrections
- All limbs must have the same sign for correct arithmetic

## 4. Reference Implementation: td_cascade (Triple-Double)

**File**: `/home/stillwater/dev/stillwater/clones/universal/include/sw/universal/number/td_cascade/td_cascade_impl.hpp`

**Lines 63-95** - SpecificValue Constructor (identical pattern to dd_cascade)

**Lines 272-298** - Specific Value Methods:
```cpp
constexpr td_cascade& maxpos() noexcept {
    cascade[0] = 1.7976931348623157e+308;
    cascade[1] = 1.9958403095347196e+292;
    cascade[2] = 1.9958403095347196e+292;
    return *this;
}
constexpr td_cascade& minpos() noexcept {
    cascade[0] = std::numeric_limits<double>::min();
    cascade[1] = cascade[2] = 0.0;
    return *this;
}
// ... zero(), minneg(), maxneg() similar pattern
```

**Class Constants** (lines 36-39):
```cpp
static constexpr int EXP_BIAS = ((1 << (es - 1u)) - 1l);
static constexpr int MAX_EXP = (es == 1) ? 1 : ((1 << es) - EXP_BIAS - 1);
static constexpr int MIN_EXP_NORMAL = 1 - EXP_BIAS;
static constexpr int MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits);
```

Where:
- `es = 11` (exponent bits, same as IEEE-754 double)
- `fbits = 159` (fraction bits for td_cascade: 3 * 53)
- `EXP_BIAS = 1023` (same as IEEE-754 double)
- `MAX_EXP = 1024`
- `MIN_EXP_NORMAL = -1022`

## 5. Reference Implementation: qd_cascade (Quad-Double)

**File**: `/home/stillwater/dev/stillwater/clones/universal/include/sw/universal/number/qd_cascade/qd_cascade_impl.hpp`

**Lines 290-322** - Specific Value Methods:
```cpp
constexpr qd_cascade& maxpos() noexcept {
    cascade[0] = 1.79769313486231570814527423731704357e+308;
    cascade[1] = 9.97920154767359795037289025843547926e+291;
    cascade[2] = 5.53956966280111259858119742279688267e+275;
    cascade[3] = 3.07507899888268538886654502482441665e+259;
    return *this;
}
constexpr qd_cascade& minpos() noexcept {
    cascade[0] = std::numeric_limits<double>::min();
    cascade[1] = 0.0;
    cascade[2] = 0.0;
    cascade[3] = 0.0;
    return *this;
}
// ... zero(), minneg(), maxneg() similar pattern
```

**Pattern Analysis**:
- For `maxpos()`: Each successive limb captures the next more-significant chunk of the maximum double value
- For `minpos()`: First limb is `numeric_limits<double>::min()`, rest are 0
- For `minneg()` and `maxneg()`: Mirror of minpos and maxpos with sign flip

## 6. numeric_limits Specialization for ereal

**File**: `/home/stillwater/dev/stillwater/clones/universal/include/sw/universal/number/ereal/numeric_limits.hpp`

**Lines 20-45**:
```cpp
static constexpr ErealType max() { // return maximum value
    return ErealType(sw::universal::SpecificValue::maxpos);
} 
static constexpr ErealType infinity() { // return positive infinity
    return ErealType(sw::universal::SpecificValue::infpos);
}
static constexpr ErealType quiet_NaN() { // return non-signaling NaN
    return ErealType(sw::universal::SpecificValue::qnan);
}
static constexpr ErealType signaling_NaN() { // return signaling NaN
    return ErealType(sw::universal::SpecificValue::snan);
}
```

**Issue**: These lines fail to compile because `ereal` has no constructor accepting `SpecificValue`.

**Missing Class Constants** in ereal (referenced at line 63):
```cpp
static constexpr int min_exponent = ErealType::MIN_EXP_NORMAL + 1;
static constexpr int max_exponent = ErealType::MAX_EXP;
```

## 7. Multi-Component Representation Mathematics

### Shewchuk's Expansion Arithmetic Requirements

From the ereal_impl.hpp comment block (lines 35-61):

The ereal type uses Shewchuk's expansion arithmetic which requires:
- All components are representable as NORMAL IEEE-754 double-precision values
- Each limb adds approximately 53 bits of precision (one double's mantissa)
- After n limbs, the smallest representable correction term is approximately 2^(-53n)
- This must remain >= DBL_MIN (2^-1022) to maintain the non-overlapping property

Mathematical limit:
```
2^(-53n) >= 2^(-1022)
-53n >= -1022
n <= 19.28
```

**Therefore**: `maxlimbs <= 19` for algorithmic correctness (enforced by static_assert at line 72)

### Composition of Multi-Component Values

For a cascade type with `maxlimbs` components:

**maxpos()**: Each component captures the next scale of the maximum double:
- Component 0: 1.7976931348623157e+308 (highest magnitude)
- Component 1: 9.9792015476735972e+291 (approximately 2^-53 * component[0])
- Component 2: 5.5395696628011126e+275 (approximately 2^-53 * component[1])
- ... etc

**minpos()**: Component 0 = std::numeric_limits<double>::min() = 2.2250738585072014e-308, rest = 0

**Zero**: All components = 0.0

**minneg()**: Negative minpos

**maxneg()**: Negative maxpos

### Why This Pattern Works

Each successive limb in the expansion represents a smaller magnitude error term. When we want maxpos, we want to accumulate all possible positive error terms. When we want minpos, we only need the primary term (the smallest normalized double).

## 8. Current CI Failure

**Error**:
```
error: no matching function for call to 'sw::universal::ereal<4>::ereal(sw::universal::SpecificValue)'
```

**Location**: `numeric_limits.hpp` line 38 attempting to call:
```cpp
return ErealType(sw::universal::SpecificValue::infpos);
```

## Recommendations for Implementation

### 1. Add Class Constants to ereal_impl.hpp (after line 68)

```cpp
// Exponent characteristics (same as IEEE-754 double)
static constexpr int EXP_BIAS = 1023;
static constexpr int MAX_EXP = 1024;
static constexpr int MIN_EXP_NORMAL = -1022;
static constexpr int MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - static_cast<int>(53 * maxlimbs);
```

### 2. Add Specific Value Methods to ereal_impl.hpp (after line 186, after setinf)

```cpp
ereal& maxpos() noexcept {
    clear();
    // For maximum value, we want all components to be positive and represent
    // successive scales of the maximum double value
    _limb[0] = 1.7976931348623157e+308;  // max double
    if (maxlimbs >= 2) _limb.push_back(9.9792015476735972e+291);
    if (maxlimbs >= 3) _limb.push_back(5.5395696628011126e+275);
    if (maxlimbs >= 4) _limb.push_back(3.0750789988826854e+259);
    // For additional limbs, continue the pattern with each being ~2^-53 of previous
    // However, we can use a simpler pattern: each subsequent limb is the error
    // correction term when the previous limbs are summed
    return *this;
}

ereal& minpos() noexcept {
    clear();
    _limb[0] = std::numeric_limits<double>::min();
    return *this;
}

ereal& minneg() noexcept {
    clear();
    _limb[0] = -std::numeric_limits<double>::min();
    return *this;
}

ereal& maxneg() noexcept {
    clear();
    _limb[0] = -1.7976931348623157e+308;
    if (maxlimbs >= 2) _limb.push_back(-9.9792015476735972e+291);
    if (maxlimbs >= 3) _limb.push_back(-5.5395696628011126e+275);
    if (maxlimbs >= 4) _limb.push_back(-3.0750789988826854e+259);
    return *this;
}
```

### 3. Add SpecificValue Constructor to ereal_impl.hpp (after line 100, after double constructor)

```cpp
ereal(const SpecificValue code) noexcept {
    switch (code) {
    case SpecificValue::maxpos:
        maxpos();
        break;
    case SpecificValue::minpos:
        minpos();
        break;
    case SpecificValue::zero:
    default:
        setzero();
        break;
    case SpecificValue::minneg:
        minneg();
        break;
    case SpecificValue::maxneg:
        maxneg();
        break;
    case SpecificValue::infpos:
        setinf(false);
        break;
    case SpecificValue::infneg:
        setinf(true);
        break;
    case SpecificValue::nar:  // approximation as erals don't have a NaR
    case SpecificValue::qnan:
        setnan();
        break;
    case SpecificValue::snan:
        // Note: C++ std::numeric_limits doesn't distinguish signaling vs quiet NaN
        // For ereal, we use quiet NaN (same as qnan)
        setnan();
        break;
    }
}
```

### 4. Add Forward Declaration in ereal_fwd.hpp (if needed)

The SpecificValue is already available through `#include <universal/number/shared/specific_value_encoding.hpp>` in ereal_impl.hpp, so no additional forward declarations are needed.

## Key Design Considerations

1. **Constexpr**: The constructor should NOT be constexpr because `_limb` is a `std::vector`, which cannot be modified in constexpr context. Unlike `dd_cascade` which uses a fixed `floatcascade<2>`, ereal uses a dynamic vector.

2. **Exception Safety**: The methods should be noexcept since they don't allocate dynamically (they use existing space).

3. **Sign Consistency**: All components must have the same sign for correct arithmetic. The implementation ensures this by negating all limbs together.

4. **Adaptive Precision**: For maxpos/maxneg with different maxlimbs values, we may need to compute additional terms or use a smarter algorithm. A simple approach is to pre-compute values for common maxlimbs (2, 4, 8, 16) and provide a general computation for others.

5. **Numeric Limits Integration**: The numeric_limits specialization will work once the SpecificValue constructor is added. The class constants (MIN_EXP_NORMAL, MAX_EXP) are essential for setting exponent ranges.

## Testing the Implementation

After implementation, test with:

```cpp
ereal<8> x(SpecificValue::maxpos);      // Should create maximum value
ereal<8> y(SpecificValue::minpos);      // Should create minimum positive
ereal<8> inf = numeric_limits<ereal<8>>::infinity();  // Should work
ereal<8> nan = numeric_limits<ereal<8>>::quiet_NaN(); // Should work
```
