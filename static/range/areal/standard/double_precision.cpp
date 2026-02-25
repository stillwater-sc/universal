// double_precision.cpp: comparison tests for areal<64,11> vs IEEE fp64
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
#include <functional>

/*
 * This test suite compares areal<64,11> (double precision with ubit) against
 * IEEE fp64 (cfloat<64,11>). The key difference:
 * - cfloat: rounds at each operation, accumulating rounding errors
 * - areal: sets ubit=1 when precision is lost, indicating uncertainty
 *
 * Double precision provides substantial precision, allowing us to:
 * 1. Use higher-order Taylor expansions
 * 2. Run longer iteration sequences
 * 3. Observe subtle differences in error accumulation patterns
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

// Taylor series coefficients for sin(x)
std::vector<double> sin_taylor_coefficients(int terms) {
	std::vector<double> c(2 * terms, 0.0);
	double factorial = 1.0;
	double sign = 1.0;
	for (int i = 0; i < terms; ++i) {
		int power = 2 * i + 1;
		if (i > 0) factorial *= (2 * i) * (2 * i + 1);
		c[power] = sign / factorial;
		sign = -sign;
	}
	return c;
}

// Taylor series coefficients for cos(x)
std::vector<double> cos_taylor_coefficients(int terms) {
	std::vector<double> c(2 * terms, 0.0);
	double factorial = 1.0;
	double sign = 1.0;
	for (int i = 0; i < terms; ++i) {
		int power = 2 * i;
		if (i > 0) factorial *= (2 * i - 1) * (2 * i);
		c[power] = sign / factorial;
		sign = -sign;
	}
	return c;
}

// Taylor series coefficients for exp(x)
std::vector<double> exp_taylor_coefficients(int terms) {
	std::vector<double> c(terms, 0.0);
	double factorial = 1.0;
	for (int i = 0; i < terms; ++i) {
		if (i > 1) factorial *= i;
		c[i] = 1.0 / factorial;
	}
	return c;
}

// Taylor series coefficients for ln(1+x)
std::vector<double> ln1p_taylor_coefficients(int terms) {
	std::vector<double> c(terms + 1, 0.0);
	double sign = 1.0;
	for (int i = 1; i <= terms; ++i) {
		c[i] = sign / i;
		sign = -sign;
	}
	return c;
}

// Taylor series coefficients for atan(x) = x - x^3/3 + x^5/5 - x^7/7 + ...
std::vector<double> atan_taylor_coefficients(int terms) {
	std::vector<double> c(2 * terms, 0.0);
	double sign = 1.0;
	for (int i = 0; i < terms; ++i) {
		int power = 2 * i + 1;
		c[power] = sign / power;
		sign = -sign;
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
		double refValue = refFunc(x);

		Areal ax = x;
		Areal arealResult = horner_eval(coefficients, ax);
		bool isUncertain = (arealResult.block(0) & 1) != 0;
		if (isUncertain) uncertainCount++;

		Cfloat cx = x;
		Cfloat cfloatResult = horner_eval(coefficients, cx);

		double arealValue = double(arealResult);
		double cfloatValue = double(cfloatResult);
		double arealError = std::abs(arealValue - refValue);
		double cfloatError = std::abs(cfloatValue - refValue);

		if (arealError > maxArealError) maxArealError = arealError;
		if (cfloatError > maxCfloatError) maxCfloatError = cfloatError;

		if (reportTestCases) {
			std::cout << std::setw(8) << funcName << "(" << std::setw(12) << x << "): "
			          << "areal=" << std::setw(18) << std::setprecision(14) << arealValue
			          << (isUncertain ? "(u)" : "   ")
			          << " cfloat=" << std::setw(18) << cfloatValue
			          << " ref=" << std::setw(20) << refValue
			          << " aerr=" << std::scientific << std::setw(14) << arealError
			          << " cerr=" << std::setw(14) << cfloatError << std::fixed << '\n';
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

	// Use Kahan summation for reference to minimize double rounding error
	double refSum = 0.0, refC = 0.0;
	for (int i = 1; i <= terms; ++i) {
		double y = (1.0 / i) - refC;
		double t = refSum + y;
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

	double arealValue = double(arealSum);
	double cfloatValue = double(cfloatSum);
	double arealError = std::abs(arealValue - refSum);
	double cfloatError = std::abs(cfloatValue - refSum);

	std::cout << "Harmonic series H(" << terms << ") with areal<" << nbits << "," << es << ">:\n";
	std::cout << "  Reference:    " << std::setprecision(15) << refSum << '\n';
	std::cout << "  Areal:        " << arealValue << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
	std::cout << "  Cfloat:       " << cfloatValue << '\n';
	std::cout << "  Areal error:  " << std::scientific << arealError << std::fixed << '\n';
	std::cout << "  Cfloat error: " << std::scientific << cfloatError << std::fixed << '\n';
	std::cout << std::setprecision(6);

	return 0;
}

// Iterative square root using Newton-Raphson
template<size_t nbits, size_t es, typename bt>
int CompareNewtonSqrt(double value, int maxIter, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	double refSqrt = std::sqrt(value);

	Areal ax = value;
	Areal arealX = ax;
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
		std::cout << "Newton sqrt(" << value << ") with " << maxIter << " iterations (areal<"
		          << nbits << "," << es << ">):\n";
		std::cout << "  Reference:    " << std::setprecision(15) << refSqrt << '\n';
		std::cout << "  Areal:        " << arealValue << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
		std::cout << "  Cfloat:       " << cfloatValue << '\n';
		std::cout << "  Areal error:  " << std::scientific << arealError << std::fixed << '\n';
		std::cout << "  Cfloat error: " << std::scientific << cfloatError << std::fixed << '\n';
		std::cout << std::setprecision(6);
	}

	return 0;
}

// Compute pi using Machin's formula: pi/4 = 4*arctan(1/5) - arctan(1/239)
template<size_t nbits, size_t es, typename bt>
int CompareMachinPi(int atanTerms, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	const double refPi = 3.14159265358979323846;

	auto atanCoeffs = atan_taylor_coefficients(atanTerms);

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

	double arealPi = double(a_pi);
	double cfloatPi = double(c_pi);
	double arealError = std::abs(arealPi - refPi);
	double cfloatError = std::abs(cfloatPi - refPi);

	if (reportTestCases) {
		std::cout << "Machin's formula for pi with " << atanTerms << " atan terms (areal<"
		          << nbits << "," << es << ">):\n";
		std::cout << "  Reference:    " << std::setprecision(17) << refPi << '\n';
		std::cout << "  Areal:        " << arealPi << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
		std::cout << "  Cfloat:       " << cfloatPi << '\n';
		std::cout << "  Areal error:  " << std::scientific << arealError << std::fixed << '\n';
		std::cout << "  Cfloat error: " << std::scientific << cfloatError << std::fixed << '\n';
		std::cout << std::setprecision(6);
	}

	return 0;
}

// Compute e using Taylor series: e = sum(1/n!)
template<size_t nbits, size_t es, typename bt>
int CompareEulerNumber(int terms, bool reportTestCases) {
	using Areal  = areal<nbits, es, bt>;
	using Cfloat = cfloat<nbits, es, bt, true, false, false>;

	const double refE = 2.71828182845904523536;

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

	double arealValue = double(arealE);
	double cfloatValue = double(cfloatE);
	double arealError = std::abs(arealValue - refE);
	double cfloatError = std::abs(cfloatValue - refE);

	if (reportTestCases) {
		std::cout << "Euler's number e with " << terms << " terms (areal<" << nbits << "," << es << ">):\n";
		std::cout << "  Reference:    " << std::setprecision(17) << refE << '\n';
		std::cout << "  Areal:        " << arealValue << (isUncertain ? " (uncertain)" : " (exact)") << '\n';
		std::cout << "  Cfloat:       " << cfloatValue << '\n';
		std::cout << "  Areal error:  " << std::scientific << arealError << std::fixed << '\n';
		std::cout << "  Cfloat error: " << std::scientific << cfloatError << std::fixed << '\n';
		std::cout << std::setprecision(6);
	}

	return 0;
}

// Validate subnormal double to areal conversion
// Enumerates all MSB-set configurations of double subnormals (52 scales)
template<size_t nbits, size_t es, typename bt>
int ValidateSubnormalConversion(bool reportTestCases) {
	using Areal = areal<nbits, es, bt>;

	int nrOfFailedTestCases = 0;

	std::cout << "Validating subnormal double to areal<" << nbits << "," << es << "> conversion:\n";
	std::cout << std::setw(6) << "bit" << " | "
	          << std::setw(25) << "double value" << " | "
	          << std::setw(25) << "areal value" << " | "
	          << std::setw(20) << "binary" << " | "
	          << "status\n";
	std::cout << std::string(100, '-') << '\n';

	// Double subnormals: exponent field = 0, fraction != 0
	// Value = (-1)^s * 2^(-1022) * 0.fraction
	// Walk a 1-bit from MSB (bit 51) down to LSB (bit 0) of the 52-bit fraction
	union {
		double d;
		uint64_t bits;
	} converter;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(17);

	for (int i = 51; i >= 0; --i) {
		// Create a subnormal double with only bit i set in fraction
		converter.bits = 1ull << i;  // exponent = 0, fraction bit i = 1
		double subnormal = converter.d;

		// Convert to areal
		Areal a = subnormal;

		// Convert back to double
		double roundtrip = double(a);

		// Check if conversion preserved the value
		// For areal with fewer fraction bits than double, we expect some loss
		// For areal with same or more fraction bits, we expect exact conversion
		bool exact = (roundtrip == subnormal);
		bool isUncertain = (a.block(0) & 1) != 0;

		// The conversion should be correct if:
		// 1. The value is exact (no ubit set), OR
		// 2. The ubit is set (indicating uncertainty due to precision loss)
		bool correct = exact || isUncertain;

		if (!correct) {
			nrOfFailedTestCases++;
		}

		if (reportTestCases || !correct) {
			std::cout << std::setw(6) << i << " | "
			          << std::scientific << std::setw(25) << subnormal << " | "
			          << std::setw(25) << roundtrip << " | "
			          << to_binary(a) << " | "
			          << (exact ? "EXACT" : (isUncertain ? "UNCERTAIN" : "WRONG"))
			          << (correct ? "" : " <-- FAIL")
			          << '\n';
		}
	}

	// Also test negative subnormals
	std::cout << "\nNegative subnormals:\n";
	for (int i = 51; i >= 0; --i) {
		// Create a negative subnormal double
		converter.bits = (1ull << 63) | (1ull << i);  // sign=1, exponent=0, fraction bit i=1
		double subnormal = converter.d;

		Areal a = subnormal;
		double roundtrip = double(a);

		bool exact = (roundtrip == subnormal);
		bool isUncertain = (a.block(0) & 1) != 0;
		bool correct = exact || isUncertain;

		if (!correct) {
			nrOfFailedTestCases++;
		}

		if (reportTestCases || !correct) {
			std::cout << std::setw(6) << i << " | "
			          << std::scientific << std::setw(25) << subnormal << " | "
			          << std::setw(25) << roundtrip << " | "
			          << to_binary(a) << " | "
			          << (exact ? "EXACT" : (isUncertain ? "UNCERTAIN" : "WRONG"))
			          << (correct ? "" : " <-- FAIL")
			          << '\n';
		}
	}

	std::cout << std::setprecision(oldPrecision) << std::fixed;

	if (nrOfFailedTestCases == 0) {
		std::cout << "\nAll 104 subnormal conversions validated successfully.\n";
	} else {
		std::cout << "\nFailed test cases: " << nrOfFailedTestCases << " / 104\n";
	}

	return nrOfFailedTestCases;
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

	std::string test_suite  = "areal<64,11> vs fp64 comparison";
	std::string test_tag    = "double precision comparison";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Double precision: areal<64,11> matches IEEE fp64 (1 sign + 11 exp + 52 fraction)
	// Note: areal has ubit taking 1 fraction bit, so effective fraction is 51 bits
	constexpr size_t nbits = 64;
	constexpr size_t es = 11;
	using bt = uint32_t;

#if MANUAL_TESTING
	reportTestCases = true;

	// Test values for trig functions
	std::vector<double> trigValues = {0.0, 0.1, 0.25, 0.5, 0.785398, 1.0, 1.5708, 2.0, 2.5, 3.0, 3.14159};

	// Taylor series for sin with 15 terms
	auto sinCoeffs = sin_taylor_coefficients(15);
	nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, trigValues,
		[](double x) { return std::sin(x); }, reportTestCases);

	// Taylor series for cos with 15 terms
	auto cosCoeffs = cos_taylor_coefficients(15);
	nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("cos", cosCoeffs, trigValues,
		[](double x) { return std::cos(x); }, reportTestCases);

	// Taylor series for exp with 20 terms
	std::vector<double> expValues = {0.0, 0.1, 0.5, 1.0, 2.0, 3.0, 5.0, 10.0};
	auto expCoeffs = exp_taylor_coefficients(20);
	nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("exp", expCoeffs, expValues,
		[](double x) { return std::exp(x); }, reportTestCases);

	// Harmonic series
	CompareHarmonicSeries<nbits, es, bt>(10000, reportTestCases);
	CompareHarmonicSeries<nbits, es, bt>(100000, reportTestCases);

	// Newton-Raphson sqrt
	CompareNewtonSqrt<nbits, es, bt>(2.0, 15, reportTestCases);
	CompareNewtonSqrt<nbits, es, bt>(10.0, 15, reportTestCases);

	// Machin's formula for pi
	CompareMachinPi<nbits, es, bt>(20, reportTestCases);
	CompareMachinPi<nbits, es, bt>(50, reportTestCases);

	// Euler's number
	CompareEulerNumber<nbits, es, bt>(20, reportTestCases);
	CompareEulerNumber<nbits, es, bt>(30, reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	{
		// Validate subnormal double to areal conversion
		nrOfFailedTestCases += ValidateSubnormalConversion<nbits, es, bt>(reportTestCases);
	}
	{
		// Taylor series for sin with 10 terms
		std::vector<double> trigValues = {0.0, 0.1, 0.5, 1.0, 1.5708, 3.0};
		auto sinCoeffs = sin_taylor_coefficients(10);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, trigValues,
			[](double x) { return std::sin(x); }, reportTestCases);
	}
	{
		// Taylor series for exp with 15 terms
		std::vector<double> expValues = {0.0, 0.5, 1.0, 2.0, 5.0};
		auto expCoeffs = exp_taylor_coefficients(15);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("exp", expCoeffs, expValues,
			[](double x) { return std::exp(x); }, reportTestCases);
	}
	{
		// Harmonic series
		CompareHarmonicSeries<nbits, es, bt>(1000, reportTestCases);
	}
	{
		// Euler's number
		CompareEulerNumber<nbits, es, bt>(20, true);
	}
#endif

#if REGRESSION_LEVEL_2
	{
		// More extensive Taylor series tests
		std::vector<double> trigValues = {0.0, 0.1, 0.25, 0.5, 0.785398, 1.0, 1.5708, 2.0, 2.5, 3.0};

		auto sinCoeffs = sin_taylor_coefficients(15);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, trigValues,
			[](double x) { return std::sin(x); }, reportTestCases);

		auto cosCoeffs = cos_taylor_coefficients(15);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("cos", cosCoeffs, trigValues,
			[](double x) { return std::cos(x); }, reportTestCases);
	}
	{
		// atan Taylor series for computing pi
		std::vector<double> atanValues = {0.0, 0.1, 0.25, 0.5, 0.75, 1.0};
		auto atanCoeffs = atan_taylor_coefficients(20);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("atan", atanCoeffs, atanValues,
			[](double x) { return std::atan(x); }, reportTestCases);
	}
	{
		CompareHarmonicSeries<nbits, es, bt>(10000, reportTestCases);
	}
	{
		CompareNewtonSqrt<nbits, es, bt>(2.0, 10, true);
		CompareNewtonSqrt<nbits, es, bt>(1000000.0, 15, true);
	}
	{
		CompareMachinPi<nbits, es, bt>(30, true);
	}
#endif

#if REGRESSION_LEVEL_3
	{
		CompareHarmonicSeries<nbits, es, bt>(100000, reportTestCases);
	}
	{
		CompareMachinPi<nbits, es, bt>(50, true);
	}
	{
		CompareEulerNumber<nbits, es, bt>(30, true);
	}
#endif

#if REGRESSION_LEVEL_4
	{
		// Extensive polynomial evaluation tests
		std::vector<double> manyValues;
		for (double x = -3.14159; x <= 3.14159; x += 0.01) {
			manyValues.push_back(x);
		}
		auto sinCoeffs = sin_taylor_coefficients(20);
		nrOfFailedTestCases += CompareTaylorSeries<nbits, es, bt>("sin", sinCoeffs, manyValues,
			[](double x) { return std::sin(x); }, reportTestCases);
	}
	{
		CompareHarmonicSeries<nbits, es, bt>(1000000, reportTestCases);
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
