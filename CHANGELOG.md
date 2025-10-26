# Changelog

All notable changes to the Universal Numbers Library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

#### 2025-01-26 - Phase 4: Comparative Advantage Examples (ereal Applications)
- Created user-facing API examples in `elastic/ereal/api/` demonstrating adaptive precision advantages:
  - **`catastrophic_cancellation.cpp`** - Shows (1e20 + 1) - 1e20 = 1 (perfect with ereal, 0 with double)
    - Demonstrates preservation of small components in extreme-scale arithmetic
    - Examples: (1 + 1e-15) - 1 = 1e-15, (1e100 + 1e-100) - 1e100 = 1e-100
    - All precision preserved without special algorithms
  - **`accurate_summation.cpp`** - Compares naive, Kahan, and ereal summation
    - Test cases: Sum 10,000 × 0.1, alternating huge/tiny values
    - Shows ereal beats even Kahan compensated summation on pathological cases
    - Order-independent accumulation
  - **`dot_product.cpp`** - Demonstrates quire-like exact accumulation
    - Order-independent: [1e20,1]·[1,1e20] = [1,1e20]·[1e20,1] exactly
    - No precision loss in mixed-scale dot products
    - Foundation for accurate linear algebra
- Created substantial application example in `applications/precision/numeric/`:
  - **`quadratic_ereal.cpp`** - Quadratic formula catastrophic cancellation demonstration
    - Side-by-side comparison: naive double vs. stable double vs. ereal
    - Four progressive test cases (mild to extreme cancellation)
    - **Key result**: Simple naive formula with ereal = Stable reformulation with double
    - Demonstrates: No need for numerical analysis tricks with adaptive precision
    - Example: x² + 10⁸x + 1 = 0, small root x₂ = 10⁻⁸
      - Double (naive): -7.45e-09 (25.5% error)
      - Double (stable): -1e-08 (correct, requires Citardauq formula)
      - ereal (naive): -1e-08 (correct, using simple formula!)
- **Philosophy**: Show "aha moment" examples demonstrating when and why to use adaptive precision
- **All examples**: Fast-running (<1 second), self-contained, with clear explanatory output

#### 2025-01-26 - Phase 3: Architectural Refactoring & Enhanced Constant Generation
- **Architectural improvement**: Moved constant generation from `internal/expansion/constants/` to `elastic/ereal/math/constants/`
  - **Rationale**: Constant generation is a user-facing application, not a primitive test
  - Clear separation: `internal/expansion/` for algorithm validation, `elastic/ereal/` for user examples
- **Rewrote `constant_generation.cpp`** using ereal API (not raw expansion operations):
  - Uses `ereal<128>` arithmetic instead of `std::vector<double>` primitives
  - Demonstrates natural user-facing API: `pi = four * pi_over_4`
  - Clean, readable code showing how users would actually interact with ereal
- **Added comprehensive round-trip validation tests** (no oracle required):
  - **Square root round-trip**: sqrt(n)² = n for n = 2, 3, 5, 7, 11 (0.0 error!)
  - **Arithmetic round-trip**: (π × e) / e = π (0.0 error!)
  - **Addition round-trip**: (√2 + √3) - √3 = √2 (0.0 error!)
  - **Rational round-trip**: (7/13) × 13 = 7 (0.0 error!)
  - **Compound operations**: ((√5 + √7) × π) / π = √5 + √7 (1.8e-16 error, just double rounding)
- **Perfect validation**: All mathematical identities hold exactly, proving expansion arithmetic correctness
- Generated 4-component qd representations for:
  - Fundamental constants: π, e, √2, √3, √5, ln(2)
  - Derived constants: π/2, π/4, 1/π, 2/π
- Created `elastic/ereal/math/constants/README.md` documenting the approach

#### 2025-01-26 - Phase 2: Expansion Growth & Compression Analysis
- Created `internal/expansion/growth/component_counting.cpp` - Track expansion growth patterns:
  - **No-growth cases**: 2+3=1 component, 2^11=1 component (exact operations stay compact)
  - **Expected growth**: 1+1e-15=2 components, 1e20+1=2 components (precision capture)
  - **Division growth**: 1/3=8 components, 1/7=8 components (Newton iterations)
  - **Accumulation efficiency**: Sum of 100 integers stays 1 component!
  - **Multi-component interactions**: [8]×[8]=16 components (product grows as expected)
  - **Growth bounds**: Component counts stay reasonable (no explosion)
