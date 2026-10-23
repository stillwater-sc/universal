// pow.cpp: regression tests for efloat mathematical power and exponential functions.
//
// Categories tested:
//   - integer_power, pow, exp2, exp10, expm1
//   - pow(x, n) for an integer n, correctly rounded in every mode against x^n formed exactly
//     (#1804): random P-bit x, negative n, and x next to 1 whose power forces Ziv's widening;
//     an argument found by search whose first try rounds wrong, powers past the exponent's
//     range, and a y wider than x
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	int VerifyEfloatPower(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. integer_power & pow(x, int_y) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying integer_power...\n";
		{
			efloat<4> base(2.0);
			if (pow(base, 3) != 8.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(2.0, 3) is not 8.0. Result: " << double(pow(base, 3)) << "\n";
				++failures;
			}
			if (pow(base, -2) != 0.25) {
				if (reportTestCases) std::cout << "    FAIL: pow(2.0, -2) is not 0.25. Result: " << double(pow(base, -2)) << "\n";
				++failures;
			}
			if (pow(base, 0) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(2.0, 0) is not 1.0\n";
				++failures;
			}
			// Verify INT_MIN safe negation (CodeRabbit feedback)
			efloat<4> neg_one(-1.0);
			int min_int = std::numeric_limits<int>::min();
			efloat<4> res_min_int = pow(neg_one, min_int);
			if (res_min_int != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(-1.0, INT_MIN) is not 1.0. Result: " << double(res_min_int) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. pow(x, y) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying pow(x, y)...\n";
		{
			// x^y positive base fractional exponent: pow(4.0, 0.5) == 2.0
			efloat<4> base(4.0);
			efloat<4> exp_val(0.5);
			double d_pow = double(pow(base, exp_val));
			if (std::abs(d_pow - 2.0) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: pow(4.0, 0.5) is not 2.0. Result: " << d_pow << "\n";
				++failures;
			}

			// Negative base integer exponent: pow(-2.0, 3.0) == -8.0
			efloat<4> neg_base(-2.0);
			efloat<4> odd_exp(3.0);
			double d_neg_pow = double(pow(neg_base, odd_exp));
			if (std::abs(d_neg_pow - (-8.0)) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: pow(-2.0, 3.0) is not -8.0. Result: " << d_neg_pow << "\n";
				++failures;
			}

			// Negative base fractional exponent (should yield NaN)
			efloat<4> frac_exp(1.5);
			efloat<4> res_frac = pow(neg_base, frac_exp);
			if (!res_frac.isnan()) {
				if (reportTestCases) std::cout << "    FAIL: pow(-2.0, 1.5) did not return NaN\n";
				++failures;
			}

			// Zero base negative exponent (Divide by Zero Exception, yields Inf)
			clear_efloat_exceptions();
			efloat<4> zero(0.0);
			efloat<4> neg_exp(-2.0);
			efloat<4> res_zero = pow(zero, neg_exp);
			if (!res_zero.isinf()) {
				if (reportTestCases) std::cout << "    FAIL: pow(0.0, -2.0) did not return Inf\n";
				++failures;
			}
			if (!has_efloat_exception(ExceptionFlag::DivisionByZero)) {
				if (reportTestCases) std::cout << "    FAIL: pow(0.0, -2.0) did not raise DivisionByZero exception\n";
				++failures;
			}

			// IEEE-754 Boundary cases (CodeRabbit feedback)
			efloat<4> nan; nan.setnan();
			efloat<4> one(1.0);
			if (pow(nan, zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(NaN, 0.0) is not 1.0\n";
				++failures;
			}
			if (pow(one, nan) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: pow(1.0, NaN) is not 1.0\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. exp2 Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying exp2...\n";
		{
			efloat<4> exponent(2.0);
			double d_exp2 = double(exp2(exponent));
			if (std::abs(d_exp2 - 4.0) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: exp2(2.0) is not 4.0. Result: " << d_exp2 << "\n";
				++failures;
			}
			// Zero input check
			efloat<4> zero(0.0);
			if (exp2(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: exp2(0.0) is not 1.0. Result: " << double(exp2(zero)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 4. exp10 Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying exp10...\n";
		{
			efloat<4> exponent(2.0);
			double d_exp10 = double(exp10(exponent));
			if (std::abs(d_exp10 - 100.0) > 1e-12) {
				if (reportTestCases) std::cout << "    FAIL: exp10(2.0) is not 100.0. Result: " << d_exp10 << "\n";
				++failures;
			}
			// Zero input check
			efloat<4> zero(0.0);
			if (exp10(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: exp10(0.0) is not 1.0. Result: " << double(exp10(zero)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 5. expm1 Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying expm1...\n";
		{
			efloat<4> zero(0.0);
			if (expm1(zero) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: expm1(0.0) is not 0.0\n";
				++failures;
			}

			// Small input Taylor series check (cancellation prevention)
			// expm1(1e-12) ~= 1.0000000000005e-12
			efloat<4> small(1e-12);
			efloat<4> res_expm1 = expm1(small);
			double d_res = double(res_expm1);
			if (std::abs(d_res - 1e-12) > 1e-20) {
				if (reportTestCases) std::cout << "    FAIL: expm1(1e-12) was not accurate. Result: " << d_res << "\n";
				++failures;
			}
		}

		return failures;
	}

	// pow(x, n) and pow(x, -n) for P-bit x against x^n formed exactly and rounded once, in every
	// mode (#1804). The integer path powers on limbs with bit_width(n) + 21 guard bits and
	// widens when Ziv's test fails; x = 1 + 2^-j + 2^-(P - 2), j past P / 2 + 40, makes it fail,
	// since x^n's bits past P are 0 until about bit 2j. The reciprocal's reference divides at
	// 4P + 512 bits more than the exact power and rounds again: a directed rounding repeated in
	// its direction is the single rounding, and to nearest the wide quotient would have to lie
	// within 2^-(4P + 512) of a midpoint, which a quotient that is not a binary fraction does not.
	template<unsigned nlimbs>
	int VerifyIntegerPowerRounding(bool reportTestCases, unsigned P, int count) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		auto same = [](const E& a, const E& b) { return a == b && a.sign() == b.sign() && a.get_precision() == b.get_precision(); };
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		std::mt19937_64 rng(1804);
		for (int i = 0; i < count; ++i) {
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			E x(1);
			x.set_precision(P);
			for (unsigned j = 0; j < (P + 31) / 32; ++j) x.setlimb(j, static_cast<uint32_t>(rng()) | (j == 0 ? 0x80000000u : 0u));
			x.set_precision(P);
			x.setexponent(static_cast<int64_t>(rng() % 9) - 4);
			int n = 2 + static_cast<int>(rng() % 40);
			const int64_t jmin = static_cast<int64_t>(P) / 2 + 41;
			if (i % 4 == 1 && jmin <= static_cast<int64_t>(P) - 4) {   // next to a rounding boundary
				const int64_t j = jmin + static_cast<int64_t>(rng() % static_cast<uint64_t>(static_cast<int64_t>(P) - 3 - jmin));
				E t(1.0), u(1.0);
				t.setexponent(-j);
				u.setexponent(-static_cast<int64_t>(P) + 2);
				x = E(1);
				x.set_precision(P);
				x = (rng() & 1) ? x + t + u : x - t + u;
				n = 3 + static_cast<int>(rng() % 7);
			}
			if (rng() & 1) x = -x;
			const int k = (rng() & 1) ? n : -n;
			// x^n exactly: n P bits and a margin hold every partial product
			const unsigned width = static_cast<unsigned>(n) * P + 64u;
			E exact(x);
			exact.set_precision(width);
			E power(exact);
			for (int m = 1; m < n; ++m) power *= exact;
			for (RoundingMode mode : modes) {
				efloat_rounding_mode = mode;
				E want;
				if (k > 0) {
					want = power;
				}
				else {
					E one(1);
					one.set_precision(width + 4 * P + 512);
					want = one / power;
				}
				want.set_precision(P);
				const E got_int = pow(x, k);
				const E got_efloat = pow(x, E(k));
				if (!same(got_int, want) || !same(got_efloat, want)) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: pow(" << double(x) << ", " << k << ") at " << P << " bits, mode " << int(mode) << '\n';
					++failures;
				}
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// x^n exactly, then rounded once at P bits in the current mode; a reciprocal as above
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> integer_power_reference(const sw::universal::efloat<nlimbs>& x, int k, unsigned P) {
		using E = sw::universal::efloat<nlimbs>;
		const int n = (k < 0) ? -k : k;
		const unsigned width = static_cast<unsigned>(n) * x.get_precision() + 64u;
		E exact(x);
		exact.set_precision(width);
		E power(exact);
		for (int m = 1; m < n; ++m) power *= exact;
		E want;
		if (k > 0) {
			want = power;
		}
		else {
			E one(1);
			one.set_precision(width + 4 * P + 512);
			want = one / power;
		}
		want.set_precision(P);
		return want;
	}

	// pow(x, n) where the integer path's first try fails Ziv's test and rounds wrong without it
	// (found by search, about one 32-bit argument in 10^8), powers far past the exponent's range,
	// where the path must not run, and a y that carries a wider precision than x (#1804)
	int VerifyIntegerPowerEdges(bool reportTestCases) {
		using namespace sw::universal;
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		{
			using E = efloat<1>;
			E x(1);
			x.set_precision(32);
			x.setlimb(0, 0xbc2b2e23u);   // x^31 lies within 2^-58 of a midpoint at 32 bits
			x.setexponent(0);
			for (RoundingMode mode : modes) {
				efloat_rounding_mode = mode;
				for (int k : { 31, -31 }) {
					const E want = integer_power_reference(x, k, 32);
					if (!(pow(x, k) == want) || !(pow(-x, k) == integer_power_reference(-x, k, 32))) {
						if (reportTestCases && failures < 8) std::cout << "    FAIL: pow(0xbc2b2e23 2^-31, " << k << "), mode " << int(mode) << '\n';
						++failures;
					}
				}
			}
		}
		{
			using E = efloat<4>;
			for (RoundingMode mode : modes) {
				efloat_rounding_mode = RoundingMode::RoundToNearest;
				E x(1.5);
				x.set_precision(128);
				x.setexponent(int64_t(1) << 50);   // x^(2^20): an exponent of 2^70, past int64_t
				efloat_rounding_mode = mode;
				const E up = pow(x, 1 << 20);
				const E down = pow(x, -(1 << 20));
				const bool up_ok   = up.isinf() || (up.sign() == 1 && up.scale() == E::max_scale);
				const bool down_ok = down.iszero() || (down.sign() == 1 && down.scale() == E::min_scale);
				if (!up_ok || !down_ok) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: pow(1.5 2^(2^50), +-2^20) did not overflow and underflow, mode " << int(mode) << '\n';
					++failures;
				}
			}
		}
		{
			using E = efloat<8>;
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			E x(1);
			x.set_precision(128);
			for (unsigned j = 0; j < 4; ++j) x.setlimb(j, 0x9e3779b9u + 0x7f4a7c15u * j);
			x.setexponent(0);
			E y(7);
			y.set_precision(256);   // the result takes y's wider precision
			for (RoundingMode mode : modes) {
				efloat_rounding_mode = mode;
				const E got = pow(x, y);
				const E want = integer_power_reference(x, 7, 256);
				if (!(got == want) || got.get_precision() != 256) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: pow(x, y) with y at 256 bits, mode " << int(mode) << '\n';
					++failures;
				}
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat mathematical power library";
	std::string test_tag            = "pow";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatPower(reportTestCases), "efloat", "pow manual");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowerRounding<4>(reportTestCases, 128, 20), "efloat<4>", "pow(x, n)");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatPower(reportTestCases), "efloat", "pow foundational");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowerRounding<4>(reportTestCases, 128, 200), "efloat<4>", "pow(x, n)");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowerRounding<3>(reportTestCases, 96, 200), "efloat<3>", "pow(x, n)");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowerRounding<1>(reportTestCases, 32, 200), "efloat<1>", "pow(x, n)");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowerEdges(reportTestCases), "efloat", "pow(x, n) edges");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowerRounding<16>(reportTestCases, 512, 60), "efloat<16>", "pow(x, n)");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowerRounding<5>(reportTestCases, 160, 100), "efloat<5>", "pow(x, n)");
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerPowerRounding<64>(reportTestCases, 2048, 10), "efloat<64>", "pow(x, n)");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
