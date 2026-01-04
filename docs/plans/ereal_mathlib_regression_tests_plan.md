# ereal Mathlib Regression Tests Implementation Plan

**Date:** 2025-11-03
**Objective:** Create regression test structure for ereal mathlib (skeleton only, no functionality)
**Status:** Planning Phase
**Depends On:** Phase 0 complete (mathlib stub infrastructure)

---

## Executive Summary

Create skeleton regression test files for ereal mathlib that match the structure of dd_cascade/math tests. These will be empty/minimal skeletons that honor the MANUAL_TESTING and REGRESSION_LEVEL_* preprocessor structure and include proper exception handling blocks. The actual test functionality will be implemented later when we refine each math function for adaptive precision.

---

## Analysis of dd_cascade Test Structure

### Files in static/dd_cascade/math/

```
classify.cpp           (152 lines) - Classification functions
error_and_gamma.cpp    ( 81 lines) - erf, erfc, tgamma, lgamma
exponent.cpp           (105 lines) - exp, exp2, exp10, expm1
fractional.cpp         ( 80 lines) - fmod, remainder
hyperbolic.cpp         (101 lines) - sinh, cosh, tanh, asinh, acosh, atanh
hypot.cpp              ( 95 lines) - hypot
minmax.cpp             ( 75 lines) - min, max
next.cpp               ( 99 lines) - nextafter, nexttoward
pow.cpp                (388 lines) - pow, pown (most complex)
sqrt.cpp               (175 lines) - sqrt, cbrt
truncate.cpp           ( 81 lines) - floor, ceil, trunc, round
```

**Total:** 11 test files

### Common Structure Pattern

Every test file follows this exact pattern:

```cpp
// <function>.cpp: test suite runner for <function> for <type>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/<type>/<type>.hpp>
#include <universal/verification/test_suite.hpp>

// Optional: Helper functions for test case generation
// template<typename Ty>
// void GenerateTestCase(Ty fa) { ... }

// Regression testing guards
#define MANUAL_TESTING 0  // or 1 for development
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "<type> mathlib <function> function validation";
	std::string test_tag    = "<function_names>";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual testing code here
	// Usually simple examples for development/debugging

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	// Automated regression tests here
	// Organized by REGRESSION_LEVEL_1/2/3/4 if needed
	// (dd_cascade tests don't use levels yet, but structure is there)

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
```

### Key Elements

1. **Header Comment Block:** Copyright and MIT license
2. **Includes:** directives.hpp, ereal.hpp, test_suite.hpp
3. **Helper Functions (optional):** GenerateTestCase, Verify functions
4. **Regression Guards:** MANUAL_TESTING and REGRESSION_LEVEL_1/2/3/4
5. **main() with try:** Sets up test suite name and reporting
6. **#if MANUAL_TESTING:** Manual development/debug code (returns SUCCESS)
7. **#else:** Automated regression tests (returns based on failures)
8. **catch blocks:** 5 standard exception handlers (ad-hoc, arithmetic, internal, runtime, unknown)

---

## ereal-Specific Considerations

### Differences from dd_cascade

1. **Template Parameter:** ereal is `ereal<maxlimbs>` (template), dd_cascade is concrete type
2. **Precision Testing:** ereal will need tests for different precision levels (not just accuracy)
3. **Adaptive Behavior:** Tests should verify precision grows appropriately
4. **No numeric_limits yet:** ereal doesn't have numeric_limits specialization (commented out)

### Test Philosophy for ereal

**Phase 0 Tests (Now):**
- Skeleton structure only
- MANUAL_TESTING = 1 (development mode)
- Minimal smoke test: "call function, verify it returns something"
- Comment: "TODO Phase 1/2/3: Implement precision validation tests"

**Future Phases:**
- Phase 1: Add tests for simple functions with precision validation
- Phase 2: Add tests for transcendental functions at various precisions
- Phase 3: Add tests for trigonometric functions with argument reduction
- Phase 4: Add tests for precision control API

---

## Proposed Test Files for ereal

Create skeleton tests matching dd_cascade structure:

