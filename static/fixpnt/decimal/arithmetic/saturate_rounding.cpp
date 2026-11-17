// saturate_rounding.cpp: dfixpnt Saturate range ends and the rounding of * and / (#2112)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <string>

#include <universal/number/dfixpnt/dfixpnt.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// The reference works on the raw value r, the dfixpnt being r * 10^-radix, in 64-bit integers:
// - Saturate clamps a magnitude past 10^ndigits - 1 to all nines, with the sign; Modulo keeps the
//   magnitude mod 10^ndigits, with the sign (sign-magnitude), and 0 is +0;
// - * and / round half away from zero on the magnitude, as the conversions do (#2112, decided):
//   before #2112 they truncated, 0.03 * 0.5 was 0.01 and 2 / 3 was 0.66.
// Before #2112 Saturate +, -, ++, -- and / wrapped (99.99 + 1 was 0.99), and so did the integer
// constructor (dfixpnt<4,2,Saturate>(12345) was 45.00).
template<unsigned ndigits>
constexpr long long pow10ll() { long long p = 1; for (unsigned i = 0; i < ndigits; ++i) p *= 10; return p; }

template<unsigned ndigits, bool arithmetic>
long long Policy(long long r) {
	constexpr long long top = pow10ll<ndigits>();
	const bool negative = r < 0;
	long long m = negative ? -r : r;
	if (m >= top) m = arithmetic ? (m % top) : (top - 1);
	return negative ? -m : m;
}

template<typename D>
D FromRaw(long long r) {
	D d(0);
	const bool negative = r < 0;
	long long m = negative ? -r : r;
	for (unsigned i = 0; i < D::ndigits; ++i) { d.setdigit(i, static_cast<unsigned>(m % 10)); m /= 10; }
	if (negative && !d.iszero()) d.setsign(true);
	return d;
}

template<typename D>
long long ToRaw(const D& d) {
	long long m = 0;
	for (unsigned i = D::ndigits; i-- > 0; ) m = m * 10 + d.digit(i);
	return d.sign() ? -m : m;
}

long long RoundHalfAway(long long num, long long den) {   // num / den, den != 0, rounded half away from zero
	const bool negative = (num < 0) != (den < 0);
	const long long n = num < 0 ? -num : num;
	const long long d = den < 0 ? -den : den;
	long long q = n / d;
	if (2 * (n % d) >= d) ++q;
	return negative ? -q : q;
}

template<typename D>
int VerifyArithmetic(unsigned nrSamples, bool reportTestCases) {
	constexpr unsigned ndigits = D::ndigits, radix = D::radix;
	constexpr bool arithmetic = D::arithmetic;
	constexpr long long top = pow10ll<ndigits>(), scale = pow10ll<radix>();
	int nrOfFailedTestCases = 0;
	std::uint64_t x = 0x9E3779B97F4A7C15ull;
	auto next = [&x]() { x ^= x << 13; x ^= x >> 7; x ^= x << 17; return x; };
	auto randomRaw = [&]() {
		long long r;
		switch (next() % 4) {
		case 0:  r = static_cast<long long>(next() % static_cast<std::uint64_t>(top)); break;               // anywhere
		case 1:  r = top - 1 - static_cast<long long>(next() % 50); break;                                  // near the top
		case 2:  r = static_cast<long long>(next() % static_cast<std::uint64_t>(scale * 10 + 1)); break;   // small
		default: r = scale * static_cast<long long>(next() % 10) + 5 * (scale / 10); break;                 // halves
		}
		r %= top;                                   // a representable value
		return (next() & 1u) ? -r : r;
	};
	auto check = [&](const D& got, long long expectedRaw, const char* op, long long a, long long b) {
		const D expected = FromRaw<D>(expectedRaw);
		if (got == expected && got.sign() == expected.sign()) return;
		++nrOfFailedTestCases;
		if (reportTestCases && nrOfFailedTestCases < 12) {
			std::cout << "FAIL: " << FromRaw<D>(a) << ' ' << op << ' ' << FromRaw<D>(b) << " = " << got << ", expected " << expected << '\n';
		}
	};
	for (unsigned k = 0; k < nrSamples; ++k) {
		const long long a = randomRaw(), b = randomRaw();
		const D da = FromRaw<D>(a), db = FromRaw<D>(b);
		check(da + db, Policy<ndigits, arithmetic>(a + b), "+", a, b);
		check(da - db, Policy<ndigits, arithmetic>(a - b), "-", a, b);
		check(da * db, Policy<ndigits, arithmetic>(RoundHalfAway(a * b, scale)), "*", a, b);
		if (b != 0) check(da / db, Policy<ndigits, arithmetic>(RoundHalfAway(a * scale, b)), "/", a, b);
		if constexpr (ndigits > radix) {             // ++ and -- step by 1, which needs an integer digit
			D inc(da);
			++inc;
			check(inc, Policy<ndigits, arithmetic>(a + scale), "++", a, scale);
			D dec(da);
			--dec;
			check(dec, Policy<ndigits, arithmetic>(a - scale), "--", a, scale);
		}
	}
	return nrOfFailedTestCases;
}