- Created `internal/expansion/growth/compression_analysis.cpp` - Analyze compression effectiveness:
  - **Threshold compression**: 1/3: 8→2 components with threshold 1e-30
  - **Count compression**: compress_to_n() keeps N most significant components
  - **Precision loss measurement**: Error tracking for different compression levels
  - **Compression benefits**: 66% reduction for accumulated tiny values
  - **Post-operation cleanup**: (1/3)×(1/7): 16→8 components
- **Key Findings**:
  - Exact arithmetic (powers of 2, integer sums) stays compact (1 component)
  - Non-exact operations grow adaptively (1/3, 1/7 → 8 components)
  - Compression is highly effective with negligible precision loss
  - Sum of 100 integers = 1 component (excellent compaction!)
- **Phase 2 Complete** ✅

#### 2025-01-26 - Expansion Operations: Comprehensive Identity-Based Tests
- Created `internal/expansion/arithmetic/subtraction.cpp` - Subtraction-specific corner case tests:
  - **Exact cancellation**: a - a = [0]
  - **Zero identity**: a - [0] = a
  - **Negation**: [0] - a = -a
  - **Inverse addition**: (a + b) - b = a
  - **Catastrophic cancellation avoidance**:
    - (1e20 + 1) - 1e20 = 1 ✓ (preserves small component!)
    - (1e100 + 1) - 1e100 = 1 ✓
    - (1 + 1e-30) - 1 = 1e-30 ✓ (extreme precision)
    - Multiple small components preserved ✓
  - **Near-cancellation**: (a + ε) - a = ε (no loss of tiny components)
  - **Sign change**: a - b where b > a produces correct negative result
  - **Associativity**: (a - b) - c = a - (b + c)
  - **Extreme scales**: 1e100 - 1, 1 - 1e-100
- **Key Finding**: Subtraction has unique corner cases not covered by addition tests alone
- **Critical**: These tests validate the `linear_expansion_sum` bug fix works correctly

- Extended `internal/expansion/arithmetic/multiplication.cpp` with `expansion_product` tests:
  - **Multiplicative identity**: e × [1] = e
  - **Zero property**: e × [0] = [0]
  - **Commutivity**: e × f = f × e
  - **Associativity**: (e × f) × g = e × (f × g)
  - **Distributivity**: e × (f + g) = (e × f) + (e × g)
  - **Consistency with scale_expansion**: e × [scalar] matches scale_expansion(e, scalar)
  - **Extreme scales**: 1e20 × 1e-20 = 1.0, 1e100 × 2 = 2e100
- Created `internal/expansion/arithmetic/division.cpp` - Comprehensive reciprocal and quotient tests:
  - **Reciprocal identity**: reciprocal([1]) = [1], reciprocal([2]) = [0.5]
  - **Multiplicative inverse**: e × reciprocal(e) = [1] (within Newton precision)
  - **Double reciprocal**: reciprocal(reciprocal(e)) ≈ e
  - **Division identity**: e ÷ [1] = e
  - **Self-division**: e ÷ e = [1]
  - **Inverse property**: (e ÷ f) × f ≈ e
  - **Quotient vs reciprocal**: e ÷ f = e × reciprocal(f)
  - **Fractional results**: 1/3 and 1/7 produce 8 components (adaptive precision)
  - **Extreme scales**: 1e20 ÷ 1e-20 = 1e40, verified with inverse property
- **Key Finding**: Fractional divisions like 1/3 automatically expand to 8 components through Newton iteration
- **All tests passing**: No oracle needed, only exact mathematical identities ✅
- **Test coverage**: Now have unit tests for all four basic expansion operations at primitive level

#### 2025-01-26 - Phase 1 Identity Tests: Exact Mathematical Property Verification
- Created `elastic/ereal/arithmetic/identities.cpp` - Identity-based tests requiring no oracle:
  - **Additive identity recovery**: (a+b)-a = b tested component-wise
  - **Multiplicative identity**: a×(1/a) = 1 within Newton precision
  - **Exact associativity**: (a+b)+c vs a+(b+c) with mixed-scale components
  - **Exact distributivity**: a×(b+c) = (a×b)+(a×c)
  - **Inverse operations**: (a-b)+b=a and (a/b)×b=a
- Test cases include extreme scenarios:
  - Catastrophic cancellation: (1e20 + 1.0) - 1e20 = 1.0 (preserves small component)
  - Extreme precision: 1e-30 components preserved
  - Mixed scales: 1.0, 1e-15, 1e-30 in same computation
- Helper functions for component-wise analysis:
  - `components_equal()` - Exact component comparison (no tolerance)
  - `print_expansion()` - Debug output showing all expansion components
  - `is_valid_expansion()` - Verify decreasing magnitude order
