// encodings.cpp: dfixpnt arithmetic must not depend on the digit encoding
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Regression for #1474. dfixpnt picks the sign of a mixed-sign sum or difference, and
// clamps in saturating mode, by comparing blockdecimal magnitudes. For BID storage that
// comparison used blockbinary's operator<, which reads the top storage bit as a sign bit
// even for an Unsigned blockbinary (#1479), so about 22% of random dfixpnt<8,4,BID> sums
// and differences came back with the wrong sign, and division and ordering were wrong too.
// BCD, DPD and BID hold the same values, so here they must agree, operation by
// operation, and + and - must also match exact integer arithmetic.
//
// Digit counts with N % 3 == 2 also cover #1480: DPD used to drop bit 3 of the leading
// digit there, so a leading 8 or 9 read back as 0 or 1, and maxpos() was 19999.999 for
// dfixpnt<8,3,DPD> (#1477).
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/dfixpnt/dfixpnt.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// value of a dfixpnt with R fraction digits as a scaled integer
template<typename F>
long long scaled(const F& v, long long scale) {
	return static_cast<long long>(std::llround(double(v) * static_cast<double>(scale)));
}

// N digits, R of them fractional, in all three encodings: + - * / < <= > >= == != must agree,
// and + and - must equal the exact integer result whenever it is representable
template<unsigned N, unsigned R, bool arithmetic>
int VerifyEncodingsAgree(int nrSamples, bool reportTestCases) {
	using BCD = dfixpnt<N, R, DecimalEncoding::BCD, arithmetic, std::uint8_t>;
	using DPD = dfixpnt<N, R, DecimalEncoding::DPD, arithmetic, std::uint16_t>;
	using BID = dfixpnt<N, R, DecimalEncoding::BID, arithmetic, std::uint32_t>;
	long long lim = 1, scale = 1;
	for (unsigned i = 0; i < N; ++i) lim *= 10;
	for (unsigned i = 0; i < R; ++i) scale *= 10;
	std::mt19937_64 rng(1474);   // deterministic: the engine's output sequence is specified by the standard
	int nrOfFailedTests = 0;
	auto fail = [&](const std::string& what) {
		++nrOfFailedTests;
		if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL: " << what << '\n';
	};
	for (int k = 0; k < nrSamples; ++k) {
		const long long xi = static_cast<long long>(rng() % static_cast<std::uint64_t>(2 * lim - 1)) - (lim - 1);
		const long long yi = static_cast<long long>(rng() % static_cast<std::uint64_t>(2 * lim - 1)) - (lim - 1);
		const double x = static_cast<double>(xi) / static_cast<double>(scale), y = static_cast<double>(yi) / static_cast<double>(scale);
		BCD a(x), b(y); DPD c(x), d(y); BID e(x), f(y);
		const std::string pair = std::to_string(xi) + ", " + std::to_string(yi);

		const BCD s1 = a + b, m1 = a - b, p1 = a * b;
		const DPD s2 = c + d, m2 = c - d, p2 = c * d;
		const BID s3 = e + f, m3 = e - f, p3 = e * f;
		if (s1.to_string() != s2.to_string() || s1.to_string() != s3.to_string()) fail("sum " + pair + ": " + s1.to_string() + " / " + s2.to_string() + " / " + s3.to_string());
		if (m1.to_string() != m2.to_string() || m1.to_string() != m3.to_string()) fail("difference " + pair + ": " + m1.to_string() + " / " + m2.to_string() + " / " + m3.to_string());
		if (p1.to_string() != p2.to_string() || p1.to_string() != p3.to_string()) fail("product " + pair + ": " + p1.to_string() + " / " + p2.to_string() + " / " + p3.to_string());
		if (yi != 0) {
			const BCD q1 = a / b; const DPD q2 = c / d; const BID q3 = e / f;
			if (q1.to_string() != q2.to_string() || q1.to_string() != q3.to_string()) fail("quotient " + pair + ": " + q1.to_string() + " / " + q2.to_string() + " / " + q3.to_string());
		}
		const bool lt = xi < yi, eq = xi == yi;
		if ((a < b) != lt || (c < d) != lt || (e < f) != lt) fail("< " + pair);
		if ((e <= f) != (lt || eq) || (e > f) != (!lt && !eq) || (e >= f) != !lt || (e == f) != eq || (e != f) != !eq) fail("BID ordering " + pair);

		// exact integer reference for + and -, whenever the result is representable
		if (std::llabs(xi + yi) < lim && scaled(s3, scale) != xi + yi) fail("BID sum " + pair + " = " + s3.to_string());
		if (std::llabs(xi - yi) < lim && scaled(m3, scale) != xi - yi) fail("BID difference " + pair + " = " + m3.to_string());
	}
	return nrOfFailedTests;
}

