// exact_fractional.cpp: fmod, remainder, frac, fpclassify, exp(-inf) and tiny sqrt across cfloat layouts
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The engine-independent findings of #2088:
//   - fmod on a saturating cfloat never terminated: its doubling loop waited for an overflow to inf,
//     and maxpos + maxpos saturates to maxpos.  fmod is now an exact shift-and-subtract.
//   - remainder went through double (cfloat<80,11> remainder(1 + 2^-60, 1) was 0), and frac cast to
//     long long (undefined past 2^63, for NaN and inf; frac(1e19) was 7.77e17).
//   - fpclassify went through double: cfloat<128,15>'s minpos was FP_ZERO and its maxpos FP_INFINITE.
//   - exp and exp2 of -inf gave minpos.
//   - sqrt of a value below double's normal range took the host path through an inexact subnormal
//     double: cfloat<32,12> sqrt(1.25 * 2^-1073) was 12% off.
// References: for 16-bit layouts, std::fmod and std::remainder in double, which are exact and hold
// every value of the layout; for a 69-bit significand, integer arithmetic on values built as
// X * 2^-30 with X < 2^60.
#include <universal/utility/directives.hpp>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>

namespace {

// the exact result rounded into the layout -- which is the exact result, except in a layout
// without subnormals, where a result below the smallest normal rounds like any other value --
// with the same value and, for zeros, the same sign
template<typename Cfloat>
bool same(const Cfloat& got, double exact) {
	if (std::isnan(exact)) return got.isnan();
	if (got.isnan()) return false;
	const Cfloat ref(exact);
	if (!(got == ref)) return false;
	return !ref.iszero() || got.sign() == ref.sign();
}

// fmod and remainder of 16-bit layouts against double, over random pairs and the range ends
template<typename Cfloat>
int VerifyFmodRemainder16(unsigned samples, bool reportTestCases) {
	static_assert(Cfloat::nbits <= 16, "double holds every value and fmod/remainder are exact there");
	int fails = 0;
	std::mt19937_64 rng(2088);
	Cfloat x, y;
	auto check = [&](const Cfloat& a, const Cfloat& b) {
		if (a.isnan() || b.isnan() || a.isinf() || b.isinf()) return;
		const double da = double(a), db = double(b);
		const double rf = std::fmod(da, db), rr = std::remainder(da, db);
		const Cfloat f = fmod(a, b), r = remainder(a, b);
		if (!same(f, rf) || !same(r, rr)) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: " << da << " fmod/remainder " << db << " = " << double(f) << " / " << double(r)
			                                             << ", expected " << rf << " / " << rr << '\n';
			++fails;
		}
	};
	const Cfloat ends[] = { Cfloat(sw::universal::SpecificValue::maxpos), Cfloat(sw::universal::SpecificValue::minpos),
	                        Cfloat(sw::universal::SpecificValue::maxneg), Cfloat(3), Cfloat(-3), Cfloat(0.75), Cfloat(0) };
	for (const auto& a : ends) for (const auto& b : ends) check(a, b);
	for (unsigned k = 0; k < samples; ++k) {
		x.setbits(rng() & ((uint64_t(1) << Cfloat::nbits) - 1u));
		y.setbits(rng() & ((uint64_t(1) << Cfloat::nbits) - 1u));
		check(x, y);
	}
	return fails;
}

// a 69-bit-significand layout: x = +-X * 2^-30 with X < 2^60, y = B in [1, 2^20)
template<typename Cfloat>
int VerifyFmodRemainderWide(unsigned samples, bool reportTestCases) {
	static_assert(Cfloat::fbits >= 60, "X * 2^-30 must be exact");
	int fails = 0;
	std::mt19937_64 rng(2089);
	const Cfloat scale(std::ldexp(1.0, -30));
	for (unsigned k = 0; k < samples; ++k) {
		const uint64_t X = rng() >> (4u + rng() % 40u);          // < 2^60, spread over magnitudes
		const uint64_t B = 1u + (rng() >> 44);                  // [1, 2^20)
		const bool neg = (rng() & 1u) != 0;
		const uint64_t Y = B << 30;
		const uint64_t q = X / Y, R = X % Y;
		Cfloat x = Cfloat(static_cast<unsigned long long>(X)) * scale;
		if (neg) x = -x;
		const Cfloat y(static_cast<unsigned long long>(B));
		// expected fmod: +-R * 2^-30; expected remainder: R or R - Y by round-half-even of X / Y
		Cfloat ef = Cfloat(static_cast<unsigned long long>(R)) * scale;
		const bool down = (2u * R > Y) || (2u * R == Y && (q & 1u));
		Cfloat er = down ? Cfloat(static_cast<unsigned long long>(R)) * scale - y : ef;
		if (neg) { ef = -ef; er = -er; }
		const Cfloat f = fmod(x, y), r = remainder(x, y);
		if (!(f == ef) || !(r == er)) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: X " << X << " B " << B << ": fmod " << to_binary(f) << " expected " << to_binary(ef)
			                                             << ", remainder " << to_binary(r) << " expected " << to_binary(er) << '\n';
			++fails;
		}
	}
	return fails;
}

