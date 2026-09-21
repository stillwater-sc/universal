// manipulators.cpp: dbns scale() and the manipulator surface
//
// dbns::scale() returned e0 + e1*log2(3). A dbns encodes (-1)^s * base0^e0 * base1^e1
// with base0 = 0.5, so the base-0.5 exponent contributes NEGATIVELY: the scale is
// -e0 + e1*log2(3). Every value with a non-zero e0 reported the wrong scale -- 2 for
// 1.5, whose scale is 0, and +1 for 0.5, whose scale is -1. Zero reported 7, and the
// static_cast truncated toward zero where a scale must floor (#1582).
//
// operator< already had it right -- it documents the value as (-1)^s * 0.5^a * 3^b and
// orders on M(x) = -a + b*log2of3 -- so the encoding semantics were never in doubt; the
// accessor simply disagreed with the rest of the type.
//
// The oracle here is independent of the implementation: floor(log2(|v|)) computed from
// the decoded double, which is what a binary scale means.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>

#include <universal/number/dbns/dbns.hpp>
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

	// scale() must equal floor(log2(|v|)) for every encoding that has one.
	template<typename DbnsType>
	int VerifyScaleAgainstLog2(const char* tag, bool reportTestCases) {
		constexpr unsigned nbits = DbnsType::nbits;
		constexpr unsigned NR_ENCODINGS = (1u << nbits);
		int fails = 0;
		int reported = 0;

		for (unsigned bits = 0; bits < NR_ENCODINGS; ++bits) {
			DbnsType a;
			a.setbits(bits);
			if (a.iszero() || a.isnan()) {
				// no binary scale to report; the contract is that it answers 0 rather
				// than the garbage the raw exponent arithmetic used to produce
				if (a.scale() != 0) {
					++fails;
					if (reportTestCases && reported++ < 5) {
						std::cout << "    FAIL " << tag << " encoding " << bits
						          << ": a non-numeric value reports scale " << a.scale() << '\n';
					}
				}
				continue;
			}
			const double d = double(a);
			if (d == 0.0 || !std::isfinite(d)) continue;   // outside what the oracle can judge
			const int expected = static_cast<int>(std::floor(std::log2(std::fabs(d))));
			if (a.scale() != expected) {
				++fails;
				if (reportTestCases && reported++ < 5) {
					std::cout << "    FAIL " << tag << " encoding " << bits << " value " << d
					          << ": scale() = " << a.scale() << ", floor(log2|v|) = " << expected << '\n';
				}
			}
		}
		return fails;
	}

	// The specific values from the issue, spelled out so a regression names itself.
	int VerifyReportedCases(bool reportTestCases) {
		using D = dbns<8, 3>;
		int fails = 0;
		struct Case { double v; int scale; const char* what; };
		const Case cases[] = {
			{ 1.5,  0, "1.5 has scale 0, not 2" },
			{ 0.5, -1, "0.5 has scale -1, not 1" },
			{ 0.25,-2, "0.25 has scale -2, not 2" },
			{ 4.5,  2, "4.5 has scale 2, not 4" },
			{ 3.0,  1, "3.0 has scale 1" },
			{ 1.0,  0, "1.0 has scale 0" },
		};
		for (const Case& c : cases) {
			D a(c.v);
			fails += expect_true(a.scale() == c.scale, c.what, reportTestCases);
		}
		// zero has no binary scale, and used to report 7
		fails += expect_true(D(0.0).scale() == 0, "zero reports scale 0, not 7", reportTestCases);
		return fails;
	}

	// components() and to_triple() used to print fraction(), which is hard-coded to 0 for
	// every dbns value: a constant dressed up as a decoded field.
	int VerifyNoPhantomFraction(bool reportTestCases) {
		using D = dbns<8, 3>;
		int fails = 0;

		const D a(1.5), b(3.0);
		// the renderers distinguish values -- they cannot if they lean on a constant
		fails += expect_true(components(a) != components(b),
			"components distinguishes different values", reportTestCases);
		fails += expect_true(to_triple(a) != to_triple(b),
			"to_triple distinguishes different values", reportTestCases);
		// and they report what a dbns actually carries: the two base exponents
		fails += expect_true(components(a).find("e0=") != std::string::npos
		                  && components(a).find("e1=") != std::string::npos,
			"components reports the base exponents", reportTestCases);
		fails += expect_true(to_triple(a).find("e0=") != std::string::npos,
			"to_triple reports the base exponents", reportTestCases);
		// the accessor stays, for generic code that probes for it
		fails += expect_true(a.fraction() == 0, "fraction() still answers 0", reportTestCases);

		return fails;
	}

}  // anonymous namespace

#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dbns scale and manipulator surface";
	std::string test_tag    = "dbns manipulators";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	{
		using D = dbns<8, 3>;
		for (double v : { 1.5, 0.5, 0.25, 3.0, 4.5, 1.0, 0.0 }) {
			D a(v);
			std::cout << double(a) << " : scale " << a.scale()
			          << " : " << components(a) << " : " << info_print(a) << '\n';
		}
	}
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyReportedCases(reportTestCases), test_tag, "reported cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNoPhantomFraction(reportTestCases), test_tag, "no phantom fraction");
	nrOfFailedTestCases += ReportTestResult(VerifyScaleAgainstLog2<dbns<8, 3>>("dbns<8,3>", reportTestCases),
		test_tag, "scale vs floor(log2) dbns<8,3>");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyScaleAgainstLog2<dbns<8, 4>>("dbns<8,4>", reportTestCases),
		test_tag, "scale vs floor(log2) dbns<8,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyScaleAgainstLog2<dbns<10, 4>>("dbns<10,4>", reportTestCases),
		test_tag, "scale vs floor(log2) dbns<10,4>");
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