// maxpos, maxneg and minpos must print the same in all three encodings
template<unsigned N, unsigned R>
int VerifyLimitsAgree(bool reportTestCases) {
	using BCD = dfixpnt<N, R, DecimalEncoding::BCD, Modulo, std::uint8_t>;
	using DPD = dfixpnt<N, R, DecimalEncoding::DPD, Modulo, std::uint16_t>;
	using BID = dfixpnt<N, R, DecimalEncoding::BID, Modulo, std::uint32_t>;
	int nrOfFailedTests = 0;
	auto check = [&](const std::string& what, const std::string& a, const std::string& b, const std::string& c) {
		if (a != b || a != c) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: dfixpnt<" << N << ',' << R << "> " << what << ": " << a << " / " << b << " / " << c << '\n';
		}
	};
	check("maxpos", BCD().maxpos().to_string(), DPD().maxpos().to_string(), BID().maxpos().to_string());
	check("maxneg", BCD().maxneg().to_string(), DPD().maxneg().to_string(), BID().maxneg().to_string());
	check("minpos", BCD().minpos().to_string(), DPD().minpos().to_string(), BID().minpos().to_string());
	return nrOfFailedTests;
}

}} // namespace sw::universal

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dfixpnt arithmetic across BCD, DPD and BID encodings (#1474)";
	std::string test_tag    = "encodings";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using F = dfixpnt<9, 4, DecimalEncoding::BID, Modulo, std::uint32_t>;
		F a(-8427.4415), b(1234.5), s = a + b;
		std::cout << a << " + " << b << " = " << s << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// BID storage is 30 bits for 9 digits and 24 bits for 7, so magnitudes >= 2^29 (~5.4e8)
	// and >= 2^23 (~8.4e6) set its top bit: about half of all random operands
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<9, 4, Modulo>(2000, reportTestCases), "dfixpnt<9,4,Modulo>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<9, 4, Saturate>(2000, reportTestCases), "dfixpnt<9,4,Saturate>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<7, 3, Modulo>(2000, reportTestCases), "dfixpnt<7,3,Modulo>", test_tag);
	// N % 3 == 2: DPD's leading digit sits in a 7-bit two-digit group (#1480)
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<8, 4, Modulo>(2000, reportTestCases), "dfixpnt<8,4,Modulo>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<8, 4, Saturate>(2000, reportTestCases), "dfixpnt<8,4,Saturate>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<5, 2, Modulo>(2000, reportTestCases), "dfixpnt<5,2,Modulo>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimitsAgree<1, 0>(reportTestCases) + VerifyLimitsAgree<2, 1>(reportTestCases)
		+ VerifyLimitsAgree<3, 1>(reportTestCases) + VerifyLimitsAgree<4, 2>(reportTestCases) + VerifyLimitsAgree<5, 2>(reportTestCases)
		+ VerifyLimitsAgree<6, 3>(reportTestCases) + VerifyLimitsAgree<7, 3>(reportTestCases) + VerifyLimitsAgree<8, 3>(reportTestCases)
		+ VerifyLimitsAgree<9, 4>(reportTestCases) + VerifyLimitsAgree<10, 4>(reportTestCases) + VerifyLimitsAgree<11, 5>(reportTestCases)
		+ VerifyLimitsAgree<12, 6>(reportTestCases), "dfixpnt<1..12> limits", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<4, 2, Modulo>(2000, reportTestCases), "dfixpnt<4,2,Modulo>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<9, 4, Modulo>(50000, reportTestCases), "dfixpnt<9,4,Modulo> 50k", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<6, 3, Saturate>(50000, reportTestCases), "dfixpnt<6,3,Saturate> 50k", test_tag);
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingsAgree<9, 4, Modulo>(1000000, reportTestCases), "dfixpnt<9,4,Modulo> 1M", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
