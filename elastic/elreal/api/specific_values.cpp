// specific_values.cpp: elreal SpecificValue extremes follow the host
//
// elreal(SpecificValue) hard-coded double's magnitudes whatever the host: maxpos was
// 1.0e308 and minpos 1.0e-307. Cast to a float host 1.0e308 is inf, which from_native
// rejects as a non-normalised block, so elreal<float>(SpecificValue::maxpos) aborted in a
// debug build; 1.0e-307 underflowed the cast, so minpos came back zero. half and bfloat16
// had the same problem in narrower ranges (#1463). The extremes are now the host's, which
// is also what numeric_limits<elreal<FpType>> reports.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <string>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}


	template<typename FpType>
	int VerifyExtremes(const char* host, bool reportTestCases) {
		int fails = 0;
		using R = elreal<FpType>;
		const double hostMax = static_cast<double>(std::numeric_limits<FpType>::max());
		const double hostMin = static_cast<double>(std::numeric_limits<FpType>::min());

		// constructing these used to abort on every host but double
		const R maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
		const R minpos(SpecificValue::minpos), minneg(SpecificValue::minneg);

		fails += expect_true(double(maxpos) ==  hostMax, host, reportTestCases);
		fails += expect_true(double(maxneg) == -hostMax, host, reportTestCases);
		fails += expect_true(double(minpos) ==  hostMin, host, reportTestCases);
		fails += expect_true(double(minneg) == -hostMin, host, reportTestCases);

		// minpos used to come back zero on a narrow host
		fails += expect_true(double(minpos) > 0.0, "minpos is not zero", reportTestCases);
		// and the extremes are finite, not an infinity the host overflowed to
		fails += expect_true(std::isfinite(double(maxpos)), "maxpos is finite", reportTestCases);

		// SpecificValue agrees with numeric_limits<elreal<FpType>>
		fails += expect_true(double(maxpos) == double(std::numeric_limits<R>::max()),
			"maxpos agrees with numeric_limits::max", reportTestCases);
		fails += expect_true(double(minpos) == double(std::numeric_limits<R>::min()),
			"minpos agrees with numeric_limits::min", reportTestCases);

		return fails;
	}

}  // anonymous namespace

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

	std::string test_suite  = "elreal SpecificValue extremes";
	std::string test_tag    = "elreal SpecificValue";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExtremes<double>("double", reportTestCases), test_tag, "double host");
	nrOfFailedTestCases += ReportTestResult(VerifyExtremes<float>("float", reportTestCases), test_tag, "float host");
	nrOfFailedTestCases += ReportTestResult(VerifyExtremes<half>("half", reportTestCases), test_tag, "half host");
	nrOfFailedTestCases += ReportTestResult(VerifyExtremes<bfloat16>("bfloat16", reportTestCases), test_tag, "bfloat16 host");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
