// exact_conversion.cpp: dfloat conversions from integers and doubles, and to double, round once (#2105)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each check here failed before #2105:
//   - an integer was loaded into the significand register, which is 36 bits for decimal32, so its high
//     bits were cut off: decimal32(34359738368) was 9.612604e10, decimal32(ULLONG_MAX) 1048575;
//   - a double was scaled by repeated / 10 and * 10 and only 17 digits were formed: decimal128(1e-5)
//     printed 0.000010000000000000002;
//   - dfloat -> double multiplied or divided by a double power of ten: double(decimal64("1e100")) was
//     1.0000000000000006e100, and double(decimal64("1e-310")) was 0;
//   - numeric_limits: epsilon() was 1, is_exact true, min() the smallest subnormal, the exponents off.
// References, independent of dfloat's conversion code:
//   - integer -> dfloat: the integer's decimal digits rounded half-to-even on the string;
//   - double -> dfloat: the double's exact decimal digits from integer<> arithmetic, rounded the same way;
//   - dfloat -> double: std::strtod of the dfloat's decimal string, which is correctly rounded.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>

namespace {

	// digits * 10^exp10 rounded half-to-even to n digits, as "D...DeX"
	std::string round_digits(std::string digits, long long exp10, unsigned n) {
		while (digits.size() > 1 && digits.front() == '0') digits.erase(digits.begin());
		if (digits.size() > n) {
			const std::size_t drop = digits.size() - n;
			const char first = digits[n];
			bool rest = false;
			for (std::size_t i = n + 1; i < digits.size(); ++i) rest = rest || (digits[i] != '0');
			std::string kept = digits.substr(0, n);
			const bool odd = ((kept.back() - '0') & 1) != 0;
			if (first > '5' || (first == '5' && (rest || odd))) {
				int i = static_cast<int>(n) - 1;
				while (i >= 0 && kept[static_cast<std::size_t>(i)] == '9') { kept[static_cast<std::size_t>(i)] = '0'; --i; }
				if (i >= 0) ++kept[static_cast<std::size_t>(i)];
				else { kept.insert(kept.begin(), '1'); kept.pop_back(); ++exp10; }
			}
			digits = kept;
			exp10 += static_cast<long long>(drop);
		}
		return digits + "e" + std::to_string(exp10);
	}

	// the exact decimal digits of a positive finite double, and the exponent of the last one
	std::string exact_digits(double x, long long& exp10) {
		using Big = sw::universal::integer<2700, std::uint32_t, sw::universal::IntegerNumberType::IntegerNumber>;
		std::uint64_t bits = 0;
		std::memcpy(&bits, &x, sizeof(bits));
		const std::uint64_t rawExp = (bits >> 52) & 0x7FFu;
		const std::uint64_t frac = bits & ((std::uint64_t(1) << 52) - 1u);
		const std::uint64_t m = (rawExp == 0) ? frac : ((std::uint64_t(1) << 52) | frac);
		const int E = (rawExp == 0) ? -1074 : static_cast<int>(rawExp) - 1075;
		Big N(static_cast<long long>(m));
		exp10 = 0;
		if (E > 0) N <<= E;
		else if (E < 0) {
			const int k = -E;   // m * 2^E = m * 5^k * 10^-k
			for (int i = 0; i < k; ++i) N *= Big(5);
			exp10 = E;
		}
		std::stringstream ss;
		ss << N;
		return ss.str();
	}

	template<typename D>
	int check_equal(const D& got, const std::string& expected, const char* what, bool reportTestCases) {
		const D ref(expected);
		if (got == ref) return 0;
		if (reportTestCases) std::cerr << "FAIL " << what << ": got " << sw::universal::components(got) << ", expected " << expected << '\n';
		return 1;
	}

	template<typename D>
	int VerifyIntegers(std::mt19937_64& rng, unsigned nrSamples, bool reportTestCases) {
		int fails = 0;
		auto one = [&](std::uint64_t v, bool negative) {
			D x;
			if (negative) x = -static_cast<long long>(v);
			else x = v;
			const std::string expected = (negative ? "-" : "") + round_digits(std::to_string(v), 0, D::ndigits);
			fails += check_equal(x, expected, "integer", reportTestCases);
		};
		for (std::uint64_t v : { std::uint64_t(34359738368ull), std::uint64_t(12345678901234ull), std::uint64_t(ULLONG_MAX),
		                         std::uint64_t(99999995ull), std::uint64_t(99999985ull), std::uint64_t(9999999999999999999ull) }) {
			one(v, false);
		}
		one(static_cast<std::uint64_t>(LLONG_MAX), true);
		{
			D x(LLONG_MIN);
			fails += check_equal(x, "-" + round_digits("9223372036854775808", 0, D::ndigits), "LLONG_MIN", reportTestCases);
		}
		for (unsigned k = 0; k < nrSamples; ++k) {
			const std::uint64_t v = rng() >> (rng() % 64u);
			one(v, false);
			if (v <= static_cast<std::uint64_t>(LLONG_MAX)) one(v, true);
		}
		return fails;
	}

