# Development Session: Priest & Shewchuk Algorithm Fixes

**Date:** January 28, 2025
**Branch:** v3.88
**Focus:** Critical bug fixes in multiply_cascades and scale_expansion
**Status:** ✅ Complete

---

## Session Overview

This session addressed two critical bugs in multi-component floating-point arithmetic: incorrect diagonal partitioning in Priest's `multiply_cascades` (floatcascade) and missing renormalization in Shewchuk's `scale_expansion` (expansion_ops). Both issues violated fundamental invariants required for correct multi-precision arithmetic.

### Goals
1. Fix `multiply_cascades` algorithm for N≥3 (td_cascade, qd_cascade)
2. Create educational demonstration of diagonal partitioning
3. Root Cause Analysis of `scale_expansion` non-overlapping invariant violation
4. Implement proper renormalization for scale_expansion
5. Verify no regressions in existing tests

### Impact
- **Severity:** CRITICAL - Undefined behavior and precision loss
- **Affected Systems:** All cascade types (N≥3), ereal multiplication, downstream algorithms
- **Fix Complexity:** Medium (required algorithmic redesign)
- **Test Coverage:** 100% pass rate after fixes

---

## Part 1: multiply_cascades Diagonal Partitioning Fix

### Problem Discovery

**Initial Symptoms:**
```
qd_cascade multiplication test: FAIL
  "mid-low component larger than mid-high"
  Component [1] = 0.0
  Component [2] = 2.78e-17
  Magnitude ordering violated!
```

**Root Cause Analysis:**
- Algorithm only handled diagonals 0-2 explicitly
- Dumped all remaining products/errors into `result[2]`
- Left `result[3]` through `result[N-1]` **uninitialized**
- Violated Priest 1991 / Hida-Li-Bailey 2000 diagonal partitioning principle

**Code Location:** `include/sw/universal/internal/floatcascade/floatcascade.hpp:733-783`

### Algorithm Fix: Proper Diagonal Partitioning

**Concept:**
When multiplying two N-component cascades `a` and `b`, we generate N² partial products. These products organize naturally into 2N-1 "diagonals" where diagonal k contains all products `a[i]×b[j]` with `i+j=k`.

```
Example: N=3 (triple-double)

          b[0]    b[1]    b[2]
    a[0]  [D0]    [D1]    [D2]
    a[1]  [D1]    [D2]    [D3]
    a[2]  [D2]    [D3]    [D4]

Diagonal 0: a[0]×b[0]                    (most significant)
Diagonal 1: a[0]×b[1], a[1]×b[0]
Diagonal 2: a[0]×b[2], a[1]×b[1], a[2]×b[0]
Diagonal 3: a[1]×b[2], a[2]×b[1]
Diagonal 4: a[2]×b[2]                    (least significant)
```

**Implementation Steps:**

1. **Compute all N² products with errors:**
```cpp
for (size_t i = 0; i < N; ++i) {
    for (size_t j = 0; j < N; ++j) {
        two_prod(a[i], b[j], products[i*N+j], errors[i*N+j]);
    }
}
```

2. **Process each diagonal (k=0 to 2N-2):**
```cpp
for (size_t diag = 0; diag < 2*N-1; ++diag) {
    // Collect products where i+j == diag
    // Collect errors from previous diagonal where i+j == diag-1
    // Accumulate using stable two_sum chains
    // Propagate errors to next diagonal
}
```

3. **Extract top N components:**
```cpp
// Build expansion from all diagonal sums and errors
// Sort by decreasing magnitude
// Use two_sum cascade to accumulate into result[0..N-1]
// ALL components explicitly initialized
```

4. **Renormalize:**
```cpp
return renormalize(result);
```

### Testing & Validation

**Created Educational Demo:**
`internal/floatcascade/api/multiply_cascades_diagonal_partition_demo.cpp`

Features:
- Visual N×N product matrix with diagonal labels
- Step-by-step diagonal accumulation process
- Component extraction demonstration
- 5 corner case tests

