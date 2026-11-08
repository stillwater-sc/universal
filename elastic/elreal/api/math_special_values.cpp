// math_special_values.cpp: C99 special values, domains and pow of the elreal math facade (#2003)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// expected: a value, or NaN / +-inf by the same classification
	template<typename Real>
	bool sameValue(const Real& got, double expected, double relTol) {
		if (std::isnan(expected)) return got.isnan();
		if (std::isinf(expected)) return got.isinf() && (got.sign() < 0) == (expected < 0);
		if (!got.isfinite()) return false;
		const double g = double(got);
		if (expected == 0.0) return got.iszero();
		return std::fabs(g - expected) <= relTol * std::fabs(expected);
	}

	// Before #2003 the facade passed non-finite and out-of-domain arguments to the series,
	// which read them as zero: sqrt(-4) = 0, log(0) = 0, exp(inf) = 1, sin(inf) = 0.
	template<typename Real>
	int VerifySpecialValues(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const double inf = std::numeric_limits<double>::infinity();
		const double nan = std::numeric_limits<double>::quiet_NaN();
		const double pi = 3.14159265358979323846;
		auto check = [&](const Real& got, double expected, const char* what) {
			if (!sameValue(got, expected, 1e-15)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << what << " = " << double(got) << ", expected " << expected << '\n';
			}
		};
		check(sqrt(Real(-4.0)), nan, "sqrt(-4)");
		check(sqrt(Real(-inf)), nan, "sqrt(-inf)");
		check(sqrt(Real(inf)), inf, "sqrt(inf)");
		check(sqrt(Real(nan)), nan, "sqrt(nan)");
		check(sqrt(Real(0.0)), 0.0, "sqrt(0)");
		check(sqrt(Real(4.0)), 2.0, "sqrt(4)");
		check(log(Real(0.0)), -inf, "log(0)");
		check(log(Real(-1.0)), nan, "log(-1)");
		check(log(Real(-inf)), nan, "log(-inf)");
		check(log(Real(inf)), inf, "log(inf)");
		check(log(Real(nan)), nan, "log(nan)");
		check(exp(Real(inf)), inf, "exp(inf)");
		check(exp(Real(-inf)), 0.0, "exp(-inf)");
		check(exp(Real(nan)), nan, "exp(nan)");
		check(exp(Real(0.0)), 1.0, "exp(0)");
		check(exp(Real(1e300)), inf, "exp(1e300)");
		check(exp(Real(-1e300)), 0.0, "exp(-1e300)");
		check(sin(Real(inf)), nan, "sin(inf)");
		check(cos(Real(-inf)), nan, "cos(-inf)");
		check(tan(Real(inf)), nan, "tan(inf)");
		check(sin(Real(nan)), nan, "sin(nan)");
		check(asin(Real(2.0)), nan, "asin(2)");
		check(acos(Real(-1.5)), nan, "acos(-1.5)");
		check(asin(Real(inf)), nan, "asin(inf)");
		check(atan(Real(inf)), pi / 2, "atan(inf)");
		check(atan(Real(-inf)), -pi / 2, "atan(-inf)");
		check(sinh(Real(-inf)), -inf, "sinh(-inf)");
		check(cosh(Real(-inf)), inf, "cosh(-inf)");
		check(tanh(Real(inf)), 1.0, "tanh(inf)");
		check(tanh(Real(-inf)), -1.0, "tanh(-inf)");
		check(hypot(Real(inf), Real(1.0)), inf, "hypot(inf, 1)");
		check(hypot(Real(1.0), Real(-inf)), inf, "hypot(1, -inf)");
		check(hypot(Real(inf), Real(nan)), inf, "hypot(inf, nan)");
		check(hypot(Real(nan), Real(1.0)), nan, "hypot(nan, 1)");
		check(hypot(Real(3.0), Real(4.0)), 5.0, "hypot(3, 4)");
		return nrOfFailedTestCases;
	}

	// pow with negative and zero bases used to be 1 (log of a non-positive is the empty
	// stream, and exp(0) = 1), and an exponent within double rounding of an integer was
	// treated as that integer
	template<typename Real>
	int VerifyPow(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const double inf = std::numeric_limits<double>::infinity();
		const double nan = std::numeric_limits<double>::quiet_NaN();
		auto check = [&](const Real& got, double expected, const char* what) {
			if (!sameValue(got, expected, 1e-15)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << what << " = " << double(got) << ", expected " << expected << '\n';
			}
		};
		check(pow(Real(-2.0), Real(100.0)), std::ldexp(1.0, 100), "pow(-2, 100)");
		check(pow(Real(-2.0), Real(101.0)), -std::ldexp(1.0, 101), "pow(-2, 101)");
		check(pow(Real(-2.0), Real(3.0)), -8.0, "pow(-2, 3)");
		check(pow(Real(-2.0), Real(-1.0)), -0.5, "pow(-2, -1)");
		check(pow(Real(-2.0), Real(-2.0)), 0.25, "pow(-2, -2)");
		check(pow(Real(-2.0), Real(0.5)), nan, "pow(-2, 0.5)");
		check(pow(Real(0.0), Real(0.5)), 0.0, "pow(0, 0.5)");
		check(pow(Real(0.0), Real(-1.0)), inf, "pow(0, -1)");
		check(pow(Real(nan), Real(0.0)), 1.0, "pow(nan, 0)");
		check(pow(Real(1.0), Real(nan)), 1.0, "pow(1, nan)");
		check(pow(Real(2.0), Real(nan)), nan, "pow(2, nan)");
		check(pow(Real(0.5), Real(inf)), 0.0, "pow(0.5, inf)");
		check(pow(Real(2.0), Real(inf)), inf, "pow(2, inf)");
		check(pow(Real(2.0), Real(-inf)), 0.0, "pow(2, -inf)");
		check(pow(Real(-1.0), Real(inf)), 1.0, "pow(-1, inf)");
		check(pow(Real(inf), Real(-1.0)), 0.0, "pow(inf, -1)");
		check(pow(Real(-inf), Real(3.0)), -inf, "pow(-inf, 3)");
		check(pow(Real(-inf), Real(2.0)), inf, "pow(-inf, 2)");
		check(pow(Real(2.0), Real(0.5)), std::sqrt(2.0), "pow(2, 0.5)");
		check(pow(Real(2.0), Real(10.0)), 1024.0, "pow(2, 10)");
		check(pow(Real(3.0), Real(-3.0)), 1.0 / 27.0, "pow(3, -3)");

		// 3 + 2^-200 is not an integer: 2^(3 + 2^-200) - 8 = 8 ln2 2^-200 (1 + O(2^-200))
		const Real y = Real(3.0) + Real(std::ldexp(1.0, -200));
		const Real excess = pow(Real(2.0), y) - Real(8.0);
		const double want = 8.0 * std::log(2.0) * std::ldexp(1.0, -200);
		if (excess.iszero() || std::fabs(double(excess) - want) > 1e-3 * want) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: pow(2, 3 + 2^-200) - 8 = " << double(excess) << ", expected " << want << '\n';
		}
		// an integer exponent above 2^53, where a double cannot tell its parity: 2^53 + 1 is odd
		const Real oddBig = Real((1ll << 53) + 1);
		if (pow(Real(-1.0), oddBig).sign() != -1) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: pow(-1, 2^53 + 1) is not -1\n";
		}
		return nrOfFailedTestCases;
	}

	// exp reduces by multiples of ln2 instead of halving and squaring: exp(1e6) used to run
	// for minutes, and a value whose leading block is zero is reduced on its first nonzero block
	template<typename Real>
	int VerifyRange(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto fail = [&](const std::string& what, double got, double expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << what << " = " << got << ", expected " << expected << '\n';
		};
		const auto t0 = std::chrono::steady_clock::now();
		const Real big = exp(Real(1e6));
		const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
		const double scaleWant = std::floor(1e6 / std::log(2.0));
		if (double(big.scale()) != scaleWant) fail("exp(1e6).scale()", double(big.scale()), scaleWant);
		if (std::fabs(double(log(big)) - 1e6) > 1e-9) fail("log(exp(1e6))", double(log(big)), 1e6);
		if (seconds > 10.0) fail("seconds for exp(1e6)", seconds, 10.0);
		if (std::fabs(double(exp(Real(40.0))) / 2.3538526683701998541e17 - 1.0) > 1e-15) fail("exp(40)", double(exp(Real(40.0))), 2.3538526683701998541e17);
		if (std::fabs(double(exp(Real(-40.0))) / 4.2483542552915889953e-18 - 1.0) > 1e-15) fail("exp(-40)", double(exp(Real(-40.0))), 4.2483542552915889953e-18);

		// arguments past the exp range guard, reached through sinh, cosh, tanh and pow as well
		// as exp: they used to hand an unreduced argument to the stream exp, and stall
		const double inf = std::numeric_limits<double>::infinity();
		auto limit = [&](const Real& got, double expected, const char* what) {
			if (!sameValue(got, expected, 1e-15)) fail(what, double(got), expected);
		};
		const auto t1 = std::chrono::steady_clock::now();
		limit(sinh(Real(1e300)), inf, "sinh(1e300)");
		limit(sinh(Real(-1e300)), -inf, "sinh(-1e300)");
		limit(cosh(Real(-1e300)), inf, "cosh(-1e300)");
		limit(tanh(Real(1e300)), 1.0, "tanh(1e300)");
		limit(tanh(Real(-1e300)), -1.0, "tanh(-1e300)");
		limit(pow(Real(2.0), Real(1e300)), inf, "pow(2, 1e300)");
		limit(pow(Real(2.0), Real(-1e300)), 0.0, "pow(2, -1e300)");
		limit(pow(Real(0.5), Real(1e300)), 0.0, "pow(0.5, 1e300)");
		limit(pow(Real(-2.0), Real(1e300)), inf, "pow(-2, 1e300)");   // 1e300 is an even integer
		limit(pow(Real(1.0) + Real(std::ldexp(1.0, -100)), Real(1e300)), inf, "pow(1 + 2^-100, 1e300)");   // base near 1
		const double guardSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - t1).count();
		if (guardSeconds > 10.0) fail("seconds for the range-guarded calls", guardSeconds, 10.0);

		const Real x = Real(1.0) - (Real(1.0) - Real(std::ldexp(1.0, -100)));   // 2^-100, leading zero block
		if (std::fabs(double(log(x)) + 100.0 * std::log(2.0)) > 1e-13) fail("log(2^-100)", double(log(x)), -100.0 * std::log(2.0));
		if (double(sqrt(x)) != std::ldexp(1.0, -50)) fail("sqrt(2^-100)", double(sqrt(x)), std::ldexp(1.0, -50));
		const Real nx = (Real(1.0) - std::ldexp(1.0, -100)) - Real(1.0);          // -2^-100, leading zero block
		if (!sqrt(nx).isnan()) fail("sqrt(-2^-100)", double(sqrt(nx)), std::numeric_limits<double>::quiet_NaN());
		return nrOfFailedTestCases;
	}

	// a float host keeps exponents beyond float's range, so the integer test sees a finite double
	// above FLT_MAX: it is an even integer there, and casting it to float was undefined
	int VerifyNarrowHostPow(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		using F = elreal<float>;
		const F y = F(1e30f) * F(1e30f);   // 1e60, beyond FLT_MAX
		const F r = pow(F(-2.0f), y);
		if (!r.isinf() || r.sign() != 1) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: elreal<float> pow(-2, 1e60) = " << double(r) << ", expected +inf\n";
		}
		const F z = pow(F(-2.0f), F(3.0f));
		if (double(z) != -8.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: elreal<float> pow(-2, 3) = " << double(z) << ", expected -8\n";
		}
		return nrOfFailedTestCases;
	}

	// integer-ness of values a double cannot see: elreal holds 2^2000 + 0.5 exactly on any host,
	// and its double approximation is inf. Treating every such value as an even integer let
	// pow(-2, 2^2000 + 0.5) through the negative-base domain check.
	template<typename FpType>
	int VerifyIntegerTestBeyondDouble(bool reportTestCases, const std::string& host) {
		int nrOfFailedTestCases = 0;
		using B = block<FpType>;
		using S = ZBCL<FpType>;
		using Real = elreal<FpType>;
		auto expect = [&](bool ok, const std::string& what) {
			if (!ok) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << host << ' ' << what << '\n';
			}
		};
		const S big = S::singleton(B{ static_cast<FpType>(1.0), typename B::exp_t(2000) });   // 2^2000
		bool odd = true;
		expect(detail::is_integer(big, 4, odd) && !odd, "2^2000 is an even integer");
		const S bigHalf = add(big, from_native<FpType>(0.5));
		expect(!detail::is_integer(bigHalf, 4, odd), "2^2000 + 0.5 is not an integer");
		const S bigOne = add(big, from_native<FpType>(1.0));
		expect(detail::is_integer(bigOne, 4, odd) && odd, "2^2000 + 1 is an odd integer");
		expect(pow(Real(-2.0), Real(bigHalf)).isnan(), "pow(-2, 2^2000 + 0.5) is NaN");
		expect(pow(Real(-1.0), Real(bigOne)).sign() == -1, "pow(-1, 2^2000 + 1) is -1");
		return nrOfFailedTestCases;
	}