// fpclassify against the encoding's fields, over every encoding of a 16-bit layout
template<typename Cfloat>
int VerifyFpclassify16(bool reportTestCases) {
	int fails = 0;
	Cfloat c;
	for (uint64_t bits = 0; bits < (uint64_t(1) << Cfloat::nbits); ++bits) {
		c.setbits(bits);
		const uint64_t e = (bits >> Cfloat::fbits) & ((uint64_t(1) << Cfloat::es) - 1u);
		const uint64_t f = bits & ((uint64_t(1) << Cfloat::fbits) - 1u);
		int expected;
		if (c.isnan()) expected = FP_NAN;
		else if (c.isinf()) expected = FP_INFINITE;
		else if (e == 0) expected = (f == 0 || !Cfloat::hasSubnormals) ? FP_ZERO : FP_SUBNORMAL;
		else expected = FP_NORMAL;
		if (fpclassify(c) != expected) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: fpclassify(" << to_binary(c) << ") = " << fpclassify(c) << ", expected " << expected << '\n';
			++fails;
		}
	}
	return fails;
}

// sqrt below double's normal range on cfloat<32,12>, against a scaled double root.  The scaled
// value is a normal double, its root is correctly rounded, and rounding that to 20 bits once more
// is the correctly rounded root (53 >= 2 * 20 + 2).
int VerifyTinySqrt(unsigned samples, bool reportTestCases) {
	using C = sw::universal::cfloat<32, 12, uint32_t, true, false, false>;
	int fails = 0;
	std::mt19937_64 rng(2090);
	const C up(std::ldexp(1.0, 600));
	const C down(std::ldexp(1.0, -200));   // values below double's range are built in the cfloat, exactly
	for (unsigned k = 0; k < samples; ++k) {
		const C m(std::ldexp(1.0 + double(rng() >> 44) / double(1u << 20), -900 + static_cast<int>(rng() % 78u)));
		const C a = m * down;                                                   // 2^-1100 .. 2^-1023
		// a's exact value through scaling: a * 2^600 is a normal double
		const double scaled = double(a * up);
		const C ref(std::ldexp(std::sqrt(scaled), -300));
		const C got = sqrt(a);
		if (!(got == ref)) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: sqrt(" << to_binary(a) << ") = " << to_binary(got) << ", expected " << to_binary(ref) << '\n';
			++fails;
		}
	}
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

	std::string test_suite  = "cfloat exact fmod, remainder, frac, fpclassify";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Sat = cfloat<16, 5, uint16_t, true, false, true>;
	std::cout << "fmod(maxpos, 3) = " << fmod(Sat(SpecificValue::maxpos), Sat(3)) << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	{
		// the issue's values
		using Sat = cfloat<16, 5, uint16_t, true, false, true>;
		const Sat mp(SpecificValue::maxpos);
		if (double(fmod(mp, Sat(3))) != std::fmod(double(mp), 3.0)) {
			std::cerr << "FAIL: saturating fmod(maxpos, 3)\n";
			++nrOfFailedTestCases;
		}
		using W = cfloat<80, 11, uint32_t>;
		const W one(1), x = W(1) + W(std::ldexp(1.0, -60));
		if (!(remainder(x, one) == W(std::ldexp(1.0, -60)))) {
			std::cerr << "FAIL: cfloat<80,11> remainder(1 + 2^-60, 1) = " << to_binary(remainder(x, one)) << ", expected 2^-60\n";
			++nrOfFailedTestCases;
		}
		const W big(1e19), quarter = W(std::ldexp(1.0, 60)) + W(0.25);
		const bool fracOk = frac(big).iszero() && frac(quarter) == W(0.25) && frac(W(-2.75)) == W(0.75)
		                 && frac(W(SpecificValue::qnan)).isnan() && frac(W(SpecificValue::infpos)).iszero();
		if (!fracOk) {
			std::cerr << "FAIL: frac of 1e19, 2^60 + 0.25, -2.75, NaN or inf\n";
			++nrOfFailedTestCases;
		}
		using H = cfloat<16, 5, uint16_t, true, false, false>;
		const H ninf(SpecificValue::infneg);
		if (!exp(ninf).iszero() || !exp2(ninf).iszero() || !exp(H(-1000)).iszero()) {
			std::cerr << "FAIL: exp/exp2(-inf) or exp(-1000) on cfloat<16,5> is not 0\n";
			++nrOfFailedTestCases;
		}
		// double's own layout: its range is double's, so an underflow is 0
		using D = cfloat<64, 11, uint64_t, true, false, false>;
		if (!exp(D(-1000)).iszero() || !exp2(D(-2000)).iszero()) {
			std::cerr << "FAIL: exp(-1000) or exp2(-2000) on cfloat<64,11> is not 0\n";
			++nrOfFailedTestCases;
		}
		// a finite max-exponent value: 0x7c00 is 65536 on cfloat<16,5> with max-exponent values
		using M = cfloat<16, 5, uint16_t, true, true, false>;
		M m; m.setbits(0x7c00u);
		if (double(m) != 65536.0 || !frac(m).iszero() || !(trunc(m) == m) || double(fmod(m, M(3))) != std::fmod(65536.0, 3.0)) {
			std::cerr << "FAIL: frac, trunc or fmod of the max-exponent value 65536 on cfloat<16,5,uint16_t,t,t,f>\n";
			++nrOfFailedTestCases;
		}
		// es = 20, where the arithmetic type cannot add an exponent bit
		using E = cfloat<64, 20, uint64_t, true, false, false>;
		const E mx(SpecificValue::maxpos);
		const bool e20 = fmod(E(7), E(2)) == E(1) && remainder(E(7), E(2)) == E(-1) && remainder(E(5), E(2)) == E(1)
		              && fmod(mx, mx).iszero() && remainder(mx, mx).iszero()
		              && remainder(mx - (mx / E(4)), mx) == -(mx / E(4));   // 2r past maxpos: rounds toward -1/4 maxpos
		if (!e20) {
			std::cerr << "FAIL: fmod/remainder on cfloat<64,20,uint64_t>\n";
			++nrOfFailedTestCases;
		}
		using Q = cfloat<128, 15, uint32_t, true, false, false>;
		const bool classOk = fpclassify(Q(SpecificValue::minpos)) == FP_SUBNORMAL
		                  && fpclassify(Q(SpecificValue::maxpos)) == FP_NORMAL
		                  && fpclassify(Q(SpecificValue::infneg)) == FP_INFINITE
		                  && fpclassify(Q(SpecificValue::qnan)) == FP_NAN
		                  && fpclassify(Q(0)) == FP_ZERO
		                  && fpclassify(Q(1)) == FP_NORMAL;
		if (!classOk) {
			std::cerr << "FAIL: fpclassify on cfloat<128,15>\n";
			++nrOfFailedTestCases;
		}
		using T = cfloat<32, 12, uint32_t, true, false, false>;
		const T a = T(1.25) * T(std::ldexp(1.0, -873)) * T(std::ldexp(1.0, -200));   // 1.25 * 2^-1073, exactly
		const T ref(std::ldexp(std::sqrt(double(a * T(std::ldexp(1.0, 600)))), -300));
		if (!(sqrt(a) == ref)) {
			std::cerr << "FAIL: cfloat<32,12> sqrt(1.25 * 2^-1073) = " << double(sqrt(a) * T(std::ldexp(1.0, 600))) << " * 2^-600\n";
			++nrOfFailedTestCases;
		}
	}

	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainder16<cfloat<16, 5, uint16_t, true, false, true>>(20000, reportTestCases),  "cfloat<16,5,uint16_t,t,f,t>", "fmod/remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainder16<cfloat<16, 5, uint16_t, true, false, false>>(20000, reportTestCases), "cfloat<16,5,uint16_t,t,f,f>", "fmod/remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainder16<cfloat<16, 8, uint8_t, true, true, true>>(20000, reportTestCases),    "cfloat<16,8,uint8_t,t,t,t>",  "fmod/remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainder16<cfloat<12, 4, uint8_t, false, false, false>>(20000, reportTestCases), "cfloat<12,4,uint8_t,f,f,f>",  "fmod/remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainderWide<cfloat<80, 11, uint32_t>>(5000, reportTestCases),                   "cfloat<80,11,uint32_t>",      "fmod/remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainderWide<cfloat<96, 15, uint16_t, true, false, false>>(5000, reportTestCases), "cfloat<96,15,uint16_t,t,f,f>", "fmod/remainder");

	nrOfFailedTestCases += ReportTestResult(VerifyFpclassify16<cfloat<16, 5, uint16_t, true, false, false>>(reportTestCases),  "cfloat<16,5,uint16_t,t,f,f>", "fpclassify");
	nrOfFailedTestCases += ReportTestResult(VerifyFpclassify16<cfloat<16, 5, uint16_t, false, true, false>>(reportTestCases),  "cfloat<16,5,uint16_t,f,t,f>", "fpclassify");
	nrOfFailedTestCases += ReportTestResult(VerifyFpclassify16<cfloat<16, 8, uint8_t, true, true, true>>(reportTestCases),     "cfloat<16,8,uint8_t,t,t,t>",  "fpclassify");

	nrOfFailedTestCases += ReportTestResult(VerifyTinySqrt(2000, reportTestCases), "cfloat<32,12,uint32_t,t,f,f>", "sqrt below double");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainderWide<cfloat<128, 15, uint64_t, true, false, false>>(50000, reportTestCases), "cfloat<128,15,uint64_t,t,f,f>", "fmod/remainder");
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
