# ereal Mathlib Regression Test Refactoring Plan

**Date:** 2025-11-03
**Objective:** Refactor Phase 1-3 tests from MANUAL_TESTING to REGRESSION_LEVEL_1
**Status:** Planning
**Estimated Time:** 1-2 hours

---

## Universal Standard Pattern

Based on `dd_cascade/math/sqrt.cpp` and `posit/arithmetic/addition.cpp`:

### Structure:

```cpp
namespace sw { namespace universal {
    // Helper functions for test generation and verification
    template<typename T>
    void GenerateXxxTestCase(T value) { ... }

    template<typename T>
    int VerifyXxxFunction(bool reportTestCases) {
        int nrOfFailedTestCases = 0;
        // Test logic
        return nrOfFailedTestCases;
    }
}}

#define MANUAL_TESTING 0  // NOTE: Set to 0, not removed!

int main() {
    // Setup

#if MANUAL_TESTING
    // Clean, minimal manual testing - just function calls
    GenerateXxxTestCase(1.0);
    GenerateXxxTestCase(2.0);

    return EXIT_SUCCESS;  // ignore errors
#else

#if REGRESSION_LEVEL_1
    // Regression tests using Verify functions
    nrOfFailedTestCases += ReportTestResult(
        VerifyXxxFunction(reportTestCases),
        "test description",
        test_tag
    );
#endif

#if REGRESSION_LEVEL_2
    // Future: Extended tests
#endif

#if REGRESSION_LEVEL_3
    // Future: Precision-scaled tests
#endif

#if REGRESSION_LEVEL_4
    // Future: Stress tests
#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
```

---

## Files to Refactor

### Phase 1 & 2 Functions:

1. **truncate.cpp** - floor, ceil, trunc, round (4 functions)
2. **numerics.cpp** - copysign, frexp, ldexp (3 functions)
3. **fractional.cpp** - fmod, remainder (2 functions)
4. **minmax.cpp** - min, max (2 functions)
5. **classify.cpp** - fpclassify, isnan, isinf, etc. (6 functions)

### Phase 3 Functions:

6. **sqrt.cpp** - sqrt, cbrt (2 functions)
7. **hypot.cpp** - hypot 2D/3D (2 functions)

---

## Refactoring Strategy per File

### Example: truncate.cpp

**Current state:**
- MANUAL_TESTING = 1
- 13 tests in MANUAL_TESTING block
- Cluttered with std::cout statements

**Target state:**

