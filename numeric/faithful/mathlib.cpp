// mathlib.cpp: test suite runner for faithful's math functions: classification, sign, fmin/fmax and sqrt
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Until #1950 faithful's mathlib.hpp included 14 headers from a math/ directory that did not exist,
// so faithful had no math functions. It now has the classification and sign functions, fmin and
// fmax, and a compensated sqrt. The sqrt is measured against references that hold more than the
// pair's precision: long double for faithful<float>, and qd for faithful<double>.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>
#include <cerrno>
#include <cmath>
#include <limits>
#include <random>
#include <string>

namespace {

template<typename Real>
int VerifyClassification(bool reportTestCases) {
	using namespace sw::universal;
	using F = faithful<Real>;
	using RL = std::numeric_limits<Real>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << type_tag(F()) << ' ' << what << '\n'; }
	};
	const F one(Real(1)), zero(Real(0)), nzero(Real(-0.0)), sub(RL::denorm_min()), pinf(RL::infinity()), nan(RL::quiet_NaN());
	check(fpclassify(one) == FP_NORMAL && isnormal(one) && isfinite(one) && !isnan(one) && !isinf(one), "1 is normal");
	check(fpclassify(zero) == FP_ZERO && iszero(zero) && fpclassify(nzero) == FP_ZERO, "zeros");
	check(fpclassify(sub) == FP_SUBNORMAL && isdenorm(sub) && !isnormal(sub) && isfinite(sub), "denorm_min is subnormal");
	check(fpclassify(pinf) == FP_INFINITE && isinf(pinf) && !isfinite(pinf), "inf");
	check(fpclassify(nan) == FP_NAN && isnan(nan) && !isfinite(nan), "NaN");
	// a pair that is not canonical is classified by its sum
	check(fpclassify(F(Real(1), Real(-1))) == FP_ZERO, "(1, -1) is zero");
	check(fpclassify(F(RL::min(), -RL::min() / 2)) == FP_SUBNORMAL, "(min, -min/2) is subnormal");

	// signbit, copysign, fabs
	check(!signbit(one) && signbit(-one) && signbit(nzero) && !signbit(zero), "signbit, including -0");
	check(signbit(F(Real(1), Real(-2))) && !signbit(F(Real(-1), Real(2))), "signbit of the represented value");
	check(!signbit(F(Real(1), Real(-1))), "(1, -1) sums to +0");
	check(copysign(F(Real(2)), -one) == F(Real(-2)) && copysign(F(Real(-2)), one) == F(Real(2)), "copysign");
	check(signbit(copysign(zero, -one)) && !signbit(copysign(nzero, one)), "copysign of zeros");
	check(fabs(F(Real(-2.5))) == F(Real(2.5)) && fabs(F(Real(1), Real(-2))) == F(Real(1)), "fabs");

	// fmin, fmax: exact order, and a NaN operand ignored
	const F a(Real(1), Real(0)), b(Real(1), RL::epsilon() / 4);
	check(fmin(a, b) == a && fmax(a, b) == b && fmin(b, a) == a && fmax(b, a) == b, "fmin/fmax order a tie of the values by the errors");
	check(fmin(nan, a) == a && fmin(a, nan) == a && fmax(nan, a) == a && fmax(a, nan) == a, "fmin/fmax ignore a NaN");
	check(isnan(fmin(nan, nan)) && isnan(fmax(nan, nan)), "fmin/fmax of two NaNs");
	return fails;
}

// faithful over a Universal type: the classification reads faithful's members, so it needs only cfloat's core
int VerifyClassificationOverCfloat(bool reportTestCases) {
	using namespace sw::universal;
	using C = cfloat<16, 5, std::uint16_t, true, false, false>;
	using F = faithful<C>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL faithful<cfloat<16,5>> " << what << '\n'; }
	};
	const F one(C(1.0f)), zero(C(0.0f));
	F inf(C(1.0f)), nan(C(1.0f));
	inf.setinf();
	nan.setnan();
	check(isfinite(one) && !isnan(one) && !isinf(one) && iszero(zero), "finite and zero");
	check(isinf(inf) && isnan(nan), "inf and NaN");
	check(signbit(-one) && !signbit(one), "signbit");
	check(fmax(one, nan) == one && fmin(-one, one) == -one, "fmin/fmax");
	return fails;
}