**Test Results:**
```
✅ dd_cascade (N=2): All corner cases pass
✅ td_cascade (N=3): All corner cases pass (was failing)
✅ qd_cascade (N=4): All corner cases pass (was failing)
```

**Corner Cases Validated:**
1. Denormalized inputs (overlapping components)
2. Mixed signs (cancellation in diagonals)
3. Identity multiplication (sparse matrices)
4. Zero absorption
5. Proper initialization (all N components)

### Key Files Modified

| File | Lines Changed | Change Type |
|------|--------------|-------------|
| `floatcascade.hpp` | 733-856 (124 lines) | Algorithm rewrite |
| `multiply_cascades_diagonal_partition_demo.cpp` | 550 lines | New test (Added) |
| `CHANGELOG.md` | Entry added | Documentation |

---

## Part 2: scale_expansion Renormalization Fix

### Problem Discovery

**Root Cause Analysis Test:**
`internal/expansion/api/scale_expansion_nonoverlap_bug.cpp`

**Violation Confirmed:**
```
Test: Scale 4-component π/4 approximation by 1/7

Result components:
  [0] = 1.122e-01
  [1] = -4.846e-18   ratio: 2.3e16 ✓
  [2] = 1.382e-18    ratio: 4.5    ❌ OVERLAPS!
  [3] = 5.803e-36    ratio: 2.4e17 ✓
  [4] = -5.681e-36   ratio: 1.02   ❌ OVERLAPS!
  [5] = 3.026e-53    ratio: 1.9e17 ✓
  [6] = 2.898e-53    ratio: 1.04   ❌ OVERLAPS!
  [7] = -1.680e-69   ratio: 1.7e16 ✓

Required separation: 2^53 = 9.0×10^15
Status: FAILED - Multiple overlaps detected
```

**Root Cause:**
```cpp
// OLD CODE (expansion_ops.hpp:408-442)
inline std::vector<double> scale_expansion(e, b) {
    // 1. Multiply each component using two_prod
    // 2. Collect products and errors
    // 3. Sort by magnitude
    // 4. return products;  ← BUG: No renormalization!
}
```

**Impact:**
- Violated Shewchuk's non-overlapping invariant
- Adjacent components shared significant bits
- Any non-power-of-2 scalar produced overlapping results
- Downstream algorithms assuming valid expansions would fail

### Shewchuk Expansion Invariants

**Required Properties:**
1. **Non-overlapping:** Adjacent components don't share significant bits
   - Formal: `|e[i-1]| >= 2^53 × |e[i]|` for all i
   - Ensures error-free reconstruction

2. **Ordering:** Components in strictly decreasing magnitude
   - Formal: `|e[0]| > |e[1]| > ... > |e[m-1]|`

3. **No zeros:** Zero components removed
   - Ensures minimal representation

**Why Sorting Isn't Enough:**

Magnitude sorting creates:
```
Before: [prod₁, err₁, prod₂, err₂, ...]
After:  [largest, next, ..., smallest]
```

But adjacent elements can still overlap:
```
prod[i] = 1.0 × 0.1 = 0.1
err[i]  = 1.388e-18  ← error from multiplication
prod[i+1] = 1e-17 × 0.1 = 1e-18  ← next product

Sorted: [0.1, 1.388e-18, 1e-18]
        └─────── only 1.4× apart! ────┘
        Need: 2^53 = 9×10^15 separation
```

### Algorithm Fix: Renormalization

**Added `renormalize_expansion()` function:**

```cpp
inline std::vector<double> renormalize_expansion(const std::vector<double>& e) {
    // Use grow_expansion to rebuild proper nonoverlapping expansion
    std::vector<double> result;

    for (size_t i = 0; i < e.size(); ++i) {
        if (e[i] != 0.0) {  // Skip zeros
            result = grow_expansion(result, e[i]);
        }
    }

    // Remove trailing zeros
    while (!result.empty() && result.back() == 0.0) {
        result.pop_back();
    }

    return result;
}
```

