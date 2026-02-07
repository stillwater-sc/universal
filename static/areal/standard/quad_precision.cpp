// quad_precision.cpp: comparison tests for areal<128,15> vs IEEE fp128
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <vector>
#include <cmath>
#include <iomanip>

/*
 * This test suite compares areal<128,15> (quad precision with ubit) against
 * IEEE fp128 (cfloat<128,15>). The key difference:
 * - cfloat: rounds at each operation, accumulating rounding errors
 * - areal: sets ubit=1 when precision is lost, indicating uncertainty
 *
 * Quad precision provides extremely high precision (112 fraction bits for IEEE,
 * 111 effective bits for areal due to ubit). This allows:
 * 1. Very high-order Taylor expansions
 * 2. Extended iteration sequences with minimal error accumulation
 * 3. Computation of mathematical constants to high precision
 *
 * Note: Reference values are limited by long double precision on most platforms.
 * For true quad-precision verification, external high-precision libraries would be needed.
 */

namespace sw { namespace universal {

// Horner's polynomial evaluation
template<typename Scalar>
Scalar horner_eval(const std::vector<long double>& coefficients, const Scalar& x) {
	int n = static_cast<int>(coefficients.size()) - 1;
	Scalar r = Scalar(static_cast<double>(coefficients[n]));
	for (int i = n - 1; i >= 0; --i) {
		r = r * x + Scalar(static_cast<double>(coefficients[i]));
	}
	return r;
}

// Taylor series coefficients for sin(x) using long double for higher precision
std::vector<long double> sin_taylor_coefficients_ld(int terms) {
	std::vector<long double> c(2 * terms, 0.0L);
	long double factorial = 1.0L;
	long double sign = 1.0L;
	for (int i = 0; i < terms; ++i) {
		int power = 2 * i + 1;
		if (i > 0) factorial *= static_cast<long double>((2 * i) * (2 * i + 1));
		c[power] = sign / factorial;
		sign = -sign;
	}
	return c;
}

// Taylor series coefficients for cos(x)
std::vector<long double> cos_taylor_coefficients_ld(int terms) {
	std::vector<long double> c(2 * terms, 0.0L);
	long double factorial = 1.0L;
	long double sign = 1.0L;
	for (int i = 0; i < terms; ++i) {
		int power = 2 * i;
		if (i > 0) factorial *= static_cast<long double>((2 * i - 1) * (2 * i));
		c[power] = sign / factorial;
		sign = -sign;
	}
	return c;
}

// Taylor series coefficients for exp(x)
std::vector<long double> exp_taylor_coefficients_ld(int terms) {
	std::vector<long double> c(terms, 0.0L);
	long double factorial = 1.0L;
	for (int i = 0; i < terms; ++i) {
		if (i > 1) factorial *= static_cast<long double>(i);
		c[i] = 1.0L / factorial;
	}
	return c;
}

// Taylor series coefficients for atan(x)
std::vector<long double> atan_taylor_coefficients_ld(int terms) {
	std::vector<long double> c(2 * terms, 0.0L);
	long double sign = 1.0L;
	for (int i = 0; i < terms; ++i) {
		int power = 2 * i + 1;
		c[power] = sign / static_cast<long double>(power);
		sign = -sign;
	}
	return c;
}

// Compare areal vs cfloat for polynomial evaluation
template<size_t nbits, size_t es, typename bt>
int CompareTaylorSeries(const std::string& funcName,
                        const std::vector<long double>& coefficients,
                        const std::vector<double>& testValues,
                        std::function<long double(long double)> refFunc,
                        bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	int nrOfFailedTestCases = 0;
	int uncertainCount = 0;
	long double maxArealError = 0.0L;
	long double maxCfloatError = 0.0L;

	for (double xd : testValues) {
		long double x = static_cast<long double>(xd);
		long double refValue = refFunc(x);

		Areal ax = xd;
		Areal arealResult = horner_eval(coefficients, ax);
		bool isUncertain = (arealResult.block(0) & 1) != 0;
		if (isUncertain) uncertainCount++;

		Cfloat cx = xd;
		Cfloat cfloatResult = horner_eval(coefficients, cx);

		// Convert to long double for comparison
		long double arealValue = static_cast<long double>(double(arealResult));
		long double cfloatValue = static_cast<long double>(double(cfloatResult));
		long double arealError = std::abs(arealValue - refValue);
		long double cfloatError = std::abs(cfloatValue - refValue);

		if (arealError > maxArealError) maxArealError = arealError;
		if (cfloatError > maxCfloatError) maxCfloatError = cfloatError;

		if (reportTestCases) {
			std::cout << std::setw(8) << funcName << "(" << std::setw(12) << xd << "): "
			          << "areal=" << std::setw(22) << std::setprecision(18) << arealValue
			          << (isUncertain ? "(u)" : "   ")
			          << " cfloat=" << std::setw(22) << cfloatValue
			          << '\n';
		}
	}

	std::cout << std::setprecision(6);
	std::cout << funcName << " with areal<" << nbits << "," << es << ">:\n";
	std::cout << "  Uncertain results: " << uncertainCount << " / " << testValues.size()
	          << " (" << (100.0 * uncertainCount / testValues.size()) << "%)\n";
	std::cout << "  Max areal error:  " << std::scientific << maxArealError << std::fixed << '\n';
	std::cout << "  Max cfloat error: " << std::scientific << maxCfloatError << std::fixed << '\n';

	return nrOfFailedTestCases;
}

// Compare harmonic series
template<size_t nbits, size_t es, typename bt>
int CompareHarmonicSeries(int terms, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	// Use Kahan summation with long double for reference
	long double refSum = 0.0L, refC = 0.0L;
	for (int i = 1; i <= terms; ++i) {
		long double y = (1.0L / static_cast<long double>(i)) - refC;
		long double t = refSum + y;
		refC = (t - refSum) - y;
		refSum = t;
	}

	Areal arealSum = 0;
	for (int i = 1; i <= terms; ++i) {
		arealSum += Areal(1) / Areal(i);
	}
	bool isUncertain = (arealSum.block(0) & 1) != 0;

	Cfloat cfloatSum = 0;
	for (int i = 1; i <= terms; ++i) {
		cfloatSum += Cfloat(1) / Cfloat(i);
	}

	long double arealValue = static_cast<long double>(double(arealSum));
	long double cfloatValue = static_cast<long double>(double(cfloatSum));
	long double arealError = std::abs(arealValue - refSum);
	long double cfloatError = std::abs(cfloatValue - refSum);

	std::cout << "Harmonic series H(" << terms << ") with areal<" << nbits << "," << es << ">:\n";
	std::cout << "  Reference:    " << std::setprecision(18) << refSum << '\n';
	std::cout << "  Areal:        " << arealValue << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
	std::cout << "  Cfloat:       " << cfloatValue << '\n';
	std::cout << "  Areal error:  " << std::scientific << arealError << std::fixed << '\n';
	std::cout << "  Cfloat error: " << std::scientific << cfloatError << std::fixed << '\n';
	std::cout << std::setprecision(6);

	return 0;
}

// Compute pi using Machin's formula with high precision
template<size_t nbits, size_t es, typename bt>
int CompareMachinPi(int atanTerms, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	// High precision reference (limited by long double on most platforms)
	const long double refPi = 3.14159265358979323846264338327950288L;

	auto atanCoeffs = atan_taylor_coefficients_ld(atanTerms);

	// Compute pi/4 = 4*atan(1/5) - atan(1/239) using areal
	Areal a_x1 = Areal(1) / Areal(5);
	Areal a_x2 = Areal(1) / Areal(239);
	Areal a_atan1 = horner_eval(atanCoeffs, a_x1);
	Areal a_atan2 = horner_eval(atanCoeffs, a_x2);
	Areal a_pi = Areal(4) * (Areal(4) * a_atan1 - a_atan2);
	bool isUncertain = (a_pi.block(0) & 1) != 0;

	// Compute using cfloat
	Cfloat c_x1 = Cfloat(1) / Cfloat(5);
	Cfloat c_x2 = Cfloat(1) / Cfloat(239);
	Cfloat c_atan1 = horner_eval(atanCoeffs, c_x1);
	Cfloat c_atan2 = horner_eval(atanCoeffs, c_x2);
	Cfloat c_pi = Cfloat(4) * (Cfloat(4) * c_atan1 - c_atan2);

	long double arealPi = static_cast<long double>(double(a_pi));
	long double cfloatPi = static_cast<long double>(double(c_pi));
	long double arealError = std::abs(arealPi - refPi);
	long double cfloatError = std::abs(cfloatPi - refPi);

	if (reportTestCases) {
		std::cout << "Machin's formula for pi with " << atanTerms << " atan terms (areal<"
		          << nbits << "," << es << ">):\n";
		std::cout << "  Reference:    " << std::setprecision(30) << refPi << '\n';
		std::cout << "  Areal:        " << arealPi << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
		std::cout << "  Cfloat:       " << cfloatPi << '\n';
		std::cout << "  Areal error:  " << std::scientific << arealError << std::fixed << '\n';
		std::cout << "  Cfloat error: " << std::scientific << cfloatError << std::fixed << '\n';
		std::cout << std::setprecision(6);
	}

	return 0;
}

// Compute e using Taylor series
template<size_t nbits, size_t es, typename bt>
int CompareEulerNumber(int terms, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	const long double refE = 2.71828182845904523536028747135266249L;

	// Compute e using areal
	Areal arealE = 0;
	Areal factorial = 1;
	for (int i = 0; i < terms; ++i) {
		arealE += Areal(1) / factorial;
		factorial *= Areal(i + 1);
	}
	bool isUncertain = (arealE.block(0) & 1) != 0;

	// Compute e using cfloat
	Cfloat cfloatE = 0;
	Cfloat cfactorial = 1;
	for (int i = 0; i < terms; ++i) {
		cfloatE += Cfloat(1) / cfactorial;
		cfactorial *= Cfloat(i + 1);
	}

	long double arealValue = static_cast<long double>(double(arealE));
	long double cfloatValue = static_cast<long double>(double(cfloatE));
	long double arealError = std::abs(arealValue - refE);
	long double cfloatError = std::abs(cfloatValue - refE);

	if (reportTestCases) {
		std::cout << "Euler's number e with " << terms << " terms (areal<" << nbits << "," << es << ">):\n";
		std::cout << "  Reference:    " << std::setprecision(30) << refE << '\n';
		std::cout << "  Areal:        " << arealValue << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
		std::cout << "  Cfloat:       " << cfloatValue << '\n';
		std::cout << "  Areal error:  " << std::scientific << arealError << std::fixed << '\n';
		std::cout << "  Cfloat error: " << std::scientific << cfloatError << std::fixed << '\n';
		std::cout << std::setprecision(6);
	}

	return 0;
}

// Compute sqrt(2) using Newton-Raphson
template<size_t nbits, size_t es, typename bt>
int CompareNewtonSqrt2(int maxIter, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	const long double refSqrt2 = 1.41421356237309504880168872420969807L;

	// Newton iteration for sqrt(2)
	Areal ax = 2;
	Areal arealX = ax;
	for (int i = 0; i < maxIter; ++i) {
		arealX = Areal(0.5) * (arealX + ax / arealX);
	}
	bool isUncertain = (arealX.block(0) & 1) != 0;

	Cfloat cx = 2;
	Cfloat cfloatX = cx;
	for (int i = 0; i < maxIter; ++i) {
		cfloatX = Cfloat(0.5) * (cfloatX + cx / cfloatX);
	}

	long double arealValue = static_cast<long double>(double(arealX));
	long double cfloatValue = static_cast<long double>(double(cfloatX));
	long double arealError = std::abs(arealValue - refSqrt2);
	long double cfloatError = std::abs(cfloatValue - refSqrt2);

	if (reportTestCases) {
		std::cout << "Newton sqrt(2) with " << maxIter << " iterations (areal<"
		          << nbits << "," << es << ">):\n";
		std::cout << "  Reference:    " << std::setprecision(30) << refSqrt2 << '\n';
		std::cout << "  Areal:        " << arealValue << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
		std::cout << "  Cfloat:       " << cfloatValue << '\n';
		std::cout << "  Areal error:  " << std::scientific << arealError << std::fixed << '\n';
		std::cout << "  Cfloat error: " << std::scientific << cfloatError << std::fixed << '\n';
		std::cout << std::setprecision(6);
	}

	return 0;
}

// Compute golden ratio using continued fraction iteration
template<size_t nbits, size_t es, typename bt>
int CompareGoldenRatio(int maxIter, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	const long double refPhi = 1.61803398874989484820458683436563811L;

	// Iterate: phi = 1 + 1/phi, starting with phi = 1
	Areal arealPhi = 1;
	for (int i = 0; i < maxIter; ++i) {
		arealPhi = Areal(1) + Areal(1) / arealPhi;
	}
	bool isUncertain = (arealPhi.block(0) & 1) != 0;

	Cfloat cfloatPhi = 1;
	for (int i = 0; i < maxIter; ++i) {
		cfloatPhi = Cfloat(1) + Cfloat(1) / cfloatPhi;
	}

	long double arealValue = static_cast<long double>(double(arealPhi));
	long double cfloatValue = static_cast<long double>(double(cfloatPhi));
	long double arealError = std::abs(arealValue - refPhi);
	long double cfloatError = std::abs(cfloatValue - refPhi);

	if (reportTestCases) {
		std::cout << "Golden ratio phi with " << maxIter << " iterations (areal<"
		          << nbits << "," << es << ">):\n";
		std::cout << "  Reference:    " << std::setprecision(30) << refPhi << '\n';
		std::cout << "  Areal:        " << arealValue << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
		std::cout << "  Cfloat:       " << cfloatValue << '\n';
		std::cout << "  Areal error:  " << std::scientific << arealError << std::fixed << '\n';
		std::cout << "  Cfloat error: " << std::scientific << cfloatError << std::fixed << '\n';
		std::cout << std::setprecision(6);
	}

	return 0;
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal<128,15> vs fp128 comparison";
	std::string test_tag    = "quad precision comparison";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Quad precision: areal<128,15> matches IEEE fp128 (1 sign + 15 exp + 112 fraction)
	// Note: areal has ubit taking 1 fraction bit, so effective fraction is 111 bits
	constexpr size_t nbits = 128;
	constexpr size_t es = 15;
	using bt = uint32_t;

#if MANUAL_TESTING
	reportTestCases = true;

	// Test values for trig functions
	std::vector<double> trigValues = {0.0, 0.1, 0.25, 0.5, 0.785398, 1.0, 1.5708, 2.0, 2.5, 3.0, 3.14159};

	// Taylor series for sin with 25 terms
	auto sinCoeffs = sin_taylor_coefficients_ld(25);
	nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, trigValues,
		[](long double x) { return std::sin(x); }, reportTestCases);

