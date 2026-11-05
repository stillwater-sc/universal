// exact_parse.cpp: a decimal with a finite binary value parses to that value exactly
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// parse("1.5") gave 1.5 - 1.2e-260, two limbs, not equal to ereal(1.5) (#1932). The parse built
// the integer 15 and divided by 10, and ereal divides by multiplying with a Newton reciprocal of
// the divisor; 1/10 has no finite binary expansion, so the quotient carried a residual. The parse
// now cancels the factors of five between the digits and the power of ten first, in the digit
// string, so a dyadic decimal is an integer times a power of two, and both are exact.
//
// The references are exact by construction: the decimal of m * 2^-j is m * 5^j * 10^-j, which
// this test writes out with string arithmetic, never through a double.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_reporters.hpp>

namespace {

// decimal digits of the unsigned integer v
std::string digitsOf(std::uint64_t v) { return std::to_string(v); }

// digits *= 5, in place
void timesFive(std::string& digits) {
	unsigned carry = 0;
	for (std::size_t k = digits.size(); k-- > 0; ) {
		unsigned v = 5u * static_cast<unsigned>(digits[k] - '0') + carry;
		digits[k] = static_cast<char>('0' + v % 10u);
		carry = v / 10u;
	}
	while (carry) { digits.insert(digits.begin(), static_cast<char>('0' + carry % 10u)); carry /= 10u; }
}

// the exact decimal of m * 2^-j, as "[-]int.frac" or, with scientific = true, as "[-]digitsE-j"
std::string exactDecimal(std::uint64_t m, int j, bool negative, bool scientific) {
	std::string digits = digitsOf(m);               // m * 5^j, the digits of m * 2^-j * 10^j
	for (int k = 0; k < j; ++k) timesFive(digits);
	std::string s = negative ? "-" : "";
	if (scientific) return s + digits + "e-" + std::to_string(j);
	if (static_cast<int>(digits.size()) <= j) digits.insert(digits.begin(), static_cast<std::size_t>(j) - digits.size() + 1, '0');
	std::size_t point = digits.size() - static_cast<std::size_t>(j);
	return s + digits.substr(0, point) + "." + digits.substr(point);
}

// the parse is exact: the difference is zero, and the value uses no more limbs than the reference
template<typename Real>
bool exactly(const Real& got, const Real& want) {
	Real d = got - want;
	return d.iszero() && got.limbs().size() <= want.limbs().size();
}

// m * 2^-j with m < 2^53: one double, so one limb
int VerifySingleLimb(bool reportTestCases) {
	using namespace sw::universal;
	using Real = ereal<8>;
	int nrOfFailedTests = 0;
	std::mt19937_64 rng(1932);
	for (int i = 0; i < 2000; ++i) {
		std::uint64_t m = (rng() >> 11) | 1u;         // odd, below 2^53: the exponent j is not reducible
		int j = static_cast<int>(rng() % 80);
		bool negative = (rng() & 1u) != 0;
		bool scientific = (rng() & 2u) != 0;
		std::string text = exactDecimal(m, j, negative, scientific);
		double v = std::ldexp(static_cast<double>(m), -j);
		Real want(negative ? -v : v), got;
		if (!got.parse(text) || !exactly(got, want)) {
			if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL '" << text << "' -> " << got.limbs().size() << " limbs, diff " << double(got - want) << '\n';
			++nrOfFailedTests;
		}
	}
	// the issue's example, and a few by hand
	const char* text[] = { "1.5", "-0.125", "0.0625", ".5", "2.5e-1", "15e-1", "1.50", "0.0000305175781250" };
	const double  val[] = { 1.5, -0.125, 0.0625, 0.5, 0.25, 1.5, 1.5, std::ldexp(1.0, -15) };
	for (std::size_t i = 0; i < sizeof(val) / sizeof(val[0]); ++i) {
		Real want(val[i]), got;
		if (!got.parse(text[i]) || !exactly(got, want)) {
			if (reportTestCases) std::cerr << "FAIL '" << text[i] << "' -> " << got.limbs().size() << " limbs, diff " << double(got - want) << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// 1 + m * 2^-j for j beyond a double's 53 bits: two limbs, exactly
int VerifyMultiLimb(bool reportTestCases) {
	using namespace sw::universal;
	using Real = ereal<8>;
	int nrOfFailedTests = 0;
	std::mt19937_64 rng(19320);
	for (int i = 0; i < 500; ++i) {
		std::uint64_t m = (rng() >> 44) | 1u;         // odd, 20 bits
		int j = 60 + static_cast<int>(rng() % 100);   // 2^-60 .. 2^-159 below the leading 1
		// the decimal of 1 + m * 2^-j is (2^j + m) * 5^j * 10^-j; write 2^j * 5^j = 10^j as a 1
		// followed by j zeros and add m * 5^j to it
		std::string tail = digitsOf(m);
		for (int k = 0; k < j; ++k) timesFive(tail);
		std::string frac(static_cast<std::size_t>(j) - tail.size(), '0');
		frac += tail;
		std::string text = "1." + frac;
		Real want = Real(1.0) + Real(std::ldexp(static_cast<double>(m), -j)), got;
		if (!got.parse(text) || !exactly(got, want)) {
			if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL 1 + " << m << "*2^-" << j << " -> " << got.limbs().size() << " limbs\n";
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// a decimal without a finite binary value is as accurate as before: N * 10^-k, measured in a
// wider ereal against the integer N
int VerifyNonDyadicAccuracy(bool reportTestCases) {
	using namespace sw::universal;
	using Real = ereal<8>;
	using Wide = ereal<19>;
	int nrOfFailedTests = 0;
	std::mt19937_64 rng(19321);
	const double tol = std::ldexp(1.0, -420);   // ereal<8> carries about 8 * 53 bits
	for (int i = 0; i < 500; ++i) {
		std::uint64_t N = rng() % 1000000000000000000ull + 1;
		if (i % 2 == 0) N = (N / 10) * 10 + 5;      // a trailing 5: the reduction applies, and stops short
		int k = static_cast<int>(rng() % 30) + 1;
		std::string text = digitsOf(N) + "e-" + std::to_string(k);
		Real got;
		if (!got.parse(text)) { ++nrOfFailedTests; continue; }
		Wide w(0);
		for (double limb : got.limbs()) w += Wide(limb);
		Wide scaled = w * pown(Wide(10.0), k);
		Wide n(0);
		for (char c : digitsOf(N)) n = n * Wide(10.0) + Wide(static_cast<double>(c - '0'));
		double rel = std::fabs(double((scaled - n) / n));
		if (!(rel <= tol)) {
			if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL '" << text << "' relative error " << rel << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// Truncation after the reduction can leave a positive power of ten together with halvings. The
// value (10^309 + 1) * 2^-300 is 4.9e218, but scaling its truncated digits by 10^166 before the
// halvings overflowed to inf (CodeRabbit on PR #1938). Its reference is 10^309 * 2^-300 =
// 5^309 * 2^9, which stays in range, and the dropped 2^-300 is far below ereal<8>'s precision.
int VerifyScaleOrder(bool reportTestCases) {
	using namespace sw::universal;
	using Real = ereal<8>;
	int nrOfFailedTests = 0;
	for (int extra : { 309, 305, 300 }) {          // 10^extra + 1, times 5^300, over 10^300
		std::string digits = "1" + std::string(static_cast<std::size_t>(extra - 1), '0') + "1";
		for (int k = 0; k < 300; ++k) timesFive(digits);
		Real got;
		if (!got.parse(digits + "e-300") || got.isinf() || got.isnan()) {
			if (reportTestCases) std::cerr << "FAIL (10^" << extra << " + 1) * 2^-300 is not finite\n";
			++nrOfFailedTests;
			continue;
		}
		Real ref = pown(Real(5.0), extra) * Real(std::ldexp(1.0, extra - 300));
		double rel = std::fabs(double((got - ref) / ref));
		if (!(rel <= std::ldexp(1.0, -420))) {
			if (reportTestCases) std::cerr << "FAIL (10^" << extra << " + 1) * 2^-300 relative error " << rel << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

} // namespace

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

	std::string test_suite  = "ereal exact parse of dyadic decimals";
	std::string test_tag    = "parse";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ereal<8> v;
	v.parse("1.5");
	std::cout << v.limbs().size() << " limbs, " << (v == ereal<8>(1.5)) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySingleLimb(reportTestCases), "ereal<8>", "dyadic, one limb");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiLimb(reportTestCases), "ereal<8>", "dyadic, two limbs");
	nrOfFailedTestCases += ReportTestResult(VerifyNonDyadicAccuracy(reportTestCases), "ereal<8>", "non-dyadic accuracy");
	nrOfFailedTestCases += ReportTestResult(VerifyScaleOrder(reportTestCases), "ereal<8>", "halvings before scaling");
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
