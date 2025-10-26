# Troubleshooting regression failures

The regression system in Universal is build around this structure:

```cpp
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal assignment";
	std::string test_tag    = "assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	TestCase<areal<16, 8, uint8_t>, double>(TestCaseOperator::ADD, INFINITY, INFINITY);
	TestCase<areal<8, 4, uint8_t>, float>(TestCaseOperator::ADD, 0.5f, -0.5f);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyAddition< areal<8, 2, uint8_t> >("Manual Testing", true), "areal<8,2,uint8_t>", "addition");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(tag, bReportIndividualTestCases), "areal<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(tag, bReportIndividualTestCases), "areal<8,4>", "addition");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<10, 4>(tag, bReportIndividualTestCases), "areal<10,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<16, 8>(tag, bReportIndividualTestCases), "areal<16,8>", "addition");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}

```

## Regression Testing helpers

There are helper functions that make it easier to create regression tests. The outer structure of a regression suite is this code pattern:

```cpp
try {
	using namespace sw::universal;

	std::string test_suite  = "areal assignment";
	std::string test_tag    = "assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
```

The functions `ReportTestSuiteHeader` and `ReportTestSuiteResults` are convenience functions that give the library a consistent look and feel when running regressions.

The compilation of this structure is controlled by a standard pattern of macros that can be overridden by compiler command line flags:

```cpp
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif
```

When the macro `MANUAL_TESTING` is set to non-zero, we enable the section of the `main()` that we can use to create handmade tests and inspections to troubleshoot a very specific failure mode. The typical process is to create a regression test that enumerates some portion of an operator state space, and when it fails to pick up the specific failure and then drill down into the code by writing a `TestCase` clause for that specific failure mode.

# How to create a regression suite

Regression tests suites are automatically registered through a CMake macro in the Universal cmake design, called `compile_all`.

The regression suites are organized in elastic vs static arithmetic types, as shown here:
```txt
universal
|── .devcontainer
├── .github/                               # GitHub Actions CI/CD
│   └── workflows/
│       └── cmake.yml                      # Continuous integration
|── applications
|── benchmark
|── bin
|── config
|── c_api
|── data
|── docker
|── docs
|── education
|── elastic                    # Regression Suites for elastic (== adaptive) number systems
    ├──  decimal
    ├──  efloat
    ├──  einteger
    ├──  elreal
    ├──  ereal
    ├──  rational
    └── unum
|── include
|── internal
|── joss
|── linalg
|── mixedprecision
|── numeric
|── playground
|── static                     # Regression Suites for static number systems
    |── appenv
    ├── areal
    |   ├──  api
    |   ├──  arithmetic
    |   ├──  conversion
    |   ├──  logic
    |   ├──  math
    |   └──  standard
    |── bfloat16
    |── cfloat
    |── conversions
    |── dbns
    |── dd
    |── decimal
    |── dfloat
    |── fixpnt
    |   |──  binary
    |   └──  decimal
    |── integer
    |   |──  binary
    |   └──  decimal
    |── lns
    |── native
    |── posit
    |── posit2
    |── posito
    |── qd
    |── rational
    |── regression
    |── sorn
    |── takum
    |── td
    |── unum2
    └── valid
|──  tools
|──  type_hierarchy
└──  validation
```

As the implementation of a number system needs to overload construction, conversion, assignment, arithmetic, logic operators, and math library function calls, the regression test suite is organized into 5 or 6 categories:
1. api
    - typically to show case the basic usage of the number system
2. arithmetic
    - regression suites that test the arithmetic operators of the type
3. conversion
    - regression suites that test the assignment and conversion operators
4. logic
    - regression suites that test the logic operators
5. math
    - regression suites that test the math library for the type
6. performance
    - regression suites that test the performance of the type   

If we look at the CMakeLists.txt file in the top directory of a number system, for example, `universal/static/areal/`, we see this pattern:

```cmake
file (GLOB API_SRC "api/*.cpp")
file (GLOB LOGIC_SRC "logic/*.cpp")
file (GLOB CONVERSION_SRC "conversion/*.cpp")
file (GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file (GLOB STANDARD_SRC "standard/*.cpp")

compile_all("true" "areal" "Number Systems/static/floating-point/interval/areal/api" "${API_SRC}")
compile_all("true" "areal" "Number Systems/static/floating-point/interval/areal/logic" "${LOGIC_SRC}")
compile_all("true" "areal" "Number Systems/static/floating-point/interval/areal/conversion" "${CONVERSION_SRC}")
compile_all("true" "areal" "Number Systems/static/floating-point/interval/areal/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "areal" "Number Systems/static/floating-point/interval/areal/standard" "${STANDARD_SRC}")

```

The `compile_all` macro will take all `.cpp` files in a directory and make a `ctest` out of them, registering these tests with ctest and making them part of the larger regression test constructed by composing all these number system regression tests. The top level CMakeLists.txt file will contain compilation guards that enable or disable these regression suites. The guard `UNIVRSL_BUILD_CI` will turn on the regression tests of all the number systems, including the internal types, which are used as building blocks.