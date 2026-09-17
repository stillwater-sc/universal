// decimal_to_binary_width.cpp: decimal text that needs more working bits than the default width
//
// This lives with the conversion tests rather than beside test_decimal_to_binary in static/utility
// because CI builds this directory: static/utility is on only in UNIVERSAL_BUILD_ALL, and every CI
// job builds CI_LITE, so the converter's own tests have never gated a merge -- which is part of how
// #1504 went unnoticed.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <universal/utility/decimal_to_binary.hpp>
#include <universal/verification/test_reporters.hpp>

namespace d2b = sw::universal::decimal_to_binary;

// The exact decimal expansion of 2^k, built digit by digit, so the oracle owes nothing to the
// converter under test: 2^k for k >= 0 is an integer, and 2^-k is 5^k shifted k places right.
std::string exact_power_of_two(int k) {
	std::vector<unsigned> digits{ 1u };   // least significant first
	const unsigned factor = (k >= 0) ? 2u : 5u;
	for (int i = 0; i < (k >= 0 ? k : -k); ++i) {
		unsigned carry = 0;
		for (unsigned& d : digits) {
			const unsigned v = d * factor + carry;
			d     = v % 10u;
			carry = v / 10u;
		}
		while (carry) { digits.push_back(carry % 10u); carry /= 10u; }
	}
	std::string s;
	if (k >= 0) {
		for (auto it = digits.rbegin(); it != digits.rend(); ++it) s.push_back(static_cast<char>('0' + *it));
		return s;
	}
	s = "0.";
	const std::size_t places = static_cast<std::size_t>(-k);   // 5^k scaled by 10^-k
	for (std::size_t z = digits.size(); z < places; ++z) s.push_back('0');
	for (auto it = digits.rbegin(); it != digits.rend(); ++it) s.push_back(static_cast<char>('0' + *it));
	return s;
}

// Every power of two in [2^lo, 2^hi], converted at a 53-bit target. A power of two has one bit set,
// so the answer is exact and checkable without encoding a double: the mantissa is 2^52 alone, no
// guard or sticky bit, and the binary scale is k.
//
// On main the conversion ran every input at 2048 working bits. The exact expansion of 2^-418 needs
// more, the shifts ran off the end of the integer, and the result came back valid and wrong: 657 of
// the 2098 powers of two in double's range, some as 0 and some as values like 3.7e+165 (#1504).
int VerifyPowersOfTwo(int lo, int hi, int step, bool reportTestCases) {
	int nrOfFailedTestCases = 0;
	for (int k = lo; k <= hi; k += step) {
		const std::string text = exact_power_of_two(k);
		const auto        r    = d2b::convert(text, 53);
		bool ok = r.valid && !r.is_zero && !r.negative && r.binary_scale == k && !r.guard_bit && !r.sticky_bit;
		if (ok) {
			for (unsigned i = 0; i < 53 && ok; ++i) ok = (r.mantissa.test(i) == (i == 52u));
		}
		if (!ok) {
			++nrOfFailedTestCases;
			if (reportTestCases)
				std::cerr << "FAIL: 2^" << k << " (" << text.size() << " chars): valid " << r.valid
				          << " scale " << r.binary_scale << " expected " << k << '\n';
			if (nrOfFailedTestCases > 9) return nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

// A short number with a huge exponent needs far more working bits than any rung, but every parser
// either saturates it or has the range to hold it, and both need its binary exponent. On main
// these came out right by accident, through bits falling off the end; they must still come out
// right. The reference exponents were computed with exact rational arithmetic, independently of
// the log2 estimate the converter uses.
int VerifyHugeExponents(bool reportTestCases) {
	struct Case { const char* text; std::int64_t scale; bool negative; };
	const Case cases[] = {
		{ "1e100000",         332192, false },
		{ "-1e100000",        332192, true  },
		{ "9.9e20000",         66441, false },
		{ "123456789e50000",  166123, false },
		{ "1e-100000",       -332193, false },
		{ "-1e-100000",      -332193, true  },
		{ "7e-15000",         -49827, false },
		{ "1.5e-20000",       -66438, false },
	};
	int nrOfFailedTestCases = 0;
	for (const auto& c : cases) {
		const auto r = d2b::convert(c.text, 53);
		const bool ok = r.valid && !r.is_zero && r.negative == c.negative && r.binary_scale == c.scale
		             && r.mantissa.test(52) && r.sticky_bit;   // normalized, and flagged as an estimate
		if (!ok) {
			++nrOfFailedTestCases;
			if (reportTestCases)
				std::cerr << "FAIL: " << c.text << ": valid " << r.valid << " scale " << r.binary_scale
				          << " expected " << c.scale << " sticky " << r.sticky_bit << '\n';
		}
	}
	// targets of 64 bits and more place the estimate by shifting a full 64-bit word up, and the
	// integer is signed: its top bit must not read as a sign (dd, qd and wide posits ask for these)
	for (unsigned target : { 64u, 106u, 212u }) {
		const auto r = d2b::convert("1e100000", target);
		bool ok = r.valid && r.binary_scale == 332192 && !r.mantissa.sign();
		for (unsigned i = target; i < target + 8u && ok; ++i) ok = !r.mantissa.test(i);
		ok = ok && r.mantissa.test(target - 1u);
		if (!ok) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 1e100000 at a " << target << "-bit target is not normalized\n";
		}
	}

	// all zero digits with a huge exponent is still zero
	const auto z = d2b::convert("0.000e100000", 53);
	if (!(z.valid && z.is_zero)) {
		++nrOfFailedTestCases;
		if (reportTestCases) std::cerr << "FAIL: 0.000e100000 is not a valid zero\n";
	}
	return nrOfFailedTestCases;
}

// A digit string too long to convert exactly, with an ordinary exponent, may well be representable,
// so an estimate would be exactly the silent wrong answer this is about: on main a 12,001-digit
// 1.333... came back as 0. It has to be refused, so the parse fails.
int VerifyLongDigitStringsRefused(bool reportTestCases) {
	int nrOfFailedTestCases = 0;
	const std::string text = "1." + std::string(12000, '3');
	const auto r = d2b::convert(text, 53);
	if (r.valid) {
		++nrOfFailedTestCases;
		if (reportTestCases)
			std::cerr << "FAIL: a 12,001-digit 1.333... was accepted, scale " << r.binary_scale << '\n';
	}
	return nrOfFailedTestCases;
}

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

	std::string test_suite  = "decimal_to_binary working width";
	std::string test_tag    = "decimal_to_binary";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	const std::string text = exact_power_of_two(-600);
	const auto r = d2b::convert(text, 53);
	std::cout << "2^-600 : " << text.size() << " chars, valid " << r.valid << ", scale " << r.binary_scale << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// the region where main first broke, every power; then the whole of double's range, sampled
	nrOfFailedTestCases += ReportTestResult(VerifyPowersOfTwo(-440, -400, 1, reportTestCases), "2^-440..2^-400", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyPowersOfTwo(-1074, 1023, 17, reportTestCases), "2^-1074..2^1023 sampled", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyHugeExponents(reportTestCases), "huge exponents", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLongDigitStringsRefused(reportTestCases), "long digit strings", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// every power of two in double's range
	nrOfFailedTestCases += ReportTestResult(VerifyPowersOfTwo(-1074, 1023, 1, reportTestCases), "2^-1074..2^1023", test_tag);
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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