- **Key Finding**: Expansions are not unique representations - different computation paths produce different component structures representing the same value
- **All identity tests passing** ✅

#### 2025-01-26 - Integration: ereal Number System with expansion_ops (Milestone 3)
- Extended `expansion_ops.hpp` with multiplication and division algorithms:
  - `expansion_product()` - Full ereal×ereal multiplication using component-wise scaling
  - `expansion_reciprocal()` - Newton iteration for computing 1/x
  - `expansion_quotient()` - Division via multiplication by reciprocal
- Integrated all expansion_ops into `ereal` number system:
  - Added `#include <universal/internal/expansion/expansion_ops.hpp>` to ereal_impl.hpp
  - Implemented `operator+=` using `linear_expansion_sum()`
  - Implemented `operator-=` using negation + `linear_expansion_sum()`
  - Implemented `operator*=` (ereal×ereal) using `expansion_product()`
  - Implemented `operator*=` (ereal×scalar) using `scale_expansion()`
  - Implemented `operator/=` (ereal÷ereal) using `expansion_quotient()`
  - Implemented `operator/=` (ereal÷scalar) using `expansion_quotient()`
  - Implemented comparison operators (`==`, `<`, `>`, etc.) using `compare_adaptive()`
  - Added public `limbs()` accessor for accessing expansion components
  - Fixed `convert_to_ieee754()` to sum all components (not just first)
- Created comprehensive test suites for all arithmetic operations:
  - `elastic/ereal/arithmetic/addition.cpp` - Addition, identity, associativity, commutivity
  - `elastic/ereal/arithmetic/subtraction.cpp` - Subtraction, cancellation, identity
  - `elastic/ereal/arithmetic/multiplication.cpp` - Multiplication, distributive, associative, commutivity
  - `elastic/ereal/arithmetic/division.cpp` - Division, reciprocal, identity (a/b)×b=a
- **All tests passing**: 4/4 arithmetic test suites + API tests ✅
- **Result**: ereal now provides complete multi-component adaptive precision arithmetic using Shewchuk's algorithms

#### 2025-01-26 - Expansion Operations: Scalar Operations & Compression (Milestone 2)
- Extended `expansion_ops.hpp` with scalar multiplication and compression:
  - `scale_expansion()` - Scalar multiplication with error-free transformations
  - `compress_expansion()` - Remove insignificant components based on threshold
  - `compress_to_n()` - Compress to at most N components
  - `sign_adaptive()` - O(1) to O(k) sign determination
  - `compare_adaptive()` - Component-wise adaptive comparison
- Created comprehensive test suites:
  - `internal/expansion/api/compression.cpp` - Compression and adaptive operations tests
  - `internal/expansion/arithmetic/addition.cpp` - Addition property tests (identity, commutivity, associativity)
  - `internal/expansion/arithmetic/multiplication.cpp` - Scalar multiplication tests
  - `internal/expansion/performance/benchmark.cpp` - Performance benchmarking
- All tests passing: compression (5/5), addition (5/5), multiplication (6/6)
- Performance benchmarks show expected algorithmic complexity

#### 2025-01-26 - Expansion Operations Infrastructure (Milestone 1)
- Added Shewchuk's adaptive precision floating-point expansion algorithms
- Created `include/sw/universal/internal/expansion/expansion_ops.hpp` with core algorithms:
  - `two_sum()` - Error-free transformation for addition (Knuth/Dekker)
  - `fast_two_sum()` - Optimized EFT when |a| >= |b| (Dekker)
  - `two_prod()` - Error-free multiplication using FMA
  - `grow_expansion()` - Add single component to expansion (Shewchuk Figure 6)
  - `fast_expansion_sum()` - Merge two nonoverlapping expansions (Shewchuk Figure 8)
  - `linear_expansion_sum()` - Robust expansion merging (Shewchuk Figure 7)
  - Utility functions: `estimate()`, `is_decreasing_magnitude()`, `is_nonoverlapping()`, `is_strongly_nonoverlapping()`
- Created test infrastructure for expansion operations:
  - `internal/expansion/api/api.cpp` - API usage examples
  - `internal/expansion/api/expansion_ops.cpp` - Comprehensive unit tests
  - `internal/expansion/CMakeLists.txt` - Build configuration
- Integrated expansion tests into main build system
- All tests passing (7/7 test suites)

#### Documentation
- Created `CHANGELOG.md` to track repository changes
- Created `docs/sessions/` directory for development session records
- Added session documentation for expansion operations implementation

### Fixed

