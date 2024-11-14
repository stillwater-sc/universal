// conversion.cpp: test suite runner for conversion of fixed-sized, arbitrary configuration rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/rational/rational.hpp>
#include <universal/verification/test_suite.hpp>


namespace sw { namespace universal {

	template<typename Real, std::enable_if_t<std::is_floating_point<Real>::value, bool> = true>
	void reportConversionError(Real fp) {
		auto precision = std::cout.precision();
		std::cout << std::setprecision(std::numeric_limits<Real>::max_digits10);

		std::cout << "conversion error : " << to_binary(fp) << " : " << fp << "\n\n";

		std::cout << std::setprecision(precision);
	}

	template<typename RationalType, typename Real, std::enable_if_t<is_rational<RationalType>, bool> = true>
	Real reportRoundTrip(int64_t numerator, int64_t denominator) {

		auto precision = std::cout.precision();
		std::cout << std::setprecision(std::numeric_limits<Real>::max_digits10);

		RationalType twoTenths(numerator, denominator);
		std::cout << to_binary(twoTenths) << " : " << Real(twoTenths) << '\n';

		Real fp1 = Real(twoTenths);

		RationalType roundtrip{ fp1 };
		std::cout << to_binary(roundtrip) << " : " << Real(roundtrip) << '\n';

		Real fp2 = Real(roundtrip);

		std::cout << std::setprecision(precision);
		return std::abs(fp1 - fp2);
	}

	template<typename Real, std::enable_if_t<std::is_floating_point<Real>::value, bool> = true>
	void Experiment() {
		reportConversionError(reportRoundTrip<rb8, Real>(1, 5));
		reportConversionError(reportRoundTrip<rb16, Real>(1, 5));
		reportConversionError(reportRoundTrip<rb32, Real>(1, 5));
		reportConversionError(reportRoundTrip<rb64, Real>(1, 5));
	}

} }

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

	std::string test_suite  = "rational conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// rational to ieee-754 will yield rounding errors. For example, 1/5 does not have a representation in IEEE-754.
	// what is the rounding logic that would be able to support a round-trip?

	Experiment<float>();
	Experiment<double>();

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
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