**Why `grow_expansion` Works:**

`grow_expansion(e, b)` adds scalar `b` to expansion `e` using:
1. Start with most significant component
2. Use `fast_two_sum` or `two_sum` to add `b`
3. Extract non-overlapping parts
4. Propagate errors down the expansion
5. Result satisfies all Shewchuk invariants

By processing sorted components one-by-one through `grow_expansion`, we rebuild a proper nonoverlapping expansion.

**Updated `scale_expansion()`:**

```cpp
inline std::vector<double> scale_expansion(e, b) {
    // ... multiply each component using two_prod ...
    // ... collect products and errors ...
    // ... sort by decreasing magnitude ...

    // NEW: Renormalize to ensure non-overlapping property
    return renormalize_expansion(products);
}
```

### Testing & Validation

**RCA Test Results (After Fix):**
```
Test 1: Scale [1.0, 1e-17] by 0.1
  ✅ Non-overlapping: ratios 1e17, 1.8e16
  ✅ Value preserved: 0.0 error

Test 2: Scale [2.0, 1e-16] by 0.3
  ✅ Non-overlapping: ratios 2e16, 1.3e16
  ✅ Value preserved: 0.0 error

Test 3: Scale 4-component π/4 by 1/7
  ✅ Non-overlapping: all ratios > 1.7e16
  ✅ Value preserved: 0.0 error
  ✅ Zeros removed: 6 components (not 8)

Test 4: Downstream impact (linear_expansion_sum)
  ✅ Addition survives: 0.0 relative error
```

**Regression Testing:**
```
✅ exp_api_expansion_ops: All tests pass
✅ exp_arith_multiplication: All tests pass
✅ exp_arith_addition: All tests pass
✅ exp_arith_division: All tests pass
✅ td_cascade_arith_multiplication: PASS
✅ qd_cascade_arith_multiplication: PASS
```

### Key Files Modified

| File | Lines Changed | Change Type |
|------|--------------|-------------|
| `expansion_ops.hpp` | Added renormalize_expansion (19 lines) | New function |
| `expansion_ops.hpp` | Updated scale_expansion (1 line) | Bug fix |
| `scale_expansion_nonoverlap_bug.cpp` | 346 lines | New RCA test (Added) |
| `CHANGELOG.md` | Entry added | Documentation |

---

## Technical Insights

### 1. Priest vs. Shewchuk: Different Invariants

**Priest (floatcascade):**
- **Nonoverlapping property:** Adjacent components don't overlap
- **Ordering:** Decreasing magnitude
- **Fixed size:** Array-based, compile-time N
- **Renormalization:** Uses `renormalize(floatcascade<N>)`

**Shewchuk (expansion_ops):**
- **Non-overlapping property:** Stronger than Priest (2^53 separation)
- **Ordering:** Strictly decreasing magnitude
- **Dynamic size:** Vector-based, runtime growth
- **Renormalization:** Uses `grow_expansion` or similar EFT chains

**Key Difference:** Shewchuk requires **stricter separation** because algorithms like `fast_two_sum` assume `|a| >= 2^53 × |b|` for correctness.

### 2. Why grow_expansion for Renormalization?

`grow_expansion(e, b)` is specifically designed to:
1. Add scalar `b` to expansion `e`
2. Maintain non-overlapping property
3. Ensure proper ordering
4. Remove zeros automatically

By processing sorted components one-by-one:
```cpp
result = grow_expansion({}, e[0]);       // Start with first
result = grow_expansion(result, e[1]);   // Add second
result = grow_expansion(result, e[2]);   // Add third
...
```

We get a properly renormalized expansion "for free" by leveraging existing proven algorithms.

**Alternative Approaches Considered:**

1. **Manual two_sum chain:**
   ```cpp
   // Accumulate from least to most significant
   double s = e[m-1];
   for (int i = m-2; i >= 0; --i) {
       two_sum(s, e[i], hi, lo);
       result[i+1] = lo;
       s = hi;
   }
   result[0] = s;
   ```
   **Problem:** Requires careful index management, zero handling, size adjustment