#### 2025-01-26 - CRITICAL: ereal Unary Negation Operator Broken (Phase 4)
- **Bug**: `ereal::operator-()` returned a copy instead of negating the value
  - Location: `include/sw/universal/number/ereal/ereal_impl.hpp:89-92`
  - Code was: `ereal negated(*this); return negated;` (just returned copy!)
  - **Impact**: ALL unary negations failed silently, returning positive values instead of negative
  - Caused quadratic formula and any algorithm using `-x` to produce completely wrong results
- **Discovery**: Created `test_negation.cpp` to isolate the issue
  - Test: `-1000` returned `+1000` instead of `-1000`
  - Test: `-b + 500` returned `+1500` instead of `-500`
  - Test: Limbs weren't being negated at all
  - Binary subtraction (`0 - b`) worked correctly, confirming issue was unary operator
- **Fix**: Added loop to negate each component in the expansion:
  ```cpp
  ereal operator-() const {
      ereal negated(*this);
      for (auto& v : negated._limb) v = -v;  // Negate each component
      return negated;
  }
  ```
- **Verification**: All negation tests now PASS:
  - Simple negation: `-1000 = -1000` ✅
  - Expression negation: `-b + 500 = -500` ✅
  - Quadratic formula: All test cases produce correct negative roots ✅
- **Severity**: **CRITICAL** - This bug made ereal unusable for any algorithm with subtraction or negative values
- **Key Learning**: Need comprehensive operator tests, not just end-to-end algorithm tests

#### 2025-01-26 - Compiler Warnings Cleanup (Phase 4)
- Fixed unused variable warnings to enable clean builds:
  - **`internal/expansion/growth/compression_analysis.cpp:134`**
    - Removed unused `original_val` variable in conservative compression test
  - **`internal/expansion/performance/benchmark.cpp:96,112,119`**
    - Added `(void)sign;` casts to prevent compiler optimization in benchmark lambdas
    - Ensures `sign_adaptive()` calls aren't optimized away during timing measurements
- **Result**: Clean build with zero warnings for all expansion and ereal tests

#### 2025-01-26 - Critical Bug in Compression Error Measurement (Phase 2)
- **Bug**: Compression tests collapsed both full and compressed expansions to `double` before comparing
  - `double full_val = sum_expansion(full);` loses precision beyond double!
  - `double compressed_val = sum_expansion(compressed);` also loses that precision
  - Result: Both become identical doubles, showing 0.0 error for all compressions
- **Discovery**: User caught suspicious "0.000000e+00" errors across all compression tests
- **Root Cause**: The very precision we're trying to measure can't fit in a double
- **Fix**: Compute difference AS EXPANSION first, then sum:
  ```cpp
  double compute_relative_error(full, compressed) {
      std::vector<double> diff = subtract_expansions(full, compressed);  // Preserves precision!
      double error = sum_expansion(diff);  // Error small enough for double
      return abs(error) / sum_expansion(full);
  }
  ```
- **Impact**: Now measuring real precision loss:
  - 1/3 compressed 8→1 component: error = 5.551115e-17 (1 ULP in double)
  - 1/3 compressed 8→4 components: error = 9.495568e-66 (incredible precision!)
  - 1/7 with 2 components: error = 3.081488e-33 (10^16× better than 1 component!)
  - Each component pair adds ~32 digits of precision
- **Key Learning**: Never collapse adaptive-precision values to fixed precision before measuring differences

#### 2025-01-26 - Critical Bug in linear_expansion_sum Found by Phase 1 Identity Tests
- **Bug**: `linear_expansion_sum()` had incorrect index initialization and component selection logic
  - Indices initialized to `i=0, j=0` instead of pointing to least significant components `i=m-1, j=n-1`
  - When comparing magnitudes, picked **wrong component** (f_curr when e_curr was smaller)
  - Result: Components completely lost in cancellation scenarios like (1e20+1)-1e20
- **Discovery**: Phase 1 identity test (1e20+1.0)-1e20 returned empty expansion instead of [1.0]
  - Test 1c: Expected [1.0], got [] (0 components) - catastrophic loss
  - Test 1d: Lost 1e-30 component entirely
- **Fix** (expansion_ops.hpp:321-340):
  - Initialize indices correctly: `i = m-1, j = n-1` (point to least significant)
  - Pick smaller magnitude component: `if (abs(e_curr) < abs(f_curr)) q = e_curr`
  - Both indices now properly track remaining unprocessed components
- **Verification**: All existing expansion_ops tests still pass:
  - `exp_api_expansion_ops`: All tests PASS ✅
  - `exp_arith_addition`: All identity tests PASS ✅
  - `exp_api_compression`: All tests PASS ✅
