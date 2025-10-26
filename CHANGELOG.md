# Changelog

All notable changes to the Universal Numbers Library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

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