```cpp
namespace sw { namespace universal {

    // Verify floor function
    template<typename Real>
    int VerifyFloor(bool reportTestCases) {
        int nrOfFailedTestCases = 0;

        // Test: floor(2.7) == 2.0
        Real x(2.7), expected(2.0);
        Real result = floor(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: floor(2.7) != 2.0\n";
            ++nrOfFailedTestCases;
        }

        // Test: floor(-2.3) == -3.0
        x = -2.3; expected = -3.0;
        result = floor(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: floor(-2.3) != -3.0\n";
            ++nrOfFailedTestCases;
        }

        // Test: floor(5.0) == 5.0 (integer)
        x = 5.0; expected = 5.0;
        result = floor(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: floor(5.0) != 5.0\n";
            ++nrOfFailedTestCases;
        }

        return nrOfFailedTestCases;
    }

    // Verify ceil function
    template<typename Real>
    int VerifyCeil(bool reportTestCases) {
        int nrOfFailedTestCases = 0;

        // Test: ceil(2.3) == 3.0
        Real x(2.3), expected(3.0);
        Real result = ceil(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: ceil(2.3) != 3.0\n";
            ++nrOfFailedTestCases;
        }

        // Test: ceil(-2.7) == -2.0
        x = -2.7; expected = -2.0;
        result = ceil(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: ceil(-2.7) != -2.0\n";
            ++nrOfFailedTestCases;
        }

        // Test: ceil(5.0) == 5.0 (integer)
        x = 5.0; expected = 5.0;
        result = ceil(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: ceil(5.0) != 5.0\n";
            ++nrOfFailedTestCases;
        }

        return nrOfFailedTestCases;
    }

    // Verify trunc function
    template<typename Real>
    int VerifyTrunc(bool reportTestCases) {
        int nrOfFailedTestCases = 0;

        // Test: trunc(2.7) == 2.0
        Real x(2.7), expected(2.0);
        Real result = trunc(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: trunc(2.7) != 2.0\n";
            ++nrOfFailedTestCases;
        }

        // Test: trunc(-2.7) == -2.0
        x = -2.7; expected = -2.0;
        result = trunc(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: trunc(-2.7) != -2.0\n";
            ++nrOfFailedTestCases;
        }

        return nrOfFailedTestCases;
    }

    // Verify round function
    template<typename Real>
    int VerifyRound(bool reportTestCases) {
        int nrOfFailedTestCases = 0;

        // Test: round(2.3) == 2.0
        Real x(2.3), expected(2.0);
        Real result = round(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: round(2.3) != 2.0\n";
            ++nrOfFailedTestCases;
        }

        // Test: round(2.5) == 3.0
        x = 2.5; expected = 3.0;
        result = round(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: round(2.5) != 3.0\n";
            ++nrOfFailedTestCases;
        }

        // Test: round(2.7) == 3.0
        x = 2.7; expected = 3.0;
        result = round(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: round(2.7) != 3.0\n";
            ++nrOfFailedTestCases;
        }

        // Test: round(-2.5) == -3.0
        x = -2.5; expected = -3.0;
        result = round(x);
        if (result != expected) {
            if (reportTestCases) std::cerr << "FAIL: round(-2.5) != -3.0\n";
            ++nrOfFailedTestCases;
        }

        return nrOfFailedTestCases;
    }

}} // namespace sw::universal

#define MANUAL_TESTING 0

int main() {
    using namespace sw::universal;

    std::string test_suite = "ereal mathlib truncate function validation";
    std::string test_tag = "truncate";
    bool reportTestCases = false;
    int nrOfFailedTestCases = 0;

    ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
    // Manual testing - just a few representative cases
    std::cout << "Manual testing of truncation functions:\n";
    std::cout << "floor(2.7) = " << double(floor(ereal<>(2.7))) << "\n";
    std::cout << "ceil(2.3) = " << double(ceil(ereal<>(2.3))) << "\n";
    std::cout << "trunc(2.7) = " << double(trunc(ereal<>(2.7))) << "\n";
    std::cout << "round(2.5) = " << double(round(ereal<>(2.5))) << "\n";

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
    // Phase 1 functions
    test_tag = "floor";
    nrOfFailedTestCases += ReportTestResult(
        VerifyFloor<ereal<>>(reportTestCases),
        "floor(ereal)",
        test_tag
    );

    test_tag = "ceil";
    nrOfFailedTestCases += ReportTestResult(
        VerifyCeil<ereal<>>(reportTestCases),
        "ceil(ereal)",
        test_tag
    );

    // Phase 2 functions
    test_tag = "trunc";
    nrOfFailedTestCases += ReportTestResult(
        VerifyTrunc<ereal<>>(reportTestCases),
        "trunc(ereal)",
        test_tag
    );

    test_tag = "round";
    nrOfFailedTestCases += ReportTestResult(
        VerifyRound<ereal<>>(reportTestCases),
        "round(ereal)",
        test_tag
    );
#endif

#if REGRESSION_LEVEL_2
    // Future: Extended precision tests
#endif

#if REGRESSION_LEVEL_3
    // Future: Multi-component precision validation
#endif

#if REGRESSION_LEVEL_4
    // Future: Stress tests
#endif

    ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
    return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
```

---

## Key Principles

### 1. Verify Functions

- Return `int nrOfFailedTestCases`
- Take `bool reportTestCases` parameter
- Only print on failure (if reportTestCases is true)
- Use concise error messages
- One function per math function being tested
- Template on Real type for reusability

### 2. MANUAL_TESTING Block

- **Minimal and clean** - just a few representative calls
- Use for quick visual verification during development
- Always return EXIT_SUCCESS
- No elaborate test logic here

