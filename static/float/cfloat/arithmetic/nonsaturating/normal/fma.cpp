// fma.cpp: test suite runner for cfloat fused multiply-accumulate algorithm
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
void GenerateTestCase(Ty x, Ty y, Ty z) {
	using namespace sw::universal;
	constexpr unsigned nbits       = Cfloat::nbits;
	constexpr unsigned es          = Cfloat::es;
	using BlockType                = typename Cfloat::BlockType;
	constexpr bool hasSubnormals   = Cfloat::hasSubnormals;
	constexpr bool hasMaxExpValues = Cfloat::hasMaxExpValues;
	constexpr bool isSaturating    = Cfloat::isSaturating;
	cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating> cx, cy, cz, cref, cfma;

	Ty ref = std::fma(x, y, z);
	cref = ref;
	cx = x;
	cy = y;
	cz = z;
	cfma = sw::universal::fma(cx, cy, cz);

	auto precision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Ty>::max_digits10);
	ReportValue(cx, "cx");
	ReportValue(cy, "cy");
	ReportValue(cz, "cz");
	ReportValue(ref,  "fma native reference");
	ReportValue(cref, "fma cfloat reference");
	ReportValue(cfma, "fma cfloat result   ");
	std::cout << (cref == cfma ? "PASS" : "FAIL") << "\n\n";
	std::cout << color_print(cfma) << '\n';
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

	std::string test_suite  = "cfloat fma validation";
	std::string test_tag    = "fma";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	constexpr bool hasSubnormals   = false;
	constexpr bool hasMaxExpValues = false;
	constexpr bool isSaturating    = false;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using cfloat8 = cfloat<  8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using cfloat16 = cfloat< 16, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using cfloat32 = cfloat< 32, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
//	using cfloat64 = cfloat< 64, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
//	using cfloat80 = cfloat< 80, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
//	using cfloat128 = cfloat<128, 15, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;

	float x{ 2.0f }, y{ 1.0f }, z{ 0.0f };
	cfloat8 c8(SpecificValue::minpos);
	z = float(c8);
	/* quarter  precision */ GenerateTestCase < cfloat8, float>(x, y, z);
	/* half     precision */ GenerateTestCase < cfloat16, float>(x, y, z);
	/* single   precision */ GenerateTestCase < cfloat32, float>(x, y, z);
//	/* double   precision */ GenerateTestCase < cfloat64, double>(x, y, z);
//	/* extended precision */ GenerateTestCase < cfloat80, double>(x, y, z);
//	/* quad     precision */ GenerateTestCase < cfloat128, double>(x, y, z);

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
