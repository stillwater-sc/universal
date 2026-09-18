// istream.cpp: lns<16, 8> stream extraction contracts
//
// operator>>(std::istream&, lns<16, 8>&) reads a double and assigns it. Extraction has three
// outcomes and they are not symmetric: on success num_get stores the value; on a good
// stream with bad input num_get stores 0 and sets failbit; on an ALREADY-FAILED stream the
// sentry fails, num_get never runs, and the double is never written. Reading it there was
// undefined, and assigning it clobbered the target with whatever was on the stack (#1450).
//
// The contract, matching the rest of the tree and native extraction: a failed extraction
// leaves the target ALONE.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <sstream>
#include <string>

#include <universal/number/lns/lns.hpp>
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


	using TestType = lns<16, 8>;

	int VerifyExtraction(bool reportTestCases) {
		int fails = 0;

		// a good extraction assigns
		{
			TestType v{};
			std::istringstream is("2.0");
			is >> v;
			fails += expect_true(!is.fail(), "a good read does not set failbit", reportTestCases);
			fails += expect_true(v == TestType(2.0), "and assigns the value", reportTestCases);
		}

		// an ALREADY-FAILED stream must not touch the target. This is the path that used
		// to read an indeterminate double.
		{
			TestType v = TestType(1.0);
			const TestType before = v;
			std::istringstream is("not a number");
			double drain{};
			is >> drain;                       // fails the stream
			fails += expect_true(is.fail(), "the stream is in a failed state", reportTestCases);
			is >> v;
			fails += expect_true(v == before, "a failed stream leaves the target untouched",
				reportTestCases);
		}

		// bad input on a good stream sets failbit, and also leaves the target alone
		{
			TestType v = TestType(1.0);
			const TestType before = v;
			std::istringstream is("garbage");
			is >> v;
			fails += expect_true(is.fail(), "bad input sets failbit", reportTestCases);
			fails += expect_true(v == before, "and leaves the target untouched", reportTestCases);
		}

		// the loop idiom terminates, and the values it read survive the final failed read
		{
			std::istringstream is("2.0 1.0");
			TestType a{}, b{}, sink{};
			int reads = 0;
			while (is >> sink) { if (reads == 0) a = sink; else if (reads == 1) b = sink; ++reads; }
			fails += expect_true(reads == 2, "two values are read", reportTestCases);
			fails += expect_true(a == TestType(2.0) && b == TestType(1.0),
				"and both survive the terminating read", reportTestCases);
		}

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

	std::string test_suite  = "lns<16, 8> stream extraction";
	std::string test_tag    = "lns<16, 8> istream";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExtraction(reportTestCases), test_tag, "extraction");
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
