// parse_failure.cpp: efloat leaves a well-formed value when a parse fails
//
// parse() began with value.clear(), which sets the Normal state with NO limbs -- a value
// no arithmetic produces. Every `return false` handed that back: it was not zero to
// iszero() or operator==, it printed as 0.000000e-01, and to_binary rendered a truncated
// 0b0.0. assign() ignores parse()'s result, so a failed assign() left it in the caller's
// object (#1458). A failed parse now leaves the canonical zero.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <string>

#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_text(const std::string& actual, const std::string& wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got \"" << actual << "\", expected \"" << wanted << "\"\n";
		}
		return 1;
	}

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}


	using Efloat = efloat<8>;

	int VerifyFailedParseLeavesZero(bool reportTestCases) {
		int fails = 0;

		const Efloat canonical(0);
		const std::string zeroText = [&] { std::stringstream s; s << canonical; return s.str(); }();

		// every documented failure path, and the empty and whitespace-only cases
		const char* malformed[] = { "garbage", "", "   ", "1e", "e5", "--1", "0x10", "infinit" };
		for (const char* txt : malformed) {
			Efloat v;
			v.assign(txt);

			if (!v.iszero()) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL \"" << txt << "\" did not leave a zero\n";
				continue;
			}
			fails += expect_true(v == canonical, "it compares equal to zero", reportTestCases);
			fails += expect_text(to_binary(v), to_binary(canonical), "and renders as one", reportTestCases);
			std::stringstream s;
			s << v;
			fails += expect_text(s.str(), zeroText, "and prints as one", reportTestCases);
		}

		// parse() reports the failure, whatever it leaves behind
		{
			Efloat v;
			fails += expect_true(!parse(std::string("garbage"), v), "parse returns false",
				reportTestCases);
			fails += expect_true(v.iszero(), "and leaves a zero", reportTestCases);
		}

		return fails;
	}

	int VerifySuccessfulParseStillWorks(bool reportTestCases) {
		int fails = 0;

		struct Case { const char* txt; double value; };
		const Case good[] = { {"1.0", 1.0}, {"-2.5", -2.5}, {"0", 0.0}, {"1e3", 1000.0} };
		for (const Case& c : good) {
			Efloat v;
			fails += expect_true(parse(std::string(c.txt), v), "a good parse succeeds", reportTestCases);
			fails += expect_true(double(v) == c.value, "and yields the value", reportTestCases);
		}

		// nan and inf are values, not failures
		{
			Efloat v;
			fails += expect_true(parse(std::string("inf"), v), "inf parses", reportTestCases);
			fails += expect_true(!v.iszero(), "and is not a zero", reportTestCases);
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

	std::string test_suite  = "efloat parse failure state";
	std::string test_tag    = "efloat parse";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyFailedParseLeavesZero(reportTestCases), test_tag, "failed parse");
	nrOfFailedTestCases += ReportTestResult(VerifySuccessfulParseStillWorks(reportTestCases), test_tag, "successful parse");
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