```
static/ereal/math/
├── classify.cpp           # fpclassify, isnan, isinf, isfinite, isnormal, signbit
├── error_and_gamma.cpp    # erf, erfc, tgamma, lgamma
├── exponent.cpp           # exp, exp2, exp10, expm1
├── fractional.cpp         # fmod, remainder
├── hyperbolic.cpp         # sinh, cosh, tanh, asinh, acosh, atanh
├── hypot.cpp              # hypot
├── logarithm.cpp          # log, log2, log10, log1p (NEW - dd doesn't have separate)
├── minmax.cpp             # min, max
├── next.cpp               # nextafter, nexttoward
├── numerics.cpp           # frexp, ldexp, copysign (NEW - dd doesn't have separate)
├── pow.cpp                # pow, pown
├── sqrt.cpp               # sqrt, cbrt
├── trigonometry.cpp       # sin, cos, tan, asin, acos, atan, atan2 (NEW - dd doesn't have)
└── truncate.cpp           # floor, ceil, trunc, round
```

**Total:** 14 test files (3 more than dd_cascade due to separate logarithm, numerics, trigonometry)

---

## Skeleton Test Template

Each test will follow this minimal pattern:

```cpp
// <function>.cpp: test suite runner for <function> for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib <function> function validation";
	std::string test_tag    = "<function_names>";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 0: Minimal smoke test
	// TODO Phase 1/2/3: Implement precision validation tests

	ereal<> x(2.0);
	ereal<> y(3.0);

	// Example: test that function is callable
	// std::cout << "<function>(" << x << ") = " << <function>(x) << '\n';

	std::cout << "Phase 0: stub infrastructure validation - PASS\n";
	std::cout << "TODO: Implement precision validation tests in future phases\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	// Phase 0: No automated tests yet
	// TODO Phase 1: Add REGRESSION_LEVEL_1 tests (basic functionality)
	// TODO Phase 2: Add REGRESSION_LEVEL_2 tests (extended coverage)
	// TODO Phase 3: Add REGRESSION_LEVEL_3 tests (comprehensive)
	// TODO Phase 4: Add REGRESSION_LEVEL_4 tests (stress testing)

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
```

---

## Implementation Strategy

### Approach: Template-Based Generation

Rather than manually writing 14 nearly-identical files, I will:

1. **Create a master template** with placeholders for:
   - Function name
   - Function list (for test_tag)
   - Specific smoke test calls

2. **Generate each test file** by substituting:
   - `<function>` → actual function name (e.g., "sqrt", "exponent")
   - `<function_names>` → list of functions tested (e.g., "sqrt/cbrt")
   - `<smoke_test_code>` → minimal function calls to verify compilation

3. **Customize per file:**
   - Each file will have appropriate function calls for that category
   - Comments will indicate what tests are planned for future phases

### Files to Create

| File | Functions Tested | Smoke Test Content |
|------|-----------------|-------------------|
| classify.cpp | fpclassify, isnan, isinf, isfinite, isnormal, signbit | Test each with ereal<>(2.0) |
| error_and_gamma.cpp | erf, erfc, tgamma, lgamma | Test each with ereal<>(2.0) |
| exponent.cpp | exp, exp2, exp10, expm1 | Test each with ereal<>(2.0) |
| fractional.cpp | fmod, remainder | Test with ereal<>(7.0), ereal<>(3.0) |
| hyperbolic.cpp | sinh, cosh, tanh, asinh, acosh, atanh | Test each with ereal<>(2.0) |
| hypot.cpp | hypot (2 and 3 arg) | Test hypot(x, y) and hypot(x, y, z) |
| logarithm.cpp | log, log2, log10, log1p | Test each with ereal<>(2.0) |
| minmax.cpp | min, max | Test with ereal<>(2.0), ereal<>(3.0) |
| next.cpp | nextafter, nexttoward | Test with ereal<>(2.0), ereal<>(3.0) |
| numerics.cpp | frexp, ldexp, copysign | Test each basic functionality |
| pow.cpp | pow, pown | Test pow(x,y), pown(x,n) |
| sqrt.cpp | sqrt, cbrt | Test with ereal<>(2.0), ereal<>(8.0) |
| trigonometry.cpp | sin, cos, tan, asin, acos, atan, atan2 | Test each with ereal<>(1.0) |
| truncate.cpp | floor, ceil, trunc, round | Test each with ereal<>(2.7) |

---

## Example: sqrt.cpp Skeleton