// the relative error of r against the exact square root of x, in units of eps(Real)^2 / 2
double sqrtErrorUnits(const sw::universal::faithful<float>& x, const sw::universal::faithful<float>& r) {
	const long double xv = static_cast<long double>(x.high()) + static_cast<long double>(x.low());
	const long double ref = std::sqrt(xv);
	const long double got = static_cast<long double>(r.high()) + static_cast<long double>(r.low());
	return static_cast<double>(std::fabs(got - ref) / ref) / std::ldexp(1.0, -47);
}
double sqrtErrorUnits(const sw::universal::faithful<double>& x, const sw::universal::faithful<double>& r) {
	using sw::universal::qd;
	const qd xv = qd(x.high()) + qd(x.low());
	const qd ref = sqrt(xv);
	const qd got = qd(r.high()) + qd(r.low());
	return double(abs(got - ref) / ref) / std::ldexp(1.0, -105);
}

template<typename Real>
int VerifySqrt(bool reportTestCases, double bound, double& worst) {
	using namespace sw::universal;
	using F = faithful<Real>;
	using RL = std::numeric_limits<Real>;
	int fails = 0, reported = 0;
	worst = 0.0;
	auto measure = [&](const F& x) {
		const F r = sqrt(x);
		const double u = sqrtErrorUnits(x, r);
		if (u > worst) worst = u;
		if (u <= bound) return;
		++fails;
		if (reportTestCases && reported++ < 8) std::cerr << "FAIL " << type_tag(F()) << " sqrt" << x << " = " << r << ", " << u << " units\n";
	};
	// exact squares
	measure(F(Real(4)));
	measure(F(Real(0.25)));
	{
		const F s(Real(1), std::ldexp(Real(1), RL::digits / 2 - RL::digits - 2));   // 1 + 2^-k, its square exact as a pair
		measure(s * s);
	}
	// random pairs in [1, 4), where the scaling is the identity, and across the exponent range
	std::mt19937_64 rng(1950);
	std::uniform_real_distribution<Real> mant(Real(1), Real(4));
	std::uniform_real_distribution<Real> frac(Real(-0.5), Real(0.5));
	std::uniform_int_distribution<int>   expo(RL::min_exponent + RL::digits, RL::max_exponent - 2);
	for (int i = 0; i < 100000; ++i) {
		Real head = mant(rng);
		if (i % 2) head = std::ldexp(head, expo(rng));
		const Real ulp = std::nextafter(head, RL::infinity()) - head;
		measure(F(head, frac(rng) * ulp));
	}
	// the ends of the range: the largest finite pair and the smallest normal and subnormal values
	measure(std::numeric_limits<F>::max());
	measure(F(RL::max()));
	measure(std::numeric_limits<F>::min());
	measure(F(RL::min()));
	measure(F(RL::denorm_min()));

	// the special values
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << type_tag(F()) << ' ' << what << '\n'; }
	};
	check(sqrt(F(Real(0))) == F(Real(0)) && !signbit(sqrt(F(Real(0)))), "sqrt(+0) = +0");
	check(iszero(sqrt(F(Real(-0.0)))) && signbit(sqrt(F(Real(-0.0)))), "sqrt(-0) = -0");
	check(isinf(sqrt(F(RL::infinity()))) && !signbit(sqrt(F(RL::infinity()))), "sqrt(+inf) = +inf");
	check(isnan(sqrt(F(RL::quiet_NaN()))), "sqrt(NaN) = NaN");
	errno = 0;
	check(isnan(sqrt(F(Real(-1)))) && errno == EDOM, "sqrt(-1) = NaN with EDOM");
	errno = 0;
	check(isnan(sqrt(F(Real(1), Real(-2)))) && errno == EDOM, "sqrt of the negative pair (1, -2) = NaN with EDOM");
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

	std::string test_suite  = "faithful math library: classification, sign, fmin/fmax, sqrt";
	std::string test_tag    = "faithful";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	faithful<double> x(2.0);
	std::cout << sqrt(x) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyClassification<float>(reportTestCases), "faithful<float>", "classification and sign");
	nrOfFailedTestCases += ReportTestResult(VerifyClassification<double>(reportTestCases), "faithful<double>", "classification and sign");
	nrOfFailedTestCases += ReportTestResult(VerifyClassificationOverCfloat(reportTestCases), "faithful<cfloat>", "classification and sign");
	{
		// the bound in units of eps(Real)^2 / 2, the pair's epsilon(): measured worst 1.32 units for
		// float and 1.37 for double. Evaluating through the FloatingPointType is ~10^7 and ~10^15.
		constexpr double bound = 2.0;
		double worstFloat = 0.0, worstDouble = 0.0;
		nrOfFailedTestCases += ReportTestResult(VerifySqrt<float>(reportTestCases, bound, worstFloat), "faithful<float>", "sqrt");
		nrOfFailedTestCases += ReportTestResult(VerifySqrt<double>(reportTestCases, bound, worstDouble), "faithful<double>", "sqrt");
		std::cout << "sqrt worst relative error, in units of epsilon(): faithful<float> " << worstFloat
		          << ", faithful<double> " << worstDouble << '\n';
	}
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