	// Taylor series for cos with 25 terms
	auto cosCoeffs = cos_taylor_coefficients_ld(25);
	nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("cos", cosCoeffs, trigValues,
		[](long double x) { return std::cos(x); }, reportTestCases);

	// Taylor series for exp with 30 terms
	std::vector<double> expValues = {0.0, 0.1, 0.5, 1.0, 2.0, 5.0, 10.0};
	auto expCoeffs = exp_taylor_coefficients_ld(30);
	nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("exp", expCoeffs, expValues,
		[](long double x) { return std::exp(x); }, reportTestCases);

	// Mathematical constants
	CompareMachinPi<nbits, es, bt>(50, reportTestCases);
	CompareMachinPi<nbits, es, bt>(100, reportTestCases);
	CompareEulerNumber<nbits, es, bt>(40, reportTestCases);
	CompareNewtonSqrt2<nbits, es, bt>(20, reportTestCases);
	CompareGoldenRatio<nbits, es, bt>(50, reportTestCases);

	// Harmonic series
	CompareHarmonicSeries<nbits, es, bt>(10000, reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	{
		// Taylor series for sin with 15 terms
		std::vector<double> trigValues = {0.0, 0.1, 0.5, 1.0, 1.5708, 3.0};
		auto sinCoeffs = sin_taylor_coefficients_ld(15);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, trigValues,
			[](long double x) { return std::sin(x); }, reportTestCases);
	}
	{
		// Taylor series for exp with 20 terms
		std::vector<double> expValues = {0.0, 0.5, 1.0, 2.0, 5.0};
		auto expCoeffs = exp_taylor_coefficients_ld(20);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("exp", expCoeffs, expValues,
			[](long double x) { return std::exp(x); }, reportTestCases);
	}
	{
		// Mathematical constants
		CompareEulerNumber<nbits, es, bt>(25, true);
		CompareNewtonSqrt2<nbits, es, bt>(15, true);
	}
#endif

#if REGRESSION_LEVEL_2
	{
		// More extensive Taylor series tests
		std::vector<double> trigValues = {0.0, 0.1, 0.25, 0.5, 0.785398, 1.0, 1.5708, 2.0, 2.5, 3.0};

		auto sinCoeffs = sin_taylor_coefficients_ld(20);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, trigValues,
			[](long double x) { return std::sin(x); }, reportTestCases);

		auto cosCoeffs = cos_taylor_coefficients_ld(20);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("cos", cosCoeffs, trigValues,
			[](long double x) { return std::cos(x); }, reportTestCases);
	}
	{
		CompareMachinPi<nbits, es, bt>(40, true);
	}
	{
		CompareHarmonicSeries<nbits, es, bt>(1000, reportTestCases);
	}
	{
		CompareGoldenRatio<nbits, es, bt>(30, true);
	}
#endif

#if REGRESSION_LEVEL_3
	{
		CompareMachinPi<nbits, es, bt>(75, true);
	}
	{
		CompareEulerNumber<nbits, es, bt>(40, true);
	}
	{
		CompareHarmonicSeries<nbits, es, bt>(10000, reportTestCases);
	}
#endif

#if REGRESSION_LEVEL_4
	{
		// Extensive polynomial evaluation tests
		std::vector<double> manyValues;
		for (double x = -3.14159; x <= 3.14159; x += 0.01) {
			manyValues.push_back(x);
		}
		auto sinCoeffs = sin_taylor_coefficients_ld(30);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, manyValues,
			[](long double x) { return std::sin(x); }, reportTestCases);
	}
	{
		CompareMachinPi<nbits, es, bt>(100, true);
	}
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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