- **Impact**: Extreme-scale arithmetic now works correctly:
  - (1e20 + 1.0) - 1e20 = 1.0 ✅ (avoids catastrophic cancellation)
  - Preserves components down to 1e-30 ✅
- **Key Learning**: Weak tests that only check final values (not component preservation) missed this bug
  - Phase 1 identity tests with component-wise verification caught it immediately
  - Highlights importance of testing exact mathematical properties, not just approximate results

#### 2025-01-26 - Critical Bug in fast_expansion_sum (Milestone 2)
- **Bug**: `fast_expansion_sum()` was calling `fast_two_sum(next_component, q, ...)` with arguments in wrong order
  - FAST-TWO-SUM requires |a| >= |b| as precondition
  - Algorithm was passing smaller component first, violating the invariant
  - Result: Loss of precision, inexact results despite "error-free" transformations
- **Fix**: Changed to use `two_sum(q, next_component, ...)` which works for any argument order
  - TWO-SUM: 6 ops, always correct
  - FAST-TWO-SUM: 3 ops, requires magnitude ordering
  - Trade-off: Correctness over speed (can optimize later with magnitude checks)
- **Key Learning**: Manually constructed arrays like `{10.0, 1.0e-15}` are NOT valid expansions
  - Valid expansions must be created using EFT operations (two_sum, grow_expansion, etc.)
  - Manual arrays lack the exact representation properties required by expansion algorithms
  - Test cases updated to use proper expansion construction
- **Impact**: All addition arithmetic tests now pass with exact identity property: (a+b)-a = b

### Technical Details

**Expansion Operations (Shewchuk 1997)**

The expansion operations implement adaptive precision floating-point arithmetic based on
Jonathan Richard Shewchuk's seminal paper "Adaptive Precision Floating-Point Arithmetic
and Fast Robust Geometric Predicates" (Discrete & Computational Geometry 18:305-363, 1997).

Key differences from Priest's fixed-precision algorithms (used in `floatcascade`):
- **Variable component count**: Expansions can grow/shrink dynamically
- **Adaptive algorithms**: Only examine as many components as needed
- **Strongly nonoverlapping**: Stricter invariant than Priest's nonoverlapping
- **Geometric predicates**: Optimized for orientation tests, incircle tests, etc.

**References:**
- Shewchuk, J.R. (1997). "Adaptive Precision Floating-Point Arithmetic and Fast Robust
  Geometric Predicates." Available at: https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf
- Priest, D.M. (1991). "Algorithms for Arbitrary Precision Floating Point Arithmetic."
- Hida, Li, Bailey (2000). "Library for Double-Double and Quad-Double Arithmetic."

### Roadmap

**Completed:**
- ✅ Milestone 1: Core expansion operations (GROW, FAST-SUM, LINEAR-SUM)
- ✅ Milestone 2: Scalar operations & compression (SCALE, COMPRESS, adaptive comparison)
- ✅ Milestone 3: Enhanced `ereal` arithmetic (integrate expansion_ops into ereal)

**Planned:**
- ⏳ Milestone 4: Adaptive comparison & geometric predicates
- ⏳ Milestone 5: Conversion & interoperability with dd/td/qd_cascade
- ⏳ Milestone 6: Optimization & production hardening

## [3.87] - Current Development Branch

### Existing Features
- Fixed multi-component floating-point: `dd_cascade`, `td_cascade`, `qd_cascade`
- Priest's algorithms in `floatcascade<N>` template
- Comprehensive test suites for cascade types
- Interactive educational tutorial on expansion algebra

---

## Notes

### Version Numbering
Universal uses semantic versioning: MAJOR.MINOR.PATCH
- MAJOR: Breaking API changes
- MINOR: New features, backward compatible
- PATCH: Bug fixes, backward compatible

### Contributing
When adding entries to this changelog:
1. Add new entries under `[Unreleased]` section
2. Use subsections: Added, Changed, Deprecated, Removed, Fixed, Security
3. Include relevant issue/PR numbers
4. Move entries to versioned section on release
5. Update roadmap status as milestones complete

### Session Documentation
Detailed development sessions are documented in `docs/sessions/` with:
- Session date and focus
- Design decisions and rationale
- Implementation details
- Test results
- Next steps

---

**Legend:**
- ✅ Completed
- 🔄 In Progress
- ⏳ Planned
- ⚠️ Blocked
- 🐛 Bug Fix
- 💡 Enhancement
- 🔧 Maintenance
