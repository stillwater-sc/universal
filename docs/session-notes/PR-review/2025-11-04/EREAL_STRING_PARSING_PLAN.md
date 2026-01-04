# Implementation Plan: String Parsing for ereal<maxlimbs>

## Executive Summary

Implement high-precision string parsing for `ereal<maxlimbs>` following the cascade pattern, enabling:
1. Parse arbitrary-precision decimal strings (e.g., "3.14159265358979323846264338327950288419716939937510...")
2. Maintain full precision up to ~15.95 * maxlimbs decimal digits
3. Support scientific notation (e.g., "1.23e-100")
4. Enable Option B redesign of progressive_precision test with true high-precision references

## Architecture Overview

### Design Pattern: Follow Cascade Implementation

The cascades use a proven architecture that we'll adapt for ereal:

```
String Input → parse() → Accumulate digits → Apply exponent → ereal<maxlimbs>
```

### Key Differences from Cascades

| Aspect | Cascades (floatcascade<N>) | ereal<maxlimbs> |
|--------|---------------------------|-----------------|
| Component count | Fixed at compile time (2/3/4) | Template parameter maxlimbs (4-19) |
| Precision | Fixed (~106/159/212 bits) | Adaptive (~64-304 bits) |
| Storage | Array of N doubles | Array of maxlimbs doubles |
| Algorithms | Compiler-specific volatile tricks | Shewchuk's expansion arithmetic |
| Normalization | renormalize() | Already uses Shewchuk algorithms |

### Advantage for ereal

ereal already has robust infrastructure:
- ✅ Error-free transformations (two_sum, quick_two_sum, split, two_prod)
- ✅ Non-overlapping expansion property maintained
- ✅ Adaptive precision based on maxlimbs template parameter
- ✅ All arithmetic operations preserve precision

**We just need to add the string parsing interface!**

## Implementation Plan

### Phase 1: Core Parsing Function

**File**: `/include/sw/universal/number/ereal/ereal_impl.hpp`

**Function signature**:
```cpp
template<unsigned maxlimbs>
bool ereal<maxlimbs>::parse(const std::string& str);
```

**Algorithm** (adapted from floatcascade.hpp:1364-1450):

```
1. INITIALIZATION
   - Set result = 0
   - Parse sign (+ or -)
   - Initialize decimal_point_position = -1

2. PARSE MANTISSA
   For each character c in string:
     If c is digit:
       result = result * 10 + digit   // Uses ereal multiplication and addition
       If after decimal point:
         decimal_point_position++
     Else if c is '.':
       Record decimal point seen
     Else if c is 'e' or 'E':
       Break to exponent parsing
     Else:
       Return false (parse error)

3. PARSE EXPONENT
   If 'e' or 'E' found:
     Parse exponent sign
     Parse exponent digits into integer exp

4. APPLY DECIMAL ADJUSTMENT
   If decimal point seen:
     exp -= decimal_point_position

5. APPLY EXPONENT
   If exp != 0:
     ereal<maxlimbs> ten(10.0);
     If exp > 0:
       result = result * pown(ten, exp)
     Else:
       result = result / pown(ten, -exp)

6. APPLY SIGN
   If negative:
     result = -result

7. ASSIGN TO *this
   *this = result
   return true
```

**Key Implementation Details**:

1. **No double contamination**: All operations use pure ereal arithmetic
2. **Digit accumulation**: `result = result * 10 + digit` uses ereal operators
3. **Exponent application**: Use existing `pown()` function for efficiency
4. **Error handling**: Return false on invalid input, leave *this unchanged

### Phase 2: Assignment Wrapper

**Function signature**:
```cpp
template<unsigned maxlimbs>
ereal<maxlimbs>& ereal<maxlimbs>::assign(const std::string& txt);
```

