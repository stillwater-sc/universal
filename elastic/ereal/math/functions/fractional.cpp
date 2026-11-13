// fractional.cpp: test suite runner for fractional functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <random>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/ereal_precision.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace {
	using namespace sw::universal;

		// fmod, remainder and modf are exact: every result here is checked for exact equality
		// of the dyadic values (ereal_precision.hpp), never through double, where a tolerance
		// of 1e-10 passed a result correct to 53 bits (#2008).  std::fmod and std::remainder
		// are exact too, so on double operands they are an independent exact reference.
		template<typename Real>
		bool exactly(const Real& a, const Real& b) { return agreed_bits_absolute(a, b) == kExactAgreement; }

		// a zero with the given sign
		template<typename Real>
		bool is_signed_zero(const Real& v, bool negative) { return v.iszero() && (v.signbit() == negative); }

		// Verify fmod function
		template<typename Real>
		int VerifyFmod(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: fmod property x - fmod(x,y) = n*y where n = trunc(x/y)
			Real x(5.3), y(2.0);
			Real result = fmod(x, y);
			Real n = trunc(x / y);
			Real expected = x - (n * y);

			if (!exactly(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL fmod(5.3, 2.0) property violation\n";
				++nrOfFailedTestCases;
			}
			if (!exactly(result, Real(std::fmod(5.3, 2.0)))) {
				if (reportTestCases) std::cout << "    FAIL fmod(5.3, 2.0) != std::fmod\n";
				++nrOfFailedTestCases;
			}

			// exact for any quotient size: trunc(x / y) holds only the type's precision, the
			// exact reduction does not.  fmod(1e200, 3) is 2.
			{
				const double big[][2] = { {1.0e200, 3.0}, {-1.0e200, 3.0}, {1.0e300, 7.0e-300}, {1.0e18, 0.1} };
				for (const auto& c : big) {
					Real r = fmod(Real(c[0]), Real(c[1]));
					if (!exactly(r, Real(std::fmod(c[0], c[1])))) {
						if (reportTestCases) std::cout << "    FAIL fmod(" << c[0] << ", " << c[1] << ") != std::fmod = " << std::fmod(c[0], c[1]) << '\n';
						++nrOfFailedTestCases;
					}
				}
			}

			// a dividend past double's precision: fmod(1 + 2^-300, 0.75) == 0.25 + 2^-300
			{
				Real tiny = ldexp(Real(1.0), -300);
				Real r = fmod(Real(1.0) + tiny, Real(0.75));
				if (!exactly(r, Real(0.25) + tiny)) {
					if (reportTestCases) std::cout << "    FAIL fmod(1 + 2^-300, 0.75) != 0.25 + 2^-300\n";
					++nrOfFailedTestCases;
				}
			}

			// Test: fmod sign property (same sign as dividend)
			Real pos_result = fmod(Real(5.3), Real(2.0));
			Real neg_result = fmod(Real(-5.3), Real(2.0));

			if (!pos_result.ispos()) {
				if (reportTestCases) std::cout << "    FAIL fmod(5.3, 2.0) should be positive\n";
				++nrOfFailedTestCases;
			}
			if (!neg_result.isneg()) {
				if (reportTestCases) std::cout << "    FAIL fmod(-5.3, 2.0) should be negative\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify modf: split into fractional part (returned) and integer part (*iptr)
		template<typename Real>
		int VerifyModf(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			// {input, expected integer part, expected fractional part}
			const double cases[][3] = {
				{ 3.75, 3.0, 0.75}, {-3.75, -3.0, -0.75}, {0.25, 0.0, 0.25},
				{-0.25, 0.0, -0.25}, {5.0, 5.0, 0.0}, {-2.0, -2.0, -0.0}, {-3.0, -3.0, -0.0},
			};
			for (const auto& c : cases) {
				Real ip;
				Real fr = modf(Real(c[0]), &ip);
				if (ip != Real(c[1]) || fr != Real(c[2])) {
					if (reportTestCases) std::cout << "    FAIL modf(" << c[0] << ") int=" << double(ip)
					                               << " frac=" << double(fr) << " expected int=" << c[1] << " frac=" << c[2] << "\n";
					++nrOfFailedTestCases;
				}
				// the fraction has the sign of x, also when it is zero: modf(-3) is -0 (C99 F.10.3.12)
				if (fr.signbit() != std::signbit(c[0])) {
					if (reportTestCases) std::cout << "    FAIL modf(" << c[0] << ") fraction has the wrong sign\n";
					++nrOfFailedTestCases;
				}
				// identity: int + frac == x
				if (!exactly(ip + fr, Real(c[0]))) {
					if (reportTestCases) std::cout << "    FAIL modf(" << c[0] << ") int+frac != x\n";
					++nrOfFailedTestCases;
				}
			}
			return nrOfFailedTestCases;
		}

		// Verify remainder function with IEEE round-to-nearest-even
		template<typename Real>
		int VerifyRemainder(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test 1: Normal case (no tie)
			// 7/3 = 2.333... -> rounds to 2 -> 7 - 2*3 = 1
			{
				Real x(7.0), y(3.0);
				Real result = remainder(x, y);
				Real expected(1.0);
				if (!exactly(result, expected)) {
					if (reportTestCases) std::cout << "    FAIL remainder(7.0, 3.0) = "
						<< double(result) << ", expected 1.0\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 2: IEEE round-to-nearest-even - tie case (floor_q is even)
			// 5/2 = 2.5 -> tie, floor=2 (even), ceil=3 (odd) -> choose 2 (even)
			// 5 - 2*2 = 1
			{
				Real x(5.0), y(2.0);
				Real result = remainder(x, y);
				Real expected(1.0);
				if (!exactly(result, expected)) {
					if (reportTestCases) std::cout << "    FAIL remainder(5.0, 2.0) = "
						<< double(result) << ", expected 1.0 (rounds 2.5 to 2 even)\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 3: IEEE round-to-nearest-even - tie case (ceil_q is even)
			// 7/2 = 3.5 -> tie, floor=3 (odd), ceil=4 (even) -> choose 4 (even)
			// 7 - 4*2 = -1
			{
				Real x(7.0), y(2.0);
				Real result = remainder(x, y);
				Real expected(-1.0);
				if (!exactly(result, expected)) {
					if (reportTestCases) std::cout << "    FAIL remainder(7.0, 2.0) = "
						<< double(result) << ", expected -1.0 (rounds 3.5 to 4 even)\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 4: Negative values
			// -7/3 = -2.333... -> rounds to -2 -> -7 - (-2)*3 = -1
			{
				Real x(-7.0), y(3.0);
				Real result = remainder(x, y);
				Real expected(-1.0);
				if (!exactly(result, expected)) {
					if (reportTestCases) std::cout << "    FAIL remainder(-7.0, 3.0) = "
						<< double(result) << ", expected -1.0\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 5: Exact division should give zero
			// 9/3 = 3.0 (exact) -> 9 - 3*3 = 0
			{
				Real x(9.0), y(3.0);
				Real result = remainder(x, y);
				Real expected(0.0);
				if (!exactly(result, expected)) {
					if (reportTestCases) std::cout << "    FAIL remainder(9.0, 3.0) = "
						<< double(result) << ", expected 0.0\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 6: Result in range [-|y|/2, |y|/2]
			// 10/3 = 3.333... -> rounds to 3 -> 10 - 3*3 = 1
			{
				Real x(10.0), y(3.0);
				Real result = remainder(x, y);
				Real expected(1.0);
				Real y_half = y / Real(2.0);

				if (!exactly(result, expected)) {
					if (reportTestCases) std::cout << "    FAIL remainder(10.0, 3.0) = "
						<< double(result) << ", expected 1.0\n";
					++nrOfFailedTestCases;
				}

				// Verify result is in range [-1.5, 1.5]
				if (abs(result) > y_half) {
					if (reportTestCases) std::cout << "    FAIL remainder(10.0, 3.0) out of range [-1.5, 1.5]\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 7: Another tie case with negative result
			// 9/4 = 2.25 -> rounds to 2 -> 9 - 2*4 = 1
			// 11/4 = 2.75 -> rounds to 3 -> 11 - 3*4 = -1
			{
				Real x(11.0), y(4.0);
				Real result = remainder(x, y);
				Real expected(-1.0);
				if (!exactly(result, expected)) {
					if (reportTestCases) std::cout << "    FAIL remainder(11.0, 4.0) = "
						<< double(result) << ", expected -1.0\n";
					++nrOfFailedTestCases;
				}
			}

			return nrOfFailedTestCases;
		}

		// fmod(x, 0) and remainder(x, 0): NaN in the default build, as C99 F.10.7.1/F.10.7.2
		// ask (#2006; they used to throw ereal_divide_by_zero in every build).  Under
		// EREAL_THROW_ARITHMETIC_EXCEPTION they throw ereal_divide_by_zero, as x / 0 does.
		template<typename Real>
		int VerifyDivisionByZero(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const Real x(5.0), zero(0.0);
			for (int op = 0; op < 2; ++op) {
				const char* name = (op == 0) ? "remainder(5.0, 0.0)" : "fmod(5.0, 0.0)";
#if EREAL_THROW_ARITHMETIC_EXCEPTION
				bool caught_exception = false;
				try {
					Real result = (op == 0) ? remainder(x, zero) : fmod(x, zero);
					(void)result;
				}
				catch (const ereal_divide_by_zero&) {
					caught_exception = true;
				}
				catch (...) {
					if (reportTestCases) std::cout << "    FAIL " << name << " threw wrong exception type\n";
					++nrOfFailedTestCases;
					continue;
				}
				if (!caught_exception) {
					if (reportTestCases) std::cout << "    FAIL " << name << " should throw ereal_divide_by_zero\n";
					++nrOfFailedTestCases;
				}
#else
				Real result = (op == 0) ? remainder(x, zero) : fmod(x, zero);
				if (!result.isnan()) {
					if (reportTestCases) std::cout << "    FAIL " << name << " should be NaN\n";
					++nrOfFailedTestCases;
				}
				result = (op == 0) ? remainder(x, -zero) : fmod(x, -zero);
				if (!result.isnan()) {
					if (reportTestCases) std::cout << "    FAIL " << name << " with -0 should be NaN\n";
					++nrOfFailedTestCases;
				}
#endif
			}
			return nrOfFailedTestCases;
		}

		// C99 F.10.7.1 / F.10.7.2 special values: fmod(+-0, y) = +-0, fmod(x, +-inf) = x,
		// NaN for an infinite x or a NaN operand, and a zero result has the sign of x.
		template<typename Real>
		int VerifyFmodSpecialValues(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const double inf = std::numeric_limits<double>::infinity();
			const double qnan = std::numeric_limits<double>::quiet_NaN();
			const Real zero(0.0);
			auto fail = [&](const std::string& what) {
				if (reportTestCases) std::cout << "    FAIL " << what << '\n';
				++nrOfFailedTestCases;
			};
			for (int op = 0; op < 2; ++op) {
				const std::string name = (op == 0) ? "fmod" : "remainder";
				auto f = [op](const Real& a, const Real& b) { return (op == 0) ? fmod(a, b) : remainder(a, b); };
				if (!is_signed_zero(f(zero, Real(3.0)), false))  fail(name + "(+0, 3) != +0");
				if (!is_signed_zero(f(-zero, Real(3.0)), true))  fail(name + "(-0, 3) != -0");
				if (!exactly(f(Real(5.5), Real(inf)), Real(5.5))) fail(name + "(5.5, +inf) != 5.5");
				if (!exactly(f(Real(-5.5), Real(-inf)), Real(-5.5))) fail(name + "(-5.5, -inf) != -5.5");
				if (!f(Real(inf), Real(2.0)).isnan())  fail(name + "(inf, 2) != NaN");
				if (!f(Real(qnan), Real(2.0)).isnan()) fail(name + "(NaN, 2) != NaN");
				if (!f(Real(2.0), Real(qnan)).isnan()) fail(name + "(2, NaN) != NaN");
				// an exact multiple: a zero with the sign of x
				if (!is_signed_zero(f(Real(-6.0), Real(3.0)), true))  fail(name + "(-6, 3) != -0");
				if (!is_signed_zero(f(Real(6.0), Real(-3.0)), false)) fail(name + "(6, -3) != +0");
			}
			return nrOfFailedTestCases;
		}

		// Verify fmod vs remainder difference
		template<typename Real>
		int VerifyFmodVsRemainder(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: fmod and remainder should differ for certain inputs
			// Example: 5.3 / 2.0 = 2.65
			//   fmod: trunc(2.65) = 2, so 5.3 - 4.0 = 1.3
			//   remainder: round(2.65) = 3, so 5.3 - 6.0 = -0.7
			Real x(5.3), y(2.0);
			Real fmod_result = fmod(x, y);
			Real remainder_result = remainder(x, y);

			if (fmod_result == remainder_result) {
				if (reportTestCases) std::cout << "    FAIL fmod and remainder should differ for 5.3/2.0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}


		// Property fuzzer: the fmod/remainder reduction identities and ranges over
		// random dividend/divisor pairs (divisor bounded away from zero).
		template<typename Real>
		int VerifyFractionalFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> xd(-1.0e3, 1.0e3);
			std::uniform_real_distribution<double> yd(0.5, 1.0e2);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double dx = xd(rng), dy = yd(rng) * (rng() & 1 ? 1.0 : -1.0);
				Real x(dx), y(dy);
				const std::string at = " at x=" + std::to_string(dx) + " y=" + std::to_string(dy);
				// fmod: x == trunc(x/y)*y + fmod(x,y), exactly (|x/y| < 2^11, so trunc(x/y) and
				// the product are exact)
				Real r = fmod(x, y);
				Real n = trunc(x / y);
				if (!exactly(n * y + r, x)) {
					if (reportTestCases) std::cout << "    FAIL fmod reduction" << at << '\n';
					++nrOfFailedTestCases;
				}
				// fmod and remainder are exact, and so are std::fmod and std::remainder
				if (!exactly(r, Real(std::fmod(dx, dy)))) {
					if (reportTestCases) std::cout << "    FAIL fmod != std::fmod" << at << '\n';
					++nrOfFailedTestCases;
				}
				// |fmod(x,y)| < |y| and same sign as x (when nonzero)
				if (!(abs(r) < abs(y))) {
					if (reportTestCases) std::cout << "    FAIL |fmod| >= |y|" << at << '\n';
					++nrOfFailedTestCases;
				}
				if (!r.iszero() && (r.isneg() != (dx < 0.0))) {
					if (reportTestCases) std::cout << "    FAIL fmod sign" << at << '\n';
					++nrOfFailedTestCases;
				}
				// remainder: |remainder(x,y)| <= |y|/2, exactly
				Real rem = remainder(x, y);
				if (abs(rem) > ldexp(abs(y), -1)) {
					if (reportTestCases) std::cout << "    FAIL |remainder| > |y|/2" << at << '\n';
					++nrOfFailedTestCases;
				}
				if (!exactly(rem, Real(std::remainder(dx, dy)))) {
					if (reportTestCases) std::cout << "    FAIL remainder != std::remainder" << at << '\n';
					++nrOfFailedTestCases;
				}
			}
			return nrOfFailedTestCases;
		}

}  // anonymous namespace


namespace {
	using namespace sw::universal;

	// the exact value of an ereal: the sum of its limbs as a dyadic rational
	template<unsigned maxlimbs>
	dyadic exact_value_of(const ereal<maxlimbs>& v) {
		dyadic acc;
		for (double limb : v.limbs()) acc = acc + dyadic::from_double(limb);
		return acc;
	}

	// D - n Y for the integer n nearest D / Y: zero exactly when D is an integer multiple of Y
	dyadic residual_of_multiple(const dyadic& D, const dyadic& Y) {
		using bigint = dyadic::bigint;
		if (D.numerator == bigint(0)) return dyadic();
		bigint num = D.numerator, den = Y.numerator;
		const int s = D.scale - Y.scale;
		for (int i = 0; i < s; ++i) num = num * bigint(2);
		for (int i = 0; i < -s; ++i) den = den * bigint(2);
		const bigint n = num / den;   // truncated; the residual is then below Y in magnitude
		dyadic E = D - dyadic(n, 0) * Y;
		// to the nearest multiple: a remainder near -y/2 leaves D just below a multiple
		if (less_in_magnitude(Y, E + E)) E = (E.numerator < bigint(0)) == (Y.numerator < bigint(0)) ? E - Y : E + Y;
		return E;
	}

	// fmod and remainder of operands with many limbs, checked against a dyadic oracle:
	// r = x - n y with n an integer, 0 <= r < y for fmod and |r| <= y/2 for remainder. The
	// quotient digit's product q * y has twice y's limbs and was cut to the limb budget, so
	// ereal<4> fmod(1e30/7, 1/3) missed by ~1e-101 relative to y (#2054). The exact remainder
	// is returned whenever it fits the limb budget; only a result that fills the budget may
	// be short of it, and then by less than the last limb's unit, 2^(-53 (budget - 1)) y.
	template<unsigned maxlimbs>
	int VerifyMultiLimbFmod(bool reportTestCases) {
		using Real = ereal<maxlimbs>;
		constexpr unsigned budget = Real::limb_budget;
		const int last = 53 * static_cast<int>(budget - 1);
		auto within = [&](const Real& result, const dyadic& X, const dyadic& Y) {
			const dyadic E = residual_of_multiple(X - exact_value_of(result), Y);
			if (E.numerator == dyadic::bigint(0)) return true;                       // exact
			if (result.limbs().size() < budget) return false;                        // it fit: must be exact
			return less_in_magnitude(dyadic(E.numerator, E.scale + last), Y);       // truncated to the budget
		};
		int nrOfFailedTestCases = 0;
		const Real xs[] = { Real(1e30) / Real(7.0), Real(1e200) / Real(3.0), Real(10.0) / Real(7.0), Real(2.0) / Real(3.0) + Real(1e20) };
		const Real ys[] = { Real(1.0) / Real(3.0), Real(2.0) / Real(7.0), Real(1.0) / Real(1e10), Real(3.0) / Real(11.0) };
		for (const Real& x : xs) {
			for (const Real& y : ys) {
				const dyadic X = exact_value_of(x), Y = exact_value_of(y);
				const Real f = fmod(x, y);
				const Real r = remainder(x, y);
				const dyadic F = exact_value_of(f), R = exact_value_of(r);
				const bool fmodOk = within(f, X, Y) && !f.isneg() && less_in_magnitude(F, Y);
				const bool remOk  = within(r, X, Y) && !less_in_magnitude(Y, R + R);
				if (!fmodOk || !remOk) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: ereal<" << maxlimbs << "> fmod/remainder(" << double(x) << ", " << double(y) << "): fmod " << fmodOk << " remainder " << remOk << '\n';
				}
			}
		}
		// near the top of the range: the first quotient digit's product q * y rounded past DBL_MAX
		// and left NaN limbs (#2056 review); on double operands std::fmod is an exact reference
		{
			const double x = std::numeric_limits<double>::max(), y = 2.0 - std::ldexp(1.0, -44);
			const Real f = fmod(Real(x), Real(y));
			if (f.isnan() || double(f) != std::fmod(x, y) || f.limbs().size() > 1) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: ereal<" << maxlimbs << "> fmod(DBL_MAX, 2 - 2^-44) = " << double(f) << " expected " << std::fmod(x, y) << '\n';
			}
		}
		// the case of the sweep fits the budget, so it is exact
		{
			const Real x = Real(1e30) / Real(7.0), y = Real(1.0) / Real(3.0);
			const dyadic E = residual_of_multiple(exact_value_of(x) - exact_value_of(fmod(x, y)), exact_value_of(y));
			if (E.numerator != dyadic::bigint(0)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: ereal<" << maxlimbs << "> fmod(1e30/7, 1/3) is not exact\n";
			}
		}
		return nrOfFailedTestCases;
	}

	// a remainder that needs more limbs than the budget is cut toward zero, so it stays below
	// y: 3 b = 1 - 2^-108 exactly, so fmod(1 - 2^-108 (1 + 2^-52), b) = b - 2^-160, and a plain
	// cut that drops the negative last limb returned b itself (#2056 review)
	int VerifyBudgetedFmodBound(bool reportTestCases) {
		using Real = ereal<1>;   // a two-limb budget
		int nrOfFailedTestCases = 0;
		const Real b = Real(0x1.5555555555555p-2) + Real(0x1.5555555555555p-56);
		const Real a = Real(1.0) - Real(0x1.0000000000001p-108);
		const Real f = fmod(a, b);
		if (!(f < b) || f.isneg()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: ereal<1> fmod is not below the divisor\n";
		}
		return nrOfFailedTestCases;
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib fractional function validation";
	std::string test_tag    = "fractional";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of fractional functions:\n";

	std::cout << "\nBasic tests:\n";
	ereal<> x(5.3), y(2.0);
	std::cout << "fmod(5.3, 2.0) = " << double(fmod(x, y)) << " (expected: 1.3)\n";
	std::cout << "remainder(5.3, 2.0) = " << double(remainder(x, y)) << " (expected: -0.7)\n";

	std::cout << "\nIEEE round-to-nearest-even tie cases:\n";
	ereal<> x1(5.0), y1(2.0);
	std::cout << "remainder(5.0, 2.0) = " << double(remainder(x1, y1))
		<< " (expected: 1.0, rounds 2.5 to 2 even)\n";

	ereal<> x2(7.0), y2(2.0);
	std::cout << "remainder(7.0, 2.0) = " << double(remainder(x2, y2))
		<< " (expected: -1.0, rounds 3.5 to 4 even)\n";

	std::cout << "\nDivision by zero (C99: NaN; throws under EREAL_THROW_ARITHMETIC_EXCEPTION):\n";
	{
		ereal<> x3(5.0), y3(0.0);
#if EREAL_THROW_ARITHMETIC_EXCEPTION
		try {
			ereal<> result = remainder(x3, y3);
			std::cout << "ERROR: Should have thrown exception! got " << double(result) << '\n';
		}
		catch (const ereal_divide_by_zero& e) {
			std::cout << "Caught expected exception: " << e.what() << "\n";
		}
#else
		ereal<> result = remainder(x3, y3);
		std::cout << "remainder(5.0, 0.0) = " << double(result) << " (expected: nan)\n";
#endif
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	test_tag = "fmod";
	nrOfFailedTestCases += ReportTestResult(VerifyFmod<ereal<>>(reportTestCases), "fmod(ereal)", test_tag);

	test_tag = "remainder";
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<ereal<>>(reportTestCases), "remainder(ereal)", test_tag);

	test_tag = "modf";
	nrOfFailedTestCases += ReportTestResult(VerifyModf<ereal<>>(reportTestCases), "modf(ereal)", test_tag);

	test_tag = "fmod vs remainder";
	nrOfFailedTestCases += ReportTestResult(VerifyFmodVsRemainder<ereal<>>(reportTestCases), "fmod vs remainder", test_tag);

	test_tag = "fmod/remainder special values";
	nrOfFailedTestCases += ReportTestResult(VerifyFmodSpecialValues<ereal<>>(reportTestCases), "fmod/remainder C99 special values", test_tag);

	test_tag = "fractional fuzz";
	nrOfFailedTestCases += ReportTestResult(VerifyFractionalFuzz<ereal<>>(reportTestCases, 50), "fmod/remainder property fuzz", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiLimbFmod<4>(reportTestCases), "fmod/remainder ereal<4> multi-limb", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMultiLimbFmod<8>(reportTestCases), "fmod/remainder ereal<8> multi-limb", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyBudgetedFmodBound(reportTestCases), "fmod ereal<1> below the divisor", test_tag);
#endif

#if REGRESSION_LEVEL_2
#if EREAL_THROW_ARITHMETIC_EXCEPTION
	test_tag = "division by zero throws";
#else
	test_tag = "division by zero is NaN";
#endif
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionByZero<ereal<>>(reportTestCases), "fmod/remainder(x, 0)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// Future: Precision validation
#endif

#if REGRESSION_LEVEL_4
	// Future: Stress tests
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
