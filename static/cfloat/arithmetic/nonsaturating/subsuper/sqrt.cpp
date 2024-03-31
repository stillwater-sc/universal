// sqrt.cpp: test suite runner for classic cfloat square root algorithm
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
//#define ALGORITHM_VERBOSE_OUTPUT 1
//#define ALGORITHM_TRACE_SQRT 1
#include <universal/number/algorithm/newtons_iteration.hpp>
// #define CFLOAT_NATIVE_SQRT 1
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in posit.hpp
// for most bugs they are traceable with _trace_conversion and _trace_[add|mul|div]
template<typename Cfloat, typename Ty>
void GenerateTestCase(Ty a) {
	constexpr unsigned nbits       = Cfloat::nbits;
	constexpr unsigned es          = Cfloat::es;
	using BlockType                = typename Cfloat::BlockType;
	constexpr bool hasSubnormals   = Cfloat::hasSubnormals;
	constexpr bool hasSupernormals = Cfloat::hasSupernormals;
	constexpr bool isSaturating    = Cfloat::isSaturating;
	Ty ref;
	sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> ca, cref, csqrt;
	ca = a;
	ref = std::sqrt(a);
	cref = ref;
	csqrt = sw::universal::sqrt(ca);
	auto precision = std::cout.precision();
	std::cout << std::setprecision(17);
	std::cout << std::setw(nbits) <<  a << " -> sqrt("  << a << ") = " << std::setw(nbits) << ref << '\n';
	std::cout << std::setw(nbits) << ca << " -> sqrt(" << ca << ") = " << std::setw(nbits) << csqrt << '\n';
	std::cout << to_binary(ca) << " -> sqrt(" << ca << ") = " << to_binary(csqrt) << '\n';
	std::cout << std::setw(nbits + 35) << " reference = " << to_binary(cref) << " : ";
	std::cout << (cref == csqrt ? "PASS" : "FAIL") << "\n\n";
	std::cout << color_print(csqrt) << '\n';
	std::cout << std::setprecision(precision);
}

template<typename Real>
void CheckNewtonsIteration(Real value) {
	auto precision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Real>::max_digits10);

	Real reference = sqrt(value);
	Real root = sw::universal::newtons_iteration(value);

	bool printHeader = true;
	if (printHeader && !std::isnormal(value)) {
		std::cout << "Subnormal range\n";
		printHeader = false;
	}
	std::cout << "sqrt( " << value << ")\n";
	std::cout << "Standard Library   : " << reference << '\n';
	std::cout << "Newton's Iteration : " << root << '\n';
	std::cout << "Absolute Error     : " << std::abs(root - reference) << '\n';

	std::cout << std::setprecision(precision);
}

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

	std::string test_suite  = "cfloat square root validation";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	constexpr bool hasSubnormals   = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating    = false;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	double v = 2.25;  // sqrt(2.25) = 1.5
	/* quarter  precision */ GenerateTestCase < cfloat<  8,  2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double>(v);
	/* half     precision */ GenerateTestCase < cfloat< 16,  5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double>(v);
	/* single   precision */ GenerateTestCase < cfloat< 32,  8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double>(v);
	/* double   precision */ GenerateTestCase < cfloat< 64, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double>(v);
	/* extended precision */ GenerateTestCase < cfloat< 80, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double>(v);
	/* quad     precision */ GenerateTestCase < cfloat<128, 15, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double>(v);

	CheckNewtonsIteration(2.0f);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatSqrt< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(true), "cfloat<8,4>", "sqrt");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4
	using Cfloat64_ttf = cfloat<64, 11, uint64_t, hasSubnormals, hasSupernormals, isSaturating>;

	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms< Cfloat64_ttf >(reportTestCases, OPCODE_SQRT, 1000, double(Cfloat64_ttf(SpecificValue::minpos))), type_tag(Cfloat64_ttf()), "sqrt");

#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