**Implementation** (mirrors cascade pattern):
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs>& ereal<maxlimbs>::assign(const std::string& txt) {
    ereal<maxlimbs> temp;
    if (temp.parse(txt)) {
        *this = temp;
    }
    // If parse fails, *this remains unchanged
    return *this;
}
```

### Phase 3: Free Function parse()

**File**: `/include/sw/universal/number/ereal/manipulators.hpp`

**Function signature**:
```cpp
template<unsigned maxlimbs>
inline bool parse(const std::string& txt, ereal<maxlimbs>& value);
```

**Implementation**:
```cpp
template<unsigned maxlimbs>
inline bool parse(const std::string& txt, ereal<maxlimbs>& value) {
    return value.parse(txt);
}
```

### Phase 4: String Constructor

**File**: `/include/sw/universal/number/ereal/ereal_impl.hpp`

**Function signature**:
```cpp
template<unsigned maxlimbs>
ereal<maxlimbs>::ereal(const std::string& str);
```

**Implementation**:
```cpp
template<unsigned maxlimbs>
inline ereal<maxlimbs>::ereal(const std::string& str) {
    if (!parse(str)) {
        // Parse failed - set to NaN or throw?
        // Option 1: Set to zero (silent failure)
        *this = ereal<maxlimbs>(0.0);
        // Option 2: Set to NaN (indicates error)
        // *this = ereal<maxlimbs>(std::numeric_limits<double>::quiet_NaN());
        // Option 3: Throw exception
        // throw std::invalid_argument("Failed to parse: " + str);
    }
}
```

**Design decision needed**: What should happen on parse failure?
- **Recommendation**: Follow cascade pattern - set to zero on failure

### Phase 5: Testing Strategy

**Test file**: `/elastic/ereal/api/api_string_parsing.cpp`

**Test cases**:

```cpp
// Basic integers
TEST("Parse integer", "123", 123.0);
TEST("Parse negative", "-456", -456.0);

// Decimals
TEST("Parse decimal", "3.14159", 3.14159);
TEST("Parse small decimal", "0.00001", 0.00001);

// Scientific notation
TEST("Parse sci positive", "1.23e10", 1.23e10);
TEST("Parse sci negative", "4.56e-20", 4.56e-20);

// High-precision (key test!)
TEST("Parse 100 digits",
     "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679",
     compare_with_mpfr_reference);

// Edge cases
TEST("Parse zero", "0.0", 0.0);
TEST("Parse negative zero", "-0.0", -0.0);
TEST("Parse very large", "1e308", 1e308);
TEST("Parse very small", "1e-308", 1e-308);

// Error cases
TEST("Parse invalid", "abc", false);  // Should return false
TEST("Parse empty", "", false);
TEST("Parse multiple dots", "1.2.3", false);
TEST("Parse multiple e", "1e2e3", false);
```

**Progressive precision test**:
```cpp
// Parse high-precision π
std::string pi_200_digits =
    "3.1415926535897932384626433832795028841971693993751..."
    "05820974944592307816406286208998628034825342117067982148086513282306647";

ereal<4> pi_4(pi_200_digits);   // Should use first ~64 bits
ereal<8> pi_8(pi_200_digits);   // Should use first ~128 bits
ereal<16> pi_16(pi_200_digits); // Should use first ~256 bits

// Test that precision scales
// Each should differ from double only in bits beyond double precision
```

### Phase 6: Integration with progressive_precision.cpp

**Redesign approach**:

```cpp
// OLD: Use double-precision references
double ref_double = std::stod(reference);  // Limited to ~16 digits!
Real ref(ref_double);