#if LONG_DOUBLE_SUPPORT
	// a long double block reaches 2^64, beyond what a double rounds exactly: 2^60 + 3, held as the
	// block 2^60 + 3.5 and a block -0.5, rounds to 2^60 + 4 in a double
	int VerifyLongDoubleBlockRounding(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		using B = block<long double>;
		using S = ZBCL<long double>;
		const long double lead = std::ldexp(1.0L, 60) + 3.5L;
		const S y = add(S::singleton(B{ lead, 0 }), from_native<long double>(-0.5));   // 2^60 + 3
		bool odd = false;
		if (!detail::is_integer(y, 4, odd) || !odd) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: long double 2^60 + 3 is not reported as an odd integer\n";
		}
		const S z = add(S::singleton(B{ lead, 0 }), from_native<long double>(-0.25));      // 2^60 + 3.25
		if (detail::is_integer(z, 4, odd)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: long double 2^60 + 3.25 is reported as an integer\n";
		}
		return nrOfFailedTestCases;
	}
#endif

}}  // namespace sw::universal

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

	std::string test_suite  = "elreal math special values validation";
	std::string test_tag    = "math special values";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// 4 blocks (~64 decimal digits on a double host): the special cases do not depend on
	// the precision, and the default of 32 blocks costs seconds per transcendental call
	elreal_precision_guard precision(4);

#if MANUAL_TESTING

	std::cout << "log(0) = " << double(log(elreal<double>(0.0))) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<elreal<double>>(reportTestCases), "elreal<double>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifyPow<elreal<double>>(reportTestCases), "elreal<double>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyRange<elreal<double>>(reportTestCases), "elreal<double>", "exp range and leading zero block");
	nrOfFailedTestCases += ReportTestResult(VerifyNarrowHostPow(reportTestCases), "elreal<float>", "pow beyond the host range");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerTestBeyondDouble<double>(reportTestCases, "elreal<double>"), "elreal<double>", "integer test beyond double");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerTestBeyondDouble<float>(reportTestCases, "elreal<float>"), "elreal<float>", "integer test beyond double");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerTestBeyondDouble<long double>(reportTestCases, "elreal<long double>"), "elreal<long double>", "integer test beyond double");
	nrOfFailedTestCases += ReportTestResult(VerifyLongDoubleBlockRounding(reportTestCases), "elreal<long double>", "integer test past 2^53");
#endif
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
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
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