```cpp
// sqrt.cpp: test suite runner for sqrt/cbrt functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib sqrt/cbrt function validation";
	std::string test_tag    = "sqrt/cbrt";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Phase 0: Minimal smoke test - verify functions are callable
	// TODO Phase 2: Implement adaptive-precision sqrt using Newton-Raphson
	// TODO Phase 2: Implement adaptive-precision cbrt using specialized Newton iteration
	// TODO Phase 2: Add precision validation tests (request N bits, verify accuracy)
	// TODO Phase 3: Add range tests (DBL_MIN to DBL_MAX equivalents)
	// TODO Phase 3: Add special value tests (0, negative, very large/small)

	ereal<> x(2.0);
	ereal<> y(8.0);

	std::cout << "Testing sqrt...\n";
	std::cout << "sqrt(" << x << ") = " << sqrt(x) << '\n';

	std::cout << "\nTesting cbrt...\n";
	std::cout << "cbrt(" << y << ") = " << cbrt(y) << '\n';

	std::cout << "\nPhase 0: stub infrastructure validation - PASS\n";
	std::cout << "TODO: Implement precision validation tests in Phase 2\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	// Phase 0: No automated tests yet
	// TODO Phase 2: Add REGRESSION_LEVEL_1 tests (basic sqrt/cbrt at double precision)
	// TODO Phase 2: Add REGRESSION_LEVEL_2 tests (extended precision 100-200 bits)
	// TODO Phase 2: Add REGRESSION_LEVEL_3 tests (high precision 200-500 bits)
	// TODO Phase 2: Add REGRESSION_LEVEL_4 tests (extreme precision 500-1000 bits)

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
```

---

## Success Criteria

### Phase 0 Regression Skeleton Complete When:

1. ✅ All 14 test files created in `static/ereal/math/`
2. ✅ Each file has proper structure:
   - Copyright header
   - Correct includes
   - MANUAL_TESTING and REGRESSION_LEVEL_* macros
   - main() with try block
   - Minimal smoke test in MANUAL_TESTING section
   - Empty placeholder in automated test section
   - All 5 exception catch blocks
3. ✅ Each file compiles without errors
4. ✅ Each file runs and reports PASS
5. ✅ Each file clearly marked with TODO comments for future phases
6. ✅ Structure matches dd_cascade pattern exactly

---

## Benefits of This Approach

1. **CI Integration Ready:** Structure matches what CI expects
2. **Progressive Refinement:** Easy to add real tests incrementally
3. **Documentation:** TODOs document what needs to be implemented
4. **Consistency:** All tests follow same pattern
5. **Low Risk:** Minimal code, just structure
6. **Fast Implementation:** Template-based generation

---

## Future Test Development

When implementing real tests in Phases 1-3:

1. **Keep MANUAL_TESTING = 1** during development
2. **Add helper functions** above main() for test case generation
3. **Implement automated tests** in #else section
4. **Use REGRESSION_LEVEL_1/2/3/4** to organize by precision:
   - LEVEL_1: ~53 bits (double precision equivalent)
   - LEVEL_2: ~100-200 bits (quad/octuple precision)
   - LEVEL_3: ~200-500 bits (high precision)
   - LEVEL_4: ~500-1000 bits (extreme precision)
5. **Set MANUAL_TESTING = 0** when tests are stable

---

## Notes on ereal-Specific Testing

### Precision Validation Pattern

Future tests should validate precision, not just accuracy:

```cpp
// Example pattern for Phase 2+
template<unsigned maxlimbs>
int VerifyPrecision(const ereal<maxlimbs>& computed,
                    const ereal<maxlimbs>& reference,
                    unsigned expectedBits) {
    // Verify 'computed' matches 'reference' to 'expectedBits' of precision
    // This is different from fixed-precision tests which check equality
}
```

### Adaptive Precision Tests

Tests should verify precision grows appropriately:

```cpp
// Example pattern for Phase 4
void TestAdaptivePrecision() {
    // Verify that operations automatically increase precision as needed
    // Verify that precision control API works
}
```

---

## Implementation Timeline

**Estimated Time:** 1-2 hours

- Template creation: 15 minutes
- Generate 14 files: 45 minutes (3 min each)
- Compilation fixes: 15 minutes
- Testing all 14 files: 15 minutes
- Documentation: 15 minutes

---

## Approval Checklist

Before proceeding:
- [ ] Structure matches dd_cascade tests ✓
- [ ] 14 test files is correct count ✓
- [ ] Skeleton approach (minimal smoke tests only) is acceptable ✓
- [ ] MANUAL_TESTING = 1 for Phase 0 is correct ✓
- [ ] Exception handling pattern is correct ✓
- [ ] TODO comments document future work ✓
- [ ] Ready to implement ✓

---

**Plan Status:** AWAITING REVIEW
**Created:** 2025-11-03
**Author:** Claude Code (with Theodore Omtzigt)