// the issue's cases, by value
template<DecimalEncoding E>
int VerifyIssueCases() {
	int fails = 0;
	auto check = [&fails](const std::string& got, const std::string& expected, const char* label) {
		if (got == expected) return;
		++fails;
		std::cout << "FAIL: " << label << " = " << got << ", expected " << expected << '\n';
	};
	using S = dfixpnt<4, 2, E, Saturate>;
	using S3 = dfixpnt<3, 3, E, Saturate>;
	using M = dfixpnt<4, 2, E, Modulo>;
	check((S(99.99) + S(1)).to_string(), "99.99", "Saturate 99.99 + 1");
	check((S(99.99) + S(99.99)).to_string(), "99.99", "Saturate 99.99 + 99.99");
	check((S(-99.99) - S(1)).to_string(), "-99.99", "Saturate -99.99 - 1");
	S m(SpecificValue::maxpos);
	++m;
	check(m.to_string(), "99.99", "Saturate ++maxpos");
	S n(SpecificValue::maxneg);
	--n;
	check(n.to_string(), "-99.99", "Saturate --maxneg");
	check((S(99.99) / S(0.01)).to_string(), "99.99", "Saturate 99.99 / 0.01");
	check((S3(0.5) / S3(0.25)).to_string(), "0.999", "Saturate 0.5 / 0.25");
	check(S(12345).to_string(), "99.99", "Saturate dfixpnt(12345)");
	check(S(-12345).to_string(), "-99.99", "Saturate dfixpnt(-12345)");
	check(S(12345u).to_string(), "99.99", "Saturate dfixpnt(12345u)");
	check(M(12345).to_string(), "45.00", "Modulo dfixpnt(12345)");
	check(M(-100).to_string(), "0.00", "Modulo dfixpnt(-100) is +0");
	check((M(0.03) * M(0.5)).to_string(), "0.02", "0.03 * 0.5 rounds half away");
	check((M(-0.03) * M(0.5)).to_string(), "-0.02", "-0.03 * 0.5 rounds half away");
	check((M(0.05) * M(0.1)).to_string(), "0.01", "0.05 * 0.1 rounds half away");
	check((M(2) / M(3)).to_string(), "0.67", "2 / 3 rounds to nearest");
	check((M(-2) / M(3)).to_string(), "-0.67", "-2 / 3 rounds to nearest");
	check((M(1) / M(8)).to_string(), "0.13", "1 / 8 = 0.125 rounds half away");
	check((M(0.01) / M(3)).to_string(), "0.00", "0.01 / 3 is +0");
	return fails;
}

}} // namespace sw::universal

// Regression testing guards
#define MANUAL_TESTING 0
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

	std::string test_suite  = "dfixpnt Saturate range and * / rounding (#2112)";
	std::string test_tag    = "saturate/rounding";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	using S = dfixpnt<4, 2, DecimalEncoding::BCD, Saturate>;
	std::cout << "99.99 + 1 = " << (S(99.99) + S(1)) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIssueCases<DecimalEncoding::BCD>(), test_tag, "issue cases, BCD");
	nrOfFailedTestCases += ReportTestResult(VerifyIssueCases<DecimalEncoding::BID>(), test_tag, "issue cases, BID");
	nrOfFailedTestCases += ReportTestResult(VerifyIssueCases<DecimalEncoding::DPD>(), test_tag, "issue cases, DPD");
	nrOfFailedTestCases += ReportTestResult((VerifyArithmetic<dfixpnt<4, 2, DecimalEncoding::BCD, Saturate>>(3000, reportTestCases)), test_tag, "dfixpnt<4,2,BCD,Saturate>");
	nrOfFailedTestCases += ReportTestResult((VerifyArithmetic<dfixpnt<4, 2, DecimalEncoding::BCD, Modulo>>(3000, reportTestCases)), test_tag, "dfixpnt<4,2,BCD,Modulo>");
	nrOfFailedTestCases += ReportTestResult((VerifyArithmetic<dfixpnt<4, 2, DecimalEncoding::BID, Saturate>>(3000, reportTestCases)), test_tag, "dfixpnt<4,2,BID,Saturate>");
	nrOfFailedTestCases += ReportTestResult((VerifyArithmetic<dfixpnt<4, 2, DecimalEncoding::DPD, Saturate>>(3000, reportTestCases)), test_tag, "dfixpnt<4,2,DPD,Saturate>");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyArithmetic<dfixpnt<6, 3, DecimalEncoding::BCD, Saturate>>(5000, reportTestCases)), test_tag, "dfixpnt<6,3,BCD,Saturate>");
	nrOfFailedTestCases += ReportTestResult((VerifyArithmetic<dfixpnt<6, 3, DecimalEncoding::DPD, Modulo>>(5000, reportTestCases)), test_tag, "dfixpnt<6,3,DPD,Modulo>");
	nrOfFailedTestCases += ReportTestResult((VerifyArithmetic<dfixpnt<8, 0, DecimalEncoding::BID, Saturate>>(5000, reportTestCases)), test_tag, "dfixpnt<8,0,BID,Saturate>");
	nrOfFailedTestCases += ReportTestResult((VerifyArithmetic<dfixpnt<5, 5, DecimalEncoding::BCD, Saturate>>(5000, reportTestCases)), test_tag, "dfixpnt<5,5,BCD,Saturate>");
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