// NEW: Use string constructor with full precision
Real ref(reference);  // Uses parse(), maintains all digits!
```

**Impact**:
- Can now compare ereal<8>, <12>, <16>, <19> against true high-precision references
- Will actually demonstrate precision scaling
- Test becomes meaningful for all maxlimbs values

## Implementation Order

### Sprint 1: Core Functionality (Highest Priority)
1. ✅ **Day 1**: Implement `ereal<maxlimbs>::parse()` member function
   - Handle mantissa parsing
   - Handle exponent parsing
   - Apply decimal adjustment
   - **Deliverable**: Basic parsing works for simple cases

2. ✅ **Day 2**: Implement supporting functions
   - Add `assign()` wrapper
   - Add free function `parse()`
   - Add string constructor
   - **Deliverable**: Complete string parsing API

3. ✅ **Day 3**: Write comprehensive tests
   - Create `api_string_parsing.cpp`
   - Test all cases listed above
   - **Deliverable**: All tests pass

### Sprint 2: Integration (Medium Priority)
4. ✅ **Day 4**: Update progressive_precision.cpp
   - Replace `std::stod()` with string constructor
   - Verify precision scaling now works
   - **Deliverable**: progressive_precision test demonstrates true scaling

5. ✅ **Day 5**: Update other tests that need string parsing
   - Check if other test files would benefit
   - Update documentation
   - **Deliverable**: All tests use new string parsing where appropriate

### Sprint 3: Polish (Lower Priority)
6. ✅ **Day 6**: Optimize performance
   - Profile parsing on long strings
   - Optimize digit accumulation if needed
   - **Deliverable**: Performance benchmarks

7. ✅ **Day 7**: Add convenience features
   - Operator>> for istream
   - Better error messages
   - **Deliverable**: Enhanced usability

## Technical Challenges and Solutions

### Challenge 1: Precision Loss During Accumulation

**Problem**: When parsing "123456789012345678901234567890", each `result * 10` operation must maintain precision.

**Solution**: ereal already handles this! The multiplication operator uses Shewchuk's algorithms:
```cpp
// This automatically maintains full precision:
result = result * ten;  // Uses ereal::operator*
result = result + digit; // Uses ereal::operator+
```

### Challenge 2: Large Exponents

**Problem**: Parsing "1e308" or "1e-308" requires computing 10^308.

**Solution**: Use existing `pown()` function:
```cpp
ereal<maxlimbs> ten(10.0);
ereal<maxlimbs> power_of_ten = pown(ten, abs_exp);  // O(log n) multiplications
```

**Note**: Check if `pown()` exists for ereal. If not, may need to implement or use repeated multiplication.

### Challenge 3: String Parsing Performance

**Problem**: Parsing 1000-digit strings could be slow if each digit requires full ereal operations.

**Solution**:
- **Phase 1**: Implement straightforward algorithm (correctness first)
- **Phase 2**: Optimize if profiling shows it's a bottleneck
- **Possible optimizations**:
  - Batch digits (accumulate 10 digits in a double, then add to ereal)
  - Use faster algorithms for very long strings

### Challenge 4: Error Handling

**Problem**: What if input is "1.2.3" or "abc"?

**Solution**: Follow cascade pattern:
```cpp
bool parse(const std::string& str) {
    // Validate as we go
    if (invalid_character || multiple_decimal_points) {
        return false;  // Don't modify *this
    }
    // Only assign if valid
    *this = result;
    return true;
}
```

## Expected Outcomes

### Immediate Benefits

1. **String constructor works**:
   ```cpp
   ereal<8> pi("3.14159265358979323846264338327950288419716939937510");
   // pi now contains all 50 digits of precision!
   ```

2. **progressive_precision test is meaningful**:
   ```cpp
   // Can now compare against true 100-digit references
   std::string ref = "0.4794255386042030002732879352155713880818033679...";
   Real reference(ref);  // Uses ALL digits, not just first 16!
   ```

3. **Easier to write high-precision tests**:
   ```cpp
   // No more manual component initialization!
   ereal<16> expected("1.414213562373095048801688724209698078569671875...");
   ```

### Long-term Impact

1. **Enables high-precision computing**: Users can easily input 100+ digit constants
2. **Better testing**: Can validate precision at all maxlimbs settings
3. **User-friendly API**: Matches expectations from other libraries (mpfr, boost::multiprecision)
4. **Marketing**: "Easily parse 100-digit strings into ereal!"

## Risk Assessment

### Low Risk
- ✅ **Algorithm is proven**: Cascades already use this approach successfully
- ✅ **ereal infrastructure ready**: All needed arithmetic operations exist
- ✅ **Clear specifications**: IEEE-754 string format is well-defined

### Medium Risk
- ⚠️ **Performance**: May need optimization for very long strings (>1000 digits)
  - **Mitigation**: Start with simple implementation, optimize if needed
- ⚠️ **Edge cases**: Exponent overflow (10^10000), underflow (10^-10000)
  - **Mitigation**: Add validation, return false on extreme exponents

### High Risk
- ❌ **None identified**

## Success Criteria

### Must Have (MVP)
1. ✅ Parse integers: "123" → ereal(123)
2. ✅ Parse decimals: "3.14" → ereal(3.14)
3. ✅ Parse scientific: "1e-10" → ereal(1e-10)
4. ✅ Parse 100+ digit strings without precision loss
5. ✅ Return false on invalid input

### Should Have
6. ✅ String constructor: `ereal<8> x("3.14")`
7. ✅ progressive_precision test demonstrates scaling
8. ✅ Comprehensive unit tests pass

### Nice to Have
9. ⭕ Performance: Parse 1000-digit string in < 1ms
10. ⭕ istream operator>>
11. ⭕ Detailed error messages (which character caused failure)

## Conclusion

This implementation follows a proven pattern (cascades), leverages existing infrastructure (ereal arithmetic), and solves a critical limitation (progressive_precision test). The plan is concrete, achievable, and delivers immediate value.

**Estimated effort**: 3-5 days for MVP, 7 days for full implementation with optimizations

**Recommendation**: START WITH PHASE 1 (Core Parsing Function) to validate the approach with a simple prototype.
