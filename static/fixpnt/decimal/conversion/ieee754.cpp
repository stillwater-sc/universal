// ieee754.cpp: dfixpnt conversion from and to float and double, exact or rounded once (#2076)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A double is exactly m * 2^e, so it has a finite decimal expansion. dfixpnt(double) is that value
// rounded once to radix fraction digits, to nearest with ties away from zero, and saturates at
// 10^idigits; double(dfixpnt) and float(dfixpnt) are the dfixpnt's value rounded once to nearest, ties to
// even. Before #2076 both directions went through double products: dfixpnt<50,40>(0.5) held
// 0.50000000000000000000009, dfixpnt<50,10>(2^60) lost its last four digits, and float() rounded twice.
//
// The references are independent of dfixpnt's conversion code: an exact rational in integer<> arithmetic
// (binary, not decimal) for random values, exact decimal digits of k * 5^j for the dyadic values, and
// hand-derived values for the ties, the subnormals and the ends of the range.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <universal/number/dfixpnt/dfixpnt.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	template<typename D>
	std::string digits_of(const D& d) {
		std::string s(d.sign() ? "-" : "+");
		for (unsigned i = D::ndigits; i-- > 0;) {
			if (i + 1u == D::radix) s += '.';
			s += static_cast<char>('0' + d.digit(i));
		}
		return s;
	}

	template<typename D>
	std::string tag_of() { return "dfixpnt<" + std::to_string(D::ndigits) + ',' + std::to_string(D::radix) + '>'; }

	// the dfixpnt whose magnitude digits are the decimal digits of q, shifted up by shift places
	template<typename D, typename I>
	D from_integer_digits(I q, unsigned shift, bool negative) {
		D d(0);
		const I ten(10);
		for (unsigned i = shift; !q.iszero(); ++i) {
			const I r = q % ten;
			d.setdigit(i, static_cast<unsigned>(static_cast<unsigned long long>(r)));
			q /= ten;
		}
		if (negative && !d.iszero()) d.setsign(true);
		return d;
	}

	template<typename D>
	D all_nines(bool negative) {
		D d(0);
		for (unsigned i = 0; i < D::ndigits; ++i) d.setdigit(i, 9);
		d.setsign(negative);
		return d;
	}

	// dfixpnt(x) by the definition, in integer<IW>: |x| = m 2^e, q = round(m 2^e 10^radix), ties away
	template<typename D, unsigned IW>
	D reference_from_double(double x) {
		using I = integer<IW, std::uint32_t, IntegerNumberType::IntegerNumber>;
		const bool negative = x < 0.0;
		int e = 0;
		const double f = std::frexp(std::fabs(x), &e);              // |x| = f 2^e, f in [1/2, 1)
		const auto m = static_cast<unsigned long long>(std::ldexp(f, 53));
		e -= 53;                                                    // |x| = m 2^e
		I num(m), ten(10), limit(1);
		for (unsigned i = 0; i < D::radix; ++i) num *= ten;
		for (unsigned i = 0; i < D::ndigits; ++i) limit *= ten;
		I q;
		if (e >= 0) {
			num <<= e;
			q = num;
		}
		else {
			I den(1);
			den <<= -e;
			q = num / den;
			const I r = num % den;
			if (!(r + r < den)) q += I(1);                          // ties away from zero
		}
		if (!(q < limit)) return all_nines<D>(negative);
		return from_integer_digits<D>(q, 0, negative);
	}

	// double(d) by the definition, in integer<IW>: |d| = N 10^-radix, scaled by 2^s into [2^62, 2^63), then
	// rounded to 53 bits, ties to even; for values in double's normal range
	template<typename D, unsigned IW>
	double reference_to_double(const D& d) {
		using I = integer<IW, std::uint32_t, IntegerNumberType::IntegerNumber>;
		if (d.iszero()) return 0.0;
		I N(0), ten(10), P(1);
		for (unsigned i = D::ndigits; i-- > 0;) N = N * ten + I(static_cast<long long>(d.digit(i)));
		for (unsigned i = 0; i < D::radix; ++i) P *= ten;
		I two62(1), two63(1);
		two62 <<= 62;
		two63 <<= 63;
		// a first guess from the leading digit, refined below: |d| lies in [10^(t - radix), 10^(t - radix + 1))
		unsigned t = 0;
		for (unsigned i = D::ndigits; i-- > 0;) if (d.digit(i) != 0) { t = i; break; }
		int s = 62 - static_cast<int>(std::floor((static_cast<double>(t) - static_cast<double>(D::radix)) * 3.321928094887362)) - 1;
		I Q, R;
		for (;;) {
			I num(N), den(P);
			if (s >= 0) num <<= s; else den <<= -s;
			Q = num / den;
			R = num % den;
			if (Q < two62) ++s;
			else if (!(Q < two63)) --s;
			else break;
		}
		auto q = static_cast<unsigned long long>(Q);
		const bool sticky = !R.iszero() || (q & 0x1FFull) != 0;      // the 9 bits below the round bit
		const bool round = ((q >> 9) & 1u) != 0;
		unsigned long long r = q >> 10;                              // 53 bits
		if (round && (sticky || (r & 1u) != 0)) ++r;
		const double v = std::ldexp(static_cast<double>(r), 10 - s);
		return d.sign() ? -v : v;
	}

	// a random double with a random exponent in [elo, ehi], and a random sign
	double random_double(std::mt19937_64& rng, int elo, int ehi) {
		const double f = static_cast<double>(rng() >> 11) * 0x1p-53;   // [0, 1)
		const int e = elo + static_cast<int>(rng() % static_cast<unsigned>(ehi - elo + 1));
		const double v = std::ldexp(0.5 + f / 2.0, e);
		return (rng() & 1u) ? -v : v;
	}

	// a random dfixpnt: random digits with a random number of the top ones zeroed, and a random sign
	template<typename D>
	D random_dfixpnt(std::mt19937_64& rng) {
		D d(0);
		const unsigned top = 1u + static_cast<unsigned>(rng() % D::ndigits);
		for (unsigned i = 0; i < top; ++i) d.setdigit(i, static_cast<unsigned>(rng() % 10u));
		if (!d.iszero() && (rng() & 1u)) d.setsign(true);
		return d;
	}

	// dfixpnt(double) against the integer<> reference, over random doubles with exponents in [elo, ehi]
	template<typename D, unsigned IW>
	int VerifyFromDouble(bool reportTestCases, int elo, int ehi, unsigned samples) {
		int fails = 0;
		std::mt19937_64 rng(2076);
		for (unsigned k = 0; k < samples; ++k) {
			const double x = random_double(rng, elo, ehi);
			const D got(x), want = reference_from_double<D, IW>(x);
			if (got != want || got.sign() != want.sign()) {
				if (reportTestCases && fails < 5) std::cerr << "FAIL: " << tag_of<D>() << '(' << std::hexfloat << x << std::defaultfloat
				                                            << ") = " << digits_of(got) << ", expected " << digits_of(want) << '\n';
				++fails;
			}
		}
		return fails;
	}

	// double(dfixpnt) against the integer<> reference, over random digits
	template<typename D, unsigned IW>
	int VerifyToDouble(bool reportTestCases, unsigned samples) {
		int fails = 0;
		std::mt19937_64 rng(2077);
		for (unsigned k = 0; k < samples; ++k) {
			const D d = random_dfixpnt<D>(rng);
			const double got = double(d), want = reference_to_double<D, IW>(d);
			if (got != want) {
				if (reportTestCases && fails < 5) std::cerr << "FAIL: double(" << digits_of(d) << ") = " << std::hexfloat << got
				                                            << ", expected " << want << std::defaultfloat << '\n';
				++fails;
			}
		}
		return fails;
	}

	// dyadic values k 2^-j are exact in any dfixpnt with radix >= j that holds their size, and convert back
	// exactly: their digits are those of k 5^j
	template<typename D>
	int VerifyDyadicExact(bool reportTestCases) {
		using I = integer<256, std::uint32_t, IntegerNumberType::IntegerNumber>;
		int fails = 0;
		std::mt19937_64 rng(2078);
		const unsigned jmax = (D::radix < 27u) ? D::radix : 27u;
		for (unsigned j = 0; j <= jmax; ++j) {
			for (unsigned t = 0; t < 40; ++t) {
				const unsigned long long k = (rng() >> 40) | 1u;    // odd, below 2^24
				const double x = std::ldexp(static_cast<double>(k), -static_cast<int>(j));
				I five_j(1);
				for (unsigned i = 0; i < j; ++i) five_j *= I(5);
				const D want = from_integer_digits<D>(I(k) * five_j, D::radix - j, false);
				const D got(x), gotneg(-x);
				if (got != want || gotneg != -want || double(got) != x || double(gotneg) != -x) {
					if (reportTestCases && fails < 5) std::cerr << "FAIL: " << tag_of<D>() << '(' << k << " * 2^-" << j << ") = " << digits_of(got)
					                                            << ", expected " << digits_of(want) << '\n';
					++fails;
				}
			}
		}
		return fails;
	}

	int VerifySpecialCases(bool reportTestCases) {
		int fails = 0;
		auto check = [&](bool ok, const std::string& what) {
			if (!ok) {
				if (reportTestCases) std::cerr << "FAIL: " << what << '\n';
				++fails;
			}
		};
		using W = dfixpnt<50, 40>;
		using N = dfixpnt<50, 10>;
		using S = dfixpnt<20, 10>;
		// the issue's examples
		check(digits_of(W(0.5)) == "+0000000000.5000000000000000000000000000000000000000", "dfixpnt<50,40>(0.5) is exact");
		check(digits_of(W(0.375)) == "+0000000000.3750000000000000000000000000000000000000", "dfixpnt<50,40>(0.375) is exact");
		check(digits_of(N(std::ldexp(1.0, 60))) == "+0000000000000000000001152921504606846976.0000000000", "dfixpnt<50,10>(2^60) is exact");
		check(digits_of(S(0.1)) == "+0000000000.1000000000", "dfixpnt<20,10>(0.1) rounds to 0.1");
		// 0.1 is 0.1000000000000000055511151231257827...: all of its 55 significant digits
		using T = dfixpnt<60, 55>;
		check(digits_of(T(0.1)) == "+00000.1000000000000000055511151231257827021181583404541015625", "dfixpnt<60,55>(0.1) is the exact double");
		// ties away from zero on the magnitude: 0.25 and 0.75 at one fraction digit, and their negatives
		using O = dfixpnt<4, 1>;
		check(O(0.25) == O(0.3) && O(-0.25) == O(-0.3) && O(0.75) == O(0.8) && O(-0.75) == O(-0.8), "dfixpnt<4,1> ties away from zero");
		check(O(0.05) == O(0.1), "dfixpnt<4,1>(0.05), just above the tie as a double, rounds up");
		// a magnitude that rounds to zero is +0
		check(!O(-0.01).sign() && O(-0.01).iszero(), "dfixpnt<4,1>(-0.01) is +0");
		check(!O(-std::numeric_limits<double>::denorm_min()).sign(), "dfixpnt<4,1>(-denorm_min) is +0");
		// saturation, infinities and NaN
		check(O(999.94) == all_nines<O>(false), "dfixpnt<4,1>(999.94) rounds to 999.9");
		check(O(999.95) == all_nines<O>(false) && O(-999.95) == all_nines<O>(true), "dfixpnt<4,1>(+-999.95) saturates");
		check(O(1e300) == all_nines<O>(false) && O(-1e300) == all_nines<O>(true), "dfixpnt<4,1>(+-1e300) saturates");
		check(O(std::numeric_limits<double>::infinity()) == all_nines<O>(false), "dfixpnt<4,1>(inf) saturates");
		check(O(-std::numeric_limits<double>::infinity()) == all_nines<O>(true), "dfixpnt<4,1>(-inf) saturates");
		check(O(std::numeric_limits<double>::quiet_NaN()).iszero(), "dfixpnt<4,1>(NaN) is 0");
		// double(): ties to even, and a value just past a tie rounds away from it; 2^-53 has 53 fraction
		// digits, so these need dfixpnt<60,55>
		const double t53 = std::ldexp(1.0, -53);
		T tie(1.0), above(0);
		tie += T(t53);                                              // 1 + 2^-53: halfway between 1 and 1 + 2^-52
		check(double(tie) == 1.0, "double(1 + 2^-53) ties to even: 1");
		above = tie;
		above.setdigit(0, 1);                                       // + 10^-55
		check(double(above) == 1.0 + 2.0 * t53, "double(1 + 2^-53 + 10^-55) rounds up");
		T tie3(1.0);
		tie3 += T(3.0 * t53);                                       // halfway between 1 + 2^-52 and 1 + 2^-51
		check(double(tie3) == 1.0 + 4.0 * t53, "double(1 + 3 2^-53) ties to even: 1 + 2^-51");
		// float(): rounded once. 1 + 2^-24 + 10^-40 is above the midpoint between 1 and 1 + 2^-23, but
		// through double it lands on the midpoint, and that ties down to 1
		W f(1.0);
		f += W(std::ldexp(1.0, -24));
		f.setdigit(0, 1);
		check(float(f) == 1.0f + std::ldexp(1.0f, -23), "float(1 + 2^-24 + 10^-40) rounds up, once");
		check(float(W(1.5)) == 1.5f && float(W(-0.375)) == -0.375f, "float of exact values");
		check(float(dfixpnt<50, 0>(std::ldexp(1.0, 128))) == std::numeric_limits<float>::infinity(), "float(2^128) is inf");
		// the subnormals, in a format that holds 2^-1074 exactly
		using Tiny = dfixpnt<1100, 1080>;
		const double dmin = std::numeric_limits<double>::denorm_min();
		const Tiny tmin(dmin);
		check(double(tmin) == dmin && tmin.digit(Tiny::radix - 1074u) != 0, "dfixpnt<1100,1080>(2^-1074) is exact");
		check(double(Tiny(std::ldexp(1.0, -1050) + std::ldexp(1.0, -1074))) == std::ldexp(1.0, -1050) + dmin, "a subnormal with its last bit");
		Tiny half_min(tmin), three_quarters(tmin);
		half_min *= Tiny(0.5);                                      // 2^-1075: halfway between 0 and 2^-1074
		three_quarters *= Tiny(0.75);
		check(double(half_min) == 0.0, "double(2^-1075) ties to even: 0");
		check(double(three_quarters) == dmin, "double(3 2^-1076) rounds up to 2^-1074");
		check(double(Tiny(std::numeric_limits<double>::min())) == std::numeric_limits<double>::min(), "the smallest normal");
		// wider than double's decimal exponent range: no longer rejected at compile time
		using Huge = dfixpnt<400, 10>;
		check(double(Huge(1.5)) == 1.5 && double(Huge(std::numeric_limits<double>::max())) == std::numeric_limits<double>::max(), "dfixpnt<400,10>: exact");
		Huge big(std::numeric_limits<double>::max());
		big *= Huge(2.0);
		check(double(big) == std::numeric_limits<double>::infinity(), "double beyond DBL_MAX is inf");
		check(double(-big) == -std::numeric_limits<double>::infinity(), "and -inf");
		return fails;
	}

	// the conversion from double stays usable at compile time
	constexpr dfixpnt<50, 40> cx_half(0.5);
	static_assert(cx_half.digit(39) == 5 && cx_half.digit(38) == 0 && cx_half.digit(0) == 0, "constexpr dfixpnt<50,40>(0.5) is exact");
	static_assert(double(cx_half) == 0.5, "constexpr double(dfixpnt) is exact");

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

	std::string test_suite  = "dfixpnt conversion from and to IEEE-754 (#2076)";
	std::string test_tag    = "ieee754";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases(reportTestCases), "special cases", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases(reportTestCases), "special cases", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyDyadicExact<dfixpnt<50, 40>>(reportTestCases)), "dyadic <50,40>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyDyadicExact<dfixpnt<40, 27, DecimalEncoding::DPD>>(reportTestCases)), "dyadic <40,27,dpd>", test_tag);
	// from double: narrow, where the value rounds, and wide, where it is exact or nearly so; both
	// include magnitudes that saturate
	nrOfFailedTestCases += ReportTestResult((VerifyFromDouble<dfixpnt<20, 10>, 384>(reportTestCases, -50, 40, 5000)), "from double <20,10>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyFromDouble<dfixpnt<8, 3, DecimalEncoding::BID, Saturate>, 384>(reportTestCases, -20, 20, 5000)), "from double <8,3,bid,sat>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyFromDouble<dfixpnt<50, 40>, 640>(reportTestCases, -160, 40, 2000)), "from double <50,40>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyFromDouble<dfixpnt<50, 10>, 640>(reportTestCases, -40, 140, 2000)), "from double <50,10>", test_tag);
	// to double
	nrOfFailedTestCases += ReportTestResult((VerifyToDouble<dfixpnt<20, 10>, 384>(reportTestCases, 5000)), "to double <20,10>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyToDouble<dfixpnt<50, 40>, 640>(reportTestCases, 2000)), "to double <50,40>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyToDouble<dfixpnt<60, 5, DecimalEncoding::DPD>, 640>(reportTestCases, 2000)), "to double <60,5,dpd>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyFromDouble<dfixpnt<20, 10>, 384>(reportTestCases, -50, 40, 100000)), "from double <20,10>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyToDouble<dfixpnt<20, 10>, 384>(reportTestCases, 100000)), "to double <20,10>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult((VerifyFromDouble<dfixpnt<50, 40>, 640>(reportTestCases, -160, 40, 50000)), "from double <50,40>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyToDouble<dfixpnt<50, 40>, 640>(reportTestCases, 50000)), "to double <50,40>", test_tag);
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
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
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