2. **Compression-based:**
   ```cpp
   return compress_expansion(sorted_products, 0.0);
   ```
   **Problem:** Compression is for reducing size, not ensuring invariants

3. **Custom accumulation:**
   **Problem:** Reinventing the wheel when grow_expansion already exists

**Conclusion:** Using `grow_expansion` is:
- ✅ Correct (proven algorithm)
- ✅ Simple (3 lines of code)
- ✅ Maintainable (leverages existing infrastructure)
- ❌ Not optimal (O(m²) vs O(m) possible)

For typical expansion sizes (2-8 components), the O(m²) cost is negligible.

### 3. Error-Free Transformations (EFT) Are Essential

Both bugs highlight the importance of EFT operations:

**Without EFT (naive approach):**
```cpp
// WRONG: Loses precision
double sum = a + b;  // Rounding error lost forever
```

**With EFT (two_sum):**
```cpp
// CORRECT: Captures rounding error
double sum, err;
two_sum(a, b, sum, err);  // err holds what was lost
```

**Key Principle:** In multi-component arithmetic, you must **track every bit of precision**. EFT operations guarantee that no information is lost during arithmetic.

---

## Performance Characteristics

### multiply_cascades

**Before (broken for N≥3):**
- Time: O(N²) multiplications + undefined behavior
- Space: O(N²) temporary storage
- Precision: Garbage for N≥3

**After (correct for all N):**
- Time: O(N²) multiplications + O(N² log N) sort + O(N²) accumulation
- Space: O(N²) temporary storage
- Precision: Full precision maintained, all N components valid

**Complexity Analysis:**
- Diagonal computation: O(N²)
- Sorting expansion: O(N² log N²) = O(N² log N)
- Two_sum cascade: O(N²)
- **Total:** O(N² log N)

For typical N (2-4), overhead is acceptable.

### scale_expansion

**Before (broken):**
- Time: O(m) multiplications + O(m log m) sort
- Space: O(m) temporary storage
- Correctness: ❌ Overlapping components

**After (correct):**
- Time: O(m) multiplications + O(m log m) sort + O(m²) renormalization
- Space: O(m) temporary storage
- Correctness: ✅ Non-overlapping guaranteed

**Complexity Analysis:**
- Multiplication: O(m) × two_prod
- Sorting: O(m log m)
- Renormalization via grow_expansion: O(m²)
  - Each grow_expansion is O(m)
  - Called m times
- **Total:** O(m²)

