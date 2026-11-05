// conversion.cpp : test suite runner for conversion operators to arbitrary precision, fixed-size double-base logarithmic floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This runner checked nothing until #1941. MANUAL_TESTING was set to 1, so the regression levels
// never ran; VerifyConversion printed the encodings and returned 0; and VerifyIntegerConversion
// compared an integer with a dbns built from that same integer, which always holds. That is how
// the conversion defects fixed in #1933 survived.
//
// The value of a dbns is (-1)^s * 0.5^a * 3^b, and by unique factorization every (a, b) is a
// distinct value, so conversion has a well-defined answer: the encoding nearest the input in the
// logarithmic domain. This runner checks its boundaries directly: on each side of the midpoint
// between two neighbouring encodings, a value must convert to the nearer one.
// static/logarithmic/dbns/conversion/nearest.cpp samples the same rule at random, over wider
// configurations.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <universal/number/dbns/dbns.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// the positive encodings that decode to a normal double, sorted by value
	template<typename TestType>
	std::vector<TestType> SortedPositiveEncodings() {
		std::vector<TestType> encodings;
		TestType a;
		for (std::uint64_t raw = 0; raw < (std::uint64_t(1) << (TestType::nbits - 1)); ++raw) {
			a.setbits(raw);
			if (a.isnan() || a.iszero()) continue;
			if (!std::isnormal(double(a))) continue;
			encodings.push_back(a);
		}
		std::sort(encodings.begin(), encodings.end(), [](const TestType& x, const TestType& y) { return double(x) < double(y); });
		return encodings;
	}

	template<typename TestType>
	int ReportMismatch(bool reportTestCases, double input, const TestType& result, const TestType& expected, int& reported) {
		if (reportTestCases && reported++ < 10) {
			std::cerr << "FAIL " << type_tag(TestType()) << " " << std::setprecision(17) << input
			          << " -> " << double(result) << " " << to_binary(result)
			          << ", expected " << double(expected) << " " << to_binary(expected) << '\n';
		}
		return 1;
	}

	// Conversion from double, at the rounding boundaries:
	// - every encoding converts back to itself, with either sign;
	// - just below the log-domain midpoint of two neighbours a value goes to the lower one, and
	//   just above it to the upper one, with either sign;
	// - below half of minpos a value is zero; beyond maxpos it saturates to maxpos.
	template<unsigned nbits, unsigned fbbits, typename bt, auto... x>
	int VerifyConversion(bool reportTestCases) {
		using TestType = dbns<nbits, fbbits, bt, x...>;
		int nrOfFailedTests = 0, reported = 0;
		const std::vector<TestType> enc = SortedPositiveEncodings<TestType>();
		// a nudge in log2 far larger than double's rounding of log2, far smaller than any gap
		const double nudge = 1.0e-9;
		for (std::size_t i = 0; i < enc.size(); ++i) {
			const double v = double(enc[i]);
			if (!(TestType(v) == enc[i])) nrOfFailedTests += ReportMismatch(reportTestCases, v, TestType(v), enc[i], reported);
			if (!(TestType(-v) == -enc[i])) nrOfFailedTests += ReportMismatch(reportTestCases, -v, TestType(-v), -enc[i], reported);
			if (i + 1 == enc.size()) break;
			const double mid = 0.5 * (std::log2(v) + std::log2(double(enc[i + 1])));
			const double below = std::exp2(mid - nudge), above = std::exp2(mid + nudge);
			if (!(TestType(below) == enc[i]))      nrOfFailedTests += ReportMismatch(reportTestCases, below, TestType(below), enc[i], reported);
			if (!(TestType(above) == enc[i + 1]))  nrOfFailedTests += ReportMismatch(reportTestCases, above, TestType(above), enc[i + 1], reported);
			if (!(TestType(-below) == -enc[i]))     nrOfFailedTests += ReportMismatch(reportTestCases, -below, TestType(-below), -enc[i], reported);
			if (!(TestType(-above) == -enc[i + 1])) nrOfFailedTests += ReportMismatch(reportTestCases, -above, TestType(-above), -enc[i + 1], reported);
		}
		// the ends of the range
		const TestType minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
		const double dminpos = double(minpos);
		if (!TestType(0.49 * dminpos).iszero()) nrOfFailedTests += ReportMismatch(reportTestCases, 0.49 * dminpos, TestType(0.49 * dminpos), TestType(0.0), reported);
		if (!(TestType(0.51 * dminpos) == minpos)) nrOfFailedTests += ReportMismatch(reportTestCases, 0.51 * dminpos, TestType(0.51 * dminpos), minpos, reported);
		const double dmaxpos = double(maxpos);
		if (std::isfinite(4.0 * dmaxpos) && !(TestType(4.0 * dmaxpos) == maxpos)) nrOfFailedTests += ReportMismatch(reportTestCases, 4.0 * dmaxpos, TestType(4.0 * dmaxpos), maxpos, reported);
		return nrOfFailedTests;
	}

	// Conversion from integers, and to them:
	// - an integer converts to the encoding nearest it, as its double does;
	// - a power of three, the only integers a dbns holds exactly, converts exactly;
	// - long(v) truncates the value of every encoding toward zero.
	template<unsigned nbits, unsigned fbbits, typename bt, auto... x>
	int VerifyIntegerConversion(bool reportTestCases) {
		using TestType = dbns<nbits, fbbits, bt, x...>;
		int nrOfFailedTests = 0, reported = 0;
		const std::vector<TestType> enc = SortedPositiveEncodings<TestType>();
		auto nearest = [&](double v) {   // the reference: nearest encoding to v in log2
			const double l = std::log2(v);
			TestType best = enc.front();
			double bestErr = 1.0e300;
			for (const TestType& e : enc) {
				double err = std::fabs(std::log2(double(e)) - l);
				if (err < bestErr) { bestErr = err; best = e; }
			}
			return best;
		};
		const double top = std::fmin(double(enc.back()), 4096.0);
		for (long n = 1; static_cast<double>(n) <= top; ++n) {
			const double dn = static_cast<double>(n);
			const TestType expected = nearest(dn);
			if (!(TestType(n) == expected))  nrOfFailedTests += ReportMismatch(reportTestCases, dn, TestType(n), expected, reported);
			if (!(TestType(-n) == -expected)) nrOfFailedTests += ReportMismatch(reportTestCases, -dn, TestType(-n), -expected, reported);
		}
		for (long p = 1; static_cast<double>(p) <= double(enc.back()) && p < (1l << 40); p *= 3) {
			if (double(TestType(p)) != static_cast<double>(p)) nrOfFailedTests += ReportMismatch(reportTestCases, static_cast<double>(p), TestType(p), TestType(p), reported);
		}
		for (const TestType& e : enc) {
			const double v = double(e);
			if (v >= 9.0e18) continue;   // beyond long
			const long want = static_cast<long>(std::trunc(v));
			if (long(e) != want || long(-e) != -want) {
				if (reportTestCases && reported++ < 10) std::cerr << "FAIL " << type_tag(TestType()) << " long(" << v << ") = " << long(e) << ", expected " << want << '\n';
				++nrOfFailedTests;
			}
		}
		return nrOfFailedTests;
	}

	// the encodings of a configuration, by bit pattern and sorted by value: a debugging aid
	template<unsigned nbits, unsigned fbbits, typename bt, auto... x>
	void PrintValueTable(std::ostream& ostr) {
		using TestType = dbns<nbits, fbbits, bt, x...>;
		for (const TestType& e : SortedPositiveEncodings<TestType>()) {
			ostr << std::setw(20) << double(e) << " : " << to_binary(e) << '\n';
		}
	}

} } // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

	std::string test_suite  = "dbns<> conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	PrintValueTable<6, 3, std::uint8_t>(std::cout);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<6, 3, std::uint8_t>(true), "dbns<6,3>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 1, std::uint8_t>(reportTestCases), "dbns<4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 2, std::uint8_t>(reportTestCases), "dbns<4,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 2, std::uint8_t>(reportTestCases), "dbns<6,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 3, std::uint8_t>(reportTestCases), "dbns<6,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 4, std::uint8_t>(reportTestCases), "dbns<6,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 2, std::uint8_t>(reportTestCases), "dbns<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 3, std::uint8_t>(reportTestCases), "dbns<8,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 4, std::uint8_t>(reportTestCases), "dbns<8,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 5, std::uint8_t>(reportTestCases), "dbns<8,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 6, std::uint8_t>(reportTestCases), "dbns<8,6>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion< 5, 2, std::uint8_t>(reportTestCases), "dbns<5,2>", "integer conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion< 8, 3, std::uint8_t>(reportTestCases), "dbns<8,3>", "integer conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<10, 4, std::uint8_t>(reportTestCases), "dbns<10,4>", "integer conversion");
#endif

#if REGRESSION_LEVEL_2
	// multi-block layouts: dbns wider than its uint8_t block type (#1933)
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 3, std::uint8_t>(reportTestCases), "dbns<10,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 4, std::uint8_t>(reportTestCases), "dbns<10,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 5, std::uint8_t>(reportTestCases), "dbns<10,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 4, std::uint16_t>(reportTestCases), "dbns<12,4,uint16>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 5, std::uint8_t>(reportTestCases), "dbns<14,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<12, 5, std::uint8_t>(reportTestCases), "dbns<12,5>", "integer conversion");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 5, std::uint8_t>(reportTestCases), "dbns<16,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 8, std::uint16_t>(reportTestCases), "dbns<16,8,uint16>", test_tag);
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
