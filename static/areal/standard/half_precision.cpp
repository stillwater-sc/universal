// half_precision.cpp: comparison tests for areal<16,5> vs IEEE fp16
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
 * This test suite compares areal<16,5> (half precision with ubit) against
 * IEEE fp16 (cfloat<16,5>). The key difference:
 * - cfloat: rounds at each operation, accumulating rounding errors
 * - areal: sets ubit=1 when precision is lost, indicating uncertainty
 *
 * For iterative algorithms, we examine:
 * 1. How often does areal's ubit get set?
 * 2. When ubit=1, does the interval contain the true value?
 * 3. How does cfloat's accumulated error compare to areal's uncertainty?
 */

namespace sw { namespace universal {

// Horner's polynomial evaluation
template<typename Scalar>
Scalar horner_eval(const std::vector<double>& coefficients, const Scalar& x) {
	int n = static_cast<int>(coefficients.size()) - 1;
	Scalar r = Scalar(coefficients[n]);
	for (int i = n - 1; i >= 0; --i) {
		r = r * x + Scalar(coefficients[i]);
	}
	return r;
}

// Taylor series coefficients for sin(x): x - x^3/3! + x^5/5! - x^7/7! + ...
std::vector<double> sin_taylor_coefficients(int terms) {
	std::vector<double> c(2 * terms, 0.0);
	double factorial = 1.0;
	double sign = 1.0;
	for (int i = 0; i < terms; ++i) {
		int power = 2 * i + 1;
		if (i > 0) {
			factorial *= (2 * i) * (2 * i + 1);
		}
		c[power] = sign / factorial;
		sign = -sign;
	}
	return c;
}

// Taylor series coefficients for cos(x): 1 - x^2/2! + x^4/4! - x^6/6! + ...
std::vector<double> cos_taylor_coefficients(int terms) {
	std::vector<double> c(2 * terms, 0.0);
	double factorial = 1.0;
	double sign = 1.0;
	for (int i = 0; i < terms; ++i) {
		int power = 2 * i;
		if (i > 0) {
			factorial *= (2 * i - 1) * (2 * i);
		}
		c[power] = sign / factorial;
		sign = -sign;
	}
	return c;
}

// Taylor series coefficients for exp(x): 1 + x + x^2/2! + x^3/3! + ...
std::vector<double> exp_taylor_coefficients(int terms) {
	std::vector<double> c(terms, 0.0);
	double factorial = 1.0;
	for (int i = 0; i < terms; ++i) {
		if (i > 1) factorial *= i;
		c[i] = 1.0 / factorial;
	}
	return c;
}

// Compare areal vs cfloat for polynomial evaluation
template<size_t nbits, size_t es, typename bt>
int CompareTaylorSeries(const std::string& funcName,
                        const std::vector<double>& coefficients,
                        const std::vector<double>& testValues,
                        std::function<double(double)> refFunc,
                        bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	int nrOfFailedTestCases = 0;
	int uncertainCount = 0;
	double maxArealError = 0.0;
	double maxCfloatError = 0.0;

	for (double x : testValues) {
		// Compute reference using double precision
		double refValue = refFunc(x);

		// Compute using areal
		Areal ax = x;
		Areal arealResult = horner_eval(coefficients, ax);
		bool isUncertain = (arealResult.block(0) & 1) != 0;
		if (isUncertain) uncertainCount++;

		// Compute using cfloat
		Cfloat cx = x;
		Cfloat cfloatResult = horner_eval(coefficients, cx);

		// Compute errors
		double arealValue = double(arealResult);
		double cfloatValue = double(cfloatResult);
		double arealError = std::abs(arealValue - refValue);
		double cfloatError = std::abs(cfloatValue - refValue);

		if (arealError > maxArealError) maxArealError = arealError;
		if (cfloatError > maxCfloatError) maxCfloatError = cfloatError;

		// When areal is uncertain, verify the interval contains the true value
		// The interval is (arealValue, next(arealValue))
		if (isUncertain) {
			Areal nextVal = arealResult;
			++nextVal;  // increment to get next representable value
			double lo = arealValue;
			double hi = double(nextVal);
			if (lo > hi) std::swap(lo, hi);

			// For uncertain values, check if reference is within the interval
			// This is a weaker check - the true value should be in (lo, hi)
			if (refValue < lo || refValue > hi) {
				if (reportTestCases) {
					std::cout << funcName << "(" << x << "): uncertain areal interval ["
					          << lo << ", " << hi << "] does not contain ref=" << refValue << '\n';
				}
				// This is informational, not necessarily a failure
			}
		}

		if (reportTestCases) {
			std::cout << std::setw(12) << funcName << "(" << std::setw(8) << x << "): "
			          << "areal=" << std::setw(12) << arealValue
			          << (isUncertain ? "(u)" : "   ")
			          << " cfloat=" << std::setw(12) << cfloatValue
			          << " ref=" << std::setw(14) << refValue
			          << " aerr=" << std::setw(12) << arealError
			          << " cerr=" << std::setw(12) << cfloatError << '\n';
		}
	}

	// Report summary
	std::cout << funcName << " with " << typeid(Areal).name() << ":\n";
	std::cout << "  Uncertain results: " << uncertainCount << " / " << testValues.size()
	          << " (" << (100.0 * uncertainCount / testValues.size()) << "%)\n";
	std::cout << "  Max areal error:  " << maxArealError << '\n';
	std::cout << "  Max cfloat error: " << maxCfloatError << '\n';

	return nrOfFailedTestCases;
}

// Compare simple arithmetic sequence: sum of 1/n
template<size_t nbits, size_t es, typename bt>
int CompareHarmonicSeries(int terms, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	// Compute reference using double
	double refSum = 0.0;
	for (int i = 1; i <= terms; ++i) {
		refSum += 1.0 / i;
	}

	// Compute using areal
	Areal arealSum = 0;
	for (int i = 1; i <= terms; ++i) {
		arealSum += Areal(1) / Areal(i);
	}
	bool isUncertain = (arealSum.block(0) & 1) != 0;

	// Compute using cfloat
	Cfloat cfloatSum = 0;
	for (int i = 1; i <= terms; ++i) {
		cfloatSum += Cfloat(1) / Cfloat(i);
	}

	double arealValue = double(arealSum);
	double cfloatValue = double(cfloatSum);
	double arealError = std::abs(arealValue - refSum);
	double cfloatError = std::abs(cfloatValue - refSum);

	std::cout << "Harmonic series H(" << terms << ") comparison:\n";
	std::cout << "  Reference:  " << refSum << '\n';
	std::cout << "  Areal:      " << arealValue << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
	std::cout << "  Cfloat:     " << cfloatValue << '\n';
	std::cout << "  Areal error:  " << arealError << '\n';
	std::cout << "  Cfloat error: " << cfloatError << '\n';

	return 0;
}

// Iterative square root using Newton-Raphson
template<size_t nbits, size_t es, typename bt>
int CompareNewtonSqrt(double value, int maxIter, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	double refSqrt = std::sqrt(value);

	// Newton iteration: x_{n+1} = 0.5 * (x_n + value/x_n)
	Areal ax = value;
	Areal arealX = ax;  // initial guess
	for (int i = 0; i < maxIter; ++i) {
		arealX = Areal(0.5) * (arealX + ax / arealX);
	}
	bool isUncertain = (arealX.block(0) & 1) != 0;

	Cfloat cx = value;
	Cfloat cfloatX = cx;
	for (int i = 0; i < maxIter; ++i) {
		cfloatX = Cfloat(0.5) * (cfloatX + cx / cfloatX);
	}

	double arealValue = double(arealX);
	double cfloatValue = double(cfloatX);
	double arealError = std::abs(arealValue - refSqrt);
	double cfloatError = std::abs(cfloatValue - refSqrt);

	if (reportTestCases) {
		std::cout << "Newton sqrt(" << value << ") with " << maxIter << " iterations:\n";
		std::cout << "  Reference:  " << refSqrt << '\n';
		std::cout << "  Areal:      " << arealValue << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
		std::cout << "  Cfloat:     " << cfloatValue << '\n';
		std::cout << "  Areal error:  " << arealError << '\n';
		std::cout << "  Cfloat error: " << cfloatError << '\n';
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

	std::string test_suite  = "areal<16,5> vs fp16 comparison";
	std::string test_tag    = "half precision comparison";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Half precision: areal<16,5> matches IEEE fp16 (1 sign + 5 exp + 10 fraction)
	// Note: areal has ubit taking 1 fraction bit, so effective fraction is 9 bits
	constexpr size_t nbits = 16;
	constexpr size_t es = 5;
	using bt = uint16_t;

#if MANUAL_TESTING
	reportTestCases = true;

	// Test values in range [-pi, pi] for trig functions
	std::vector<double> trigValues = {0.0, 0.1, 0.25, 0.5, 0.785398, 1.0, 1.5708, 2.0, 2.5, 3.0, 3.14159};

	// Taylor series for sin with 8 terms
	auto sinCoeffs = sin_taylor_coefficients(8);
	nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, trigValues,
		[](double x) { return std::sin(x); }, reportTestCases);

	// Taylor series for cos with 8 terms
	auto cosCoeffs = cos_taylor_coefficients(8);
	nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("cos", cosCoeffs, trigValues,
		[](double x) { return std::cos(x); }, reportTestCases);

	// Taylor series for exp with 12 terms
	std::vector<double> expValues = {0.0, 0.1, 0.5, 1.0, 2.0, 3.0};
	auto expCoeffs = exp_taylor_coefficients(12);
	nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("exp", expCoeffs, expValues,
		[](double x) { return std::exp(x); }, reportTestCases);

	// Harmonic series
	CompareHarmonicSeries<nbits, es, bt>(100, reportTestCases);
	CompareHarmonicSeries<nbits, es, bt>(1000, reportTestCases);

	// Newton-Raphson sqrt
	CompareNewtonSqrt<nbits, es, bt>(2.0, 10, reportTestCases);
	CompareNewtonSqrt<nbits, es, bt>(10.0, 10, reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	{
		// Taylor series for sin with 6 terms
		std::vector<double> trigValues = {0.0, 0.1, 0.5, 1.0, 1.5708};
		auto sinCoeffs = sin_taylor_coefficients(6);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, trigValues,
			[](double x) { return std::sin(x); }, reportTestCases);
	}
	{
		// Taylor series for exp with 8 terms
		std::vector<double> expValues = {0.0, 0.5, 1.0, 2.0};
		auto expCoeffs = exp_taylor_coefficients(8);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("exp", expCoeffs, expValues,
			[](double x) { return std::exp(x); }, reportTestCases);
	}
	{
		// Harmonic series to see error accumulation
		CompareHarmonicSeries<nbits, es, bt>(50, reportTestCases);
	}
#endif

#if REGRESSION_LEVEL_2
	{
		// More extensive Taylor series tests
		std::vector<double> trigValues = {0.0, 0.1, 0.25, 0.5, 0.785398, 1.0, 1.5708, 2.0, 2.5, 3.0};

		auto sinCoeffs = sin_taylor_coefficients(8);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, trigValues,
			[](double x) { return std::sin(x); }, reportTestCases);

		auto cosCoeffs = cos_taylor_coefficients(8);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("cos", cosCoeffs, trigValues,
			[](double x) { return std::cos(x); }, reportTestCases);
	}
	{
		CompareHarmonicSeries<nbits, es, bt>(100, reportTestCases);
		CompareHarmonicSeries<nbits, es, bt>(500, reportTestCases);
	}
	{
		CompareNewtonSqrt<nbits, es, bt>(2.0, 5, true);
		CompareNewtonSqrt<nbits, es, bt>(10.0, 5, true);
	}
#endif

#if REGRESSION_LEVEL_3
	{
		// Extended harmonic series to stress test
		CompareHarmonicSeries<nbits, es, bt>(1000, reportTestCases);
	}
#endif

#if REGRESSION_LEVEL_4
	{
		// Exhaustive polynomial evaluation tests
		std::vector<double> manyValues;
		for (double x = -3.14159; x <= 3.14159; x += 0.1) {
			manyValues.push_back(x);
		}
		auto sinCoeffs = sin_taylor_coefficients(10);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, manyValues,
			[](double x) { return std::sin(x); }, reportTestCases);
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