### 3. REGRESSION_LEVEL_1 Block

- Use `ReportTestResult()` wrapper
- Update test_tag for each group
- Accumulate failures in nrOfFailedTestCases
- Clear descriptions

### 4. Code Organization

- Helper functions at top (in namespace)
- Guards and main() below
- Clean separation of concerns

---

## Special Cases

### sqrt.cpp - Precision Validation

For sqrt/cbrt, need to validate precision, not just equality:

```cpp
template<typename Real>
int VerifySqrt(bool reportTestCases) {
    int nrOfFailedTestCases = 0;

    // Exact values
    Real x(4.0), expected(2.0);
    Real result = sqrt(x);
    if (result != expected) {
        if (reportTestCases) std::cerr << "FAIL: sqrt(4.0) != 2.0\n";
        ++nrOfFailedTestCases;
    }

    // Precision test: (sqrt(2))^2 ≈ 2.0
    x = 2.0;
    result = sqrt(x);
    Real squared = result * result;
    Real error = squared - x;
    double error_mag = std::abs(double(error));
    if (error_mag >= 1e-15) {
        if (reportTestCases) {
            std::cerr << "FAIL: sqrt(2) precision: (sqrt(2))^2 - 2 = "
                      << error_mag << "\n";
        }
        ++nrOfFailedTestCases;
    }

    // Zero test
    x = 0.0;
    result = sqrt(x);
    if (!result.iszero()) {
        if (reportTestCases) std::cerr << "FAIL: sqrt(0.0) != 0.0\n";
        ++nrOfFailedTestCases;
    }

    return nrOfFailedTestCases;
}
```

### fractional.cpp - Property-based Tests

For fmod/remainder, test the mathematical properties:

```cpp
template<typename Real>
int VerifyFmod(bool reportTestCases) {
    int nrOfFailedTestCases = 0;

    // Property: x - fmod(x,y) is divisible by y
    Real x(5.3), y(2.0);
    Real result = fmod(x, y);
    Real n = trunc(x / y);
    Real expected = x - (n * y);

    if (result != expected) {
        if (reportTestCases) {
            std::cerr << "FAIL: fmod(5.3, 2.0) property violation\n";
        }
        ++nrOfFailedTestCases;
    }

    // Sign test: fmod has same sign as dividend
    Real pos_result = fmod(Real(5.3), Real(2.0));
    Real neg_result = fmod(Real(-5.3), Real(2.0));

    if (!pos_result.ispos() || !neg_result.isneg()) {
        if (reportTestCases) {
            std::cerr << "FAIL: fmod sign property\n";
        }
        ++nrOfFailedTestCases;
    }

    return nrOfFailedTestCases;
}
```

---

## Implementation Order

1. **truncate.cpp** - Simplest, 4 functions
2. **minmax.cpp** - Very simple, 2 functions
3. **numerics.cpp** - Medium complexity (frexp/ldexp roundtrip)
4. **fractional.cpp** - Property-based tests
5. **sqrt.cpp** - Precision validation
6. **hypot.cpp** - Uses sqrt
7. **classify.cpp** - Last (6 functions, mostly boolean)

---

## Validation

After refactoring each file:

1. **Compile**: `g++ -std=c++20 -I./include/sw -o /tmp/test file.cpp`
2. **Run**: Verify all tests PASS
3. **Check output**: Clean, no clutter
4. **MANUAL_TESTING=1**: Verify manual block still works

---

## Benefits

- ✅ CI-ready: Tests run automatically
- ✅ Clean code: Easy to read and maintain
- ✅ Reusable: Verify functions can be called from other tests
- ✅ Extensible: Easy to add REGRESSION_LEVEL_2/3/4 later
- ✅ Debuggable: Manual testing still available
- ✅ Standard: Follows Universal conventions

---

## Timeline

- Planning: 30 minutes ✅
- Implementation: 1-2 hours
- Validation: 30 minutes
- **Total: 2-3 hours**

---

**Status:** READY FOR REVIEW