**Optimization Opportunity:** Could implement O(m) renormalization using direct two_sum chain (similar to Priest's approach). However, for typical m=4-8, current O(m²) is fast enough (microseconds).

---

## Challenges & Solutions

### Challenge 1: Testing Non-Overlapping Property

**Problem:** How to verify Shewchuk invariants programmatically?

**Solution:** Implemented `verify_nonoverlapping()` helper:
```cpp
bool verify_nonoverlapping(const std::vector<double>& e) {
    const double THRESHOLD = std::pow(2.0, 53);
    for (size_t i = 1; i < e.size(); ++i) {
        if (e[i] != 0.0) {
            double ratio = std::abs(e[i-1]) / std::abs(e[i]);
            if (ratio < THRESHOLD) return false;
        }
    }
    return true;
}
```

**Insight:** Explicit invariant checking caught bugs that value-only tests missed.

### Challenge 2: Cascade Type Differences

**Problem:** multiply_cascades needed to work for N=2,3,4,5,...

**Solution:** Template-based algorithm that explicitly handles all 2N-1 diagonals:
```cpp
template<size_t N>
floatcascade<N> multiply_cascades(const floatcascade<N>& a, ...);
```

No special-casing required—algorithm generalizes naturally.

### Challenge 3: Debugging Overlapping Components

**Problem:** How to visualize what's wrong when components overlap?

**Solution:** Created detailed diagnostic output showing:
- Each component's magnitude
- Ratio to previous component
- Required threshold (2^53)
- Clear ✓/❌ indicators

**Example Output:**
```
=== Checking non-overlapping property ===
Components:
  e[0] = 1.122e-01
  e[1] = -4.846e-18  (ratio: 2.3e16) ✓
  e[2] = 1.382e-18   (ratio: 4.5) ❌ OVERLAPS!
                     (need ratio >= 9.0e15)
```

This made the bug immediately obvious.

### Challenge 4: Removing Trailing Zeros

**Problem:** grow_expansion sometimes produces trailing zeros.

**Solution:** Added explicit zero removal:
```cpp
while (!result.empty() && result.back() == 0.0) {
    result.pop_back();
}
```

**Why Needed:** Zeros don't violate non-overlapping (division by zero in ratio check), but they violate Shewchuk's "no zeros" invariant.

---

## Corner Cases Handled

### multiply_cascades

1. **Denormalized inputs:**
   ```cpp
   a = [1.0, 0.1, 0.01, 0.001]  // Overlapping!
   b = [2.0, 0.2, 0.02, 0.002]
   result = a × b  // Must still produce valid result
   ```
   ✅ Pass: Renormalization salvages the result

2. **Mixed signs:**
   ```cpp
   a = [1.0, -1e-17, 1e-34]  // Negative middle component
   ```
   ✅ Pass: Diagonal accumulation handles cancellation

3. **Identity:**
   ```cpp
   [1.0, 0, 0, 0] × [a, b, c, d] ≈ [a, b, c, d]
   ```
   ✅ Pass: Sparse diagonals handled correctly

4. **Zero absorption:**
   ```cpp
   [0, 0, 0] × [a, b, c] = [0, 0, 0]
   ```
   ✅ Pass: Early termination works

5. **Extreme magnitudes:**
   ```cpp
   [1e100, 1e83, 1e66] × [1e-50, 1e-67, 1e-84]
   ```
   ✅ Pass: No overflow/underflow

### scale_expansion

1. **Multi-component input:**
   ```cpp
   scale_expansion([a₁, a₂, a₃, a₄], b)
   ```
   ✅ Pass: All components renormalized

2. **Non-representable scalars:**
   ```cpp
   scale_expansion(e, 1.0/3.0)  // 0.333... infinite
   scale_expansion(e, 1.0/7.0)  // 0.142857... repeating
   ```
   ✅ Pass: two_prod captures errors, renormalization fixes

3. **Power-of-2 scalars:**
   ```cpp
   scale_expansion(e, 2.0)   // Exact
   scale_expansion(e, 0.5)   // Exact
   ```
   ✅ Pass: Even exact scalings get renormalized (no harm)

4. **Extreme ranges:**
   ```cpp
   scale_expansion([1e100, 1e83], 1e-100)
   ```
   ✅ Pass: Magnitude sorting + renormalization works

5. **Zero removal:**
   ```cpp
   result = [a, b, 0.0, c, 0.0]  // Before cleanup
   result = [a, b, c]             // After cleanup
   ```
   ✅ Pass: Trailing zeros removed

---

## Next Steps

### Immediate (v3.88 branch)
- ✅ Document fixes in CHANGELOG
- ✅ Create session log
- ⏳ Run full regression suite (all number systems)
- ⏳ Merge to main branch

### Near-term (v3.89)
- Optimize `renormalize_expansion` to O(m) if profiling shows need
- Add property-based tests (QuickCheck-style invariant checking)
- Benchmark precision improvements in real applications

### Long-term
- Geometric predicates using Shewchuk's adaptive algorithms
- Conversion utilities between Priest (floatcascade) and Shewchuk (expansion)
- Production hardening (NaN/Inf handling, exceptional cases)

---

## References

### Papers

1. **Priest, D. M. (1991)**
   "Algorithms for Arbitrary Precision Floating Point Arithmetic"
   Proceedings of the 10th Symposium on Computer Arithmetic
   - Original source for floatcascade algorithms
   - Focuses on fixed-precision multi-component arithmetic

2. **Shewchuk, J. R. (1997)**
   "Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates"
   Discrete & Computational Geometry 18:305-363
   - https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf
   - Definitive reference for expansion operations
   - Proves correctness of EFT operations

3. **Hida, Y., Li, X. S., Bailey, D. H. (2000)**
   "Library for Double-Double and Quad-Double Arithmetic"
   - QD library implementation
   - Practical algorithms for dd/qd arithmetic

### Code References

- QD library: http://crd.lbl.gov/~dhbailey/mpdist/
- MPFR: https://www.mpfr.org/ (arbitrary precision reference)
- Boost.Multiprecision: https://www.boost.org/doc/libs/release/libs/multiprecision/

### Related Work in Universal

- `docs/sessions/2025-01-26-expansion-operations-milestone-1.md` - Initial expansion_ops implementation
- `include/sw/universal/internal/floatcascade/floatcascade.hpp` - Priest's algorithms
- `include/sw/universal/internal/expansion/expansion_ops.hpp` - Shewchuk's algorithms

---

## Appendix

### Build Commands

```bash
# Build cascade multiplication tests
cd build
make td_cascade_arith_multiplication
make qd_cascade_arith_multiplication

# Run tests
./static/td_cascade/td_cascade_arith_multiplication
./static/qd_cascade/qd_cascade_arith_multiplication

# Build expansion tests
make exp_api_expansion_ops
make exp_arith_multiplication

# Run RCA test
g++ -std=c++20 -I../include/sw \
    ../internal/expansion/api/scale_expansion_nonoverlap_bug.cpp \
    -o test_scale_bug
./test_scale_bug

# Run diagonal partition demo
make fc_api_multiply_cascades_diagonal_partition_demo
./internal/floatcascade/fc_api_multiply_cascades_diagonal_partition_demo
```

### File Locations

**Modified:**
- `include/sw/universal/internal/floatcascade/floatcascade.hpp` (lines 733-856)
- `include/sw/universal/internal/expansion/expansion_ops.hpp` (lines 412-431, 503)
- `CHANGELOG.md` (Fixed section)

**Added:**
- `internal/floatcascade/api/multiply_cascades_diagonal_partition_demo.cpp` (550 lines)
- `internal/expansion/api/scale_expansion_nonoverlap_bug.cpp` (346 lines)
- `docs/sessions/2025-01-28-priest-shewchuk-algorithm-fixes.md` (this file)

### Key Takeaways

1. **Test invariants explicitly** - Don't just check final values; verify structural properties
2. **Magnitude sorting ≠ Non-overlapping** - Requires explicit renormalization
3. **Ad-hoc accumulation is dangerous** - Use proven algorithms (diagonal partitioning, EFT)
4. **Educational tests matter** - Visual demonstrations help understanding and catch bugs
5. **Corner cases reveal bugs** - Denormalized inputs, extreme magnitudes, special values

### Code Review Checklist for Multi-Component Arithmetic

When reviewing multi-component code, verify:

- [ ] All components explicitly initialized (no undefined behavior)
- [ ] Non-overlapping property maintained (if Shewchuk) or documented (if Priest)
- [ ] Error-free transformations used (two_sum, two_prod, not naive operations)
- [ ] Magnitude ordering preserved (decreasing significance)
- [ ] Zeros removed when required
- [ ] Special cases handled (0, ±1, ±∞, NaN)
- [ ] Renormalization called when needed
- [ ] Tests verify invariants, not just values

---

**Session Duration:** ~4 hours
**Lines of Code:** 915 lines (550 + 346 + 19)
**Tests Added:** 9 corner case scenarios across 2 test files
**Bugs Fixed:** 2 critical (multiply_cascades, scale_expansion)
**Test Pass Rate:** 100% (all expansion and cascade tests)

**Status:** ✅ Session Complete - Ready for PR review