	template<typename D>
	int VerifyFromDouble(std::mt19937_64& rng, unsigned nrSamples, bool reportTestCases) {
		int fails = 0;
		auto one = [&](double x) {
			long long exp10 = 0;
			const std::string digits = exact_digits(std::fabs(x), exp10);
			const std::string expected = (x < 0 ? "-" : "") + round_digits(digits, exp10, D::ndigits);
			fails += check_equal(D(x), expected, "double", reportTestCases);
		};
		for (double x : { 1e-5, 0.1, 1.0 / 3.0, 1e100, 1e-300, 5e-324, 1.7976931348623157e308, 2.5, 0.125, 123456789.0 }) {
			one(x);
		}
		for (unsigned k = 0; k < nrSamples; ++k) {
			double x = 0.0;
			const std::uint64_t bits = rng() & 0x7FEFFFFFFFFFFFFFull;   // finite, either sign via the top bit below
			std::memcpy(&x, &bits, sizeof(x));
			if (x == 0.0) continue;
			one((rng() & 1u) ? -x : x);
		}
		return fails;
	}

	template<typename D>
	int VerifyToDouble(std::mt19937_64& rng, int expLo, int expHi, unsigned nrSamples, bool reportTestCases) {
		int fails = 0;
		auto one = [&](const std::string& input) {
			const D x(input);
			// the dfloat's own value as text: the input may have more digits than ndigits
			bool s; int e; typename D::significand_t sig;
			x.unpack(s, e, sig);
			const std::string text = (s ? "-" : "") + D::sig_to_string(sig) + "e" + std::to_string(e);
			const double got = double(x);
			const double expected = x.isinf() ? (s ? -HUGE_VAL : HUGE_VAL) : std::strtod(text.c_str(), nullptr);
			if (got == expected && std::signbit(got) == std::signbit(expected)) return;
			++fails;
			if (reportTestCases) std::cerr << "FAIL double(" << text << ") = " << got << ", expected " << expected << '\n';
		};
		for (const char* t : { "1e100", "1e-310", "1e-320", "4.9e-324", "2.4703282292062328e-324", "1.7976931348623157e308", "1e-5", "0.1", "-7e22" }) {
			one(t);   // decimal32 rounds or overflows some of them, and the reference follows its value
		}
		for (unsigned k = 0; k < nrSamples; ++k) {
			std::string digits;
			for (unsigned i = 0; i < D::ndigits; ++i) digits += static_cast<char>('0' + rng() % 10u);
			if (digits[0] == '0') digits[0] = '1';
			const int e = expLo + static_cast<int>(rng() % static_cast<unsigned>(expHi - expLo + 1));
			one(((rng() & 1u) ? "-" : "") + digits + "e" + std::to_string(e));
		}
		return fails;
	}

	template<typename D>
	int VerifyLimits(int emin_sci, int emax_sci, bool reportTestCases) {
		using L = std::numeric_limits<D>;
		int fails = 0;
		auto expect = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL limits: " << what << '\n'; } };
		expect(L::epsilon() == D("1e" + std::to_string(1 - static_cast<int>(D::ndigits))), "epsilon is 10^(1 - ndigits)");
		expect(D(1) + L::epsilon() != D(1), "1 + epsilon is not 1");
		expect(L::min() == D("1e" + std::to_string(emin_sci)), "min is the smallest normal");
		expect(L::denorm_min() < L::min(), "denorm_min is below min");
		expect(L::min_exponent == emin_sci + 1 && L::min_exponent10 == emin_sci, "min_exponent, min_exponent10");
		expect(L::max_exponent == emax_sci + 1 && L::max_exponent10 == emax_sci, "max_exponent, max_exponent10");
		expect(!L::is_exact && L::has_denorm == std::denorm_present, "is_exact, has_denorm");
		return fails;
	}

}  // namespace

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

	std::string test_suite  = "dfloat exact conversion (#2105)";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	std::mt19937_64 rng(2105);
	nrOfFailedTestCases += ReportTestResult(VerifyIntegers<decimal32>(rng, 500, reportTestCases), "decimal32", "integer -> dfloat");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegers<decimal32_dpd>(rng, 200, reportTestCases), "decimal32_dpd", "integer -> dfloat");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegers<decimal64>(rng, 500, reportTestCases), "decimal64", "integer -> dfloat");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegers<decimal128>(rng, 200, reportTestCases), "decimal128", "integer -> dfloat");

	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<decimal32>(rng, 200, reportTestCases), "decimal32", "double -> dfloat");
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<decimal64>(rng, 200, reportTestCases), "decimal64", "double -> dfloat");
	nrOfFailedTestCases += ReportTestResult(VerifyFromDouble<decimal128>(rng, 200, reportTestCases), "decimal128", "double -> dfloat");

	nrOfFailedTestCases += ReportTestResult(VerifyToDouble<decimal32>(rng, -101, 90, 500, reportTestCases), "decimal32", "dfloat -> double");
	nrOfFailedTestCases += ReportTestResult(VerifyToDouble<decimal64>(rng, -340, 293, 1000, reportTestCases), "decimal64", "dfloat -> double");
	nrOfFailedTestCases += ReportTestResult(VerifyToDouble<decimal128>(rng, -360, 275, 500, reportTestCases), "decimal128", "dfloat -> double");

	nrOfFailedTestCases += ReportTestResult(VerifyLimits<decimal32>(-95, 96, reportTestCases), "decimal32", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<decimal64>(-383, 384, reportTestCases), "decimal64", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<decimal128>(-6143, 6144, reportTestCases), "decimal128", "numeric_limits");
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
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
