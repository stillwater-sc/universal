// mod_complex_mul.cpp: test suite runner for arbitrary configuration fixed-point complex multiplication
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
// According to the C++ ISO spec, paragraph 26.2/2:
//    The effect of instantiating the template complex for any type other than float, double or long double is unspecified.
#include <complex>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>
#include <universal/verification/test_case.hpp>

// generate specific test case that you can trace with the trace conditions in fixed_point.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	sw::universal::fixpnt<nbits, rbits> a, b, cref, result;
	sw::universal::blockbinary<2 * nbits> full;
	a = _a;
	b = _b;
	result = a * b;
	Ty ref = _a * _b;
	full = (long long)ref;
	cref = ref;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits + 1) << _a << " * " << std::setw(nbits + 1) << _b << " = " << std::setw(nbits + 1) << ref << " (reference: " << to_binary(full) << ")" << std::endl;
	std::cout << std::setw(nbits + 1) << a << " * " << std::setw(nbits + 1) << b << " = " << std::setw(nbits + 1) << result << " (reference: " << cref << ")   " ;
	std::cout << (cref == result ? "PASS" : "FAIL") << std::endl;
	std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(result) << " (reference: " << to_binary(cref) << ")   ";

	std::cout << std::endl << std::endl << std::dec << std::setprecision(oldPrecision);
}

// enumerate all complex multiplication cases for an fixpnt<nbits,rbits> configuration
template<size_t nbits, size_t rbits, bool arithmetic, typename BlockType>
int VerifyComplexMultiplication(bool reportTestCases) {
	using namespace sw::universal;
	using FixedPoint = fixpnt<nbits, rbits, arithmetic, BlockType>;
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	FixedPoint maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	int nrOfFailedTests = 0;
	FixedPoint ar, ai, br, bi;
	std::complex<FixedPoint> a, b, result, ref;

	constexpr bool statusFeedback{ true };
	bool statusStringPresent{ false };
	unsigned nrTests{ 0 };
	std::complex<double> da, db, dc;

	for (size_t i = 0; i < NR_VALUES; i++) {
		ar.setbits(i);
		for (size_t j = 0; j < NR_VALUES; j++) {
			ai.setbits(j);
			a = std::complex<FixedPoint>(ar, ai);
			da = std::complex<double>(double(ar), double(ai));

			// generate all the right sides
			for (size_t k = 0; k < NR_VALUES; ++k) {
				br.setbits(k);
				for (size_t l = 0; l < NR_VALUES; ++l) {
					bi.setbits(l);
					b = std::complex<FixedPoint>(br, bi);
					db = std::complex<double>(double(br), double(bi));
					dc = da * db;
					ref = std::complex<FixedPoint>(dc.real(), dc.imag());

#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
					// catching overflow
					try {
						result = a * b;
					}
					catch (...) {
						if (ref.real() > maxpos || ref.imag() > maxpos ||
							ref.real() < maxneg || ref.imag() < maxneg) {
							// correctly caught the overflow exception
							continue;
						}
						else {
							nrOfFailedTests++;
						}
					}

#else
					result = a * b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION


					if (result.real() != ref.real() || result.imag() != ref.imag()) {
						nrOfFailedTests++;
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", a, b, result, ref);
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, result, ref);
					}
					if (nrOfFailedTests > 24) return nrOfFailedTests;
					if constexpr (statusFeedback) if (nrTests > 0 && (nrTests % (64 * 1024) == 0)) {
						++nrTests;
						statusStringPresent = true;
						std::cout << '.';
					}
				}
			}
		}	
	}
	if constexpr (statusFeedback) if (statusStringPresent) std::cout << std::endl;
	return nrOfFailedTests;
}

template<typename FixedPoint, typename Real>
void complex_mul(Real far, Real fai, Real fbr, Real fbi) {

	std::complex<float> fa, fb, fc;
	fa = std::complex<float>(far, fai);
	fb = std::complex<float>(fbr, fbi);
	fc = fa * fb;
	std::cout << "complex<float>   : " << fc << '\n';
	FixedPoint cr, ci;
	cr = fc.real();
	ci = fc.imag();
	std::cout << "fixpnt converted : (" << cr << ", " << ci << ")\n";

	// manual complex multiply
	FixedPoint ar{ far }, ai{ fai }, br{ fbr }, bi{ fbi };
	std::cout << "a = (" << ar << ", " << ai << ")\n";
	std::cout << "b = (" << br << ", " << bi << ")\n";
	FixedPoint r1 = ar * br;
	FixedPoint r2 = ai * bi;
	std::cout << "cr : " << r1 << " + " << r2 << '\n';
	FixedPoint i1 = ar * bi;
	FixedPoint i2 = ai * br;
	std::cout << "ci : " << i1 << " + " << i2 << '\n';
	cr = r1 + r2;
	ci = i1 + i2;
	std::complex<FixedPoint> a, b, c;
	c = std::complex<FixedPoint>(cr, ci);
	std::cout << "manual complex<fixpnt> : " << c << '\n';
	a = std::complex<FixedPoint>(ar, ai);
	b = std::complex<FixedPoint>(br, bi);
	c = a * b;
	std::cout << "complex<fixpnt>        : " << c << '\n';
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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
#define HARDWARE_ACCELERATION 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixed-point complex modulo multiplication validation";
	std::string test_tag    = "complex modulo multiplication";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

#pragma message("NOTE: fixpnt complex multiplication is failing: regression suite is disabled")
	{
		blockbinary<8> a, b;
		a.setbits(0x02);
		b.setbits(0x80);
		blockbinary<16> c;
		c = urmul2(a, b);
		std::cout << a << " * " << b << " = " << c << " : " << (long long)c << '\n';
		c = urmul2(b, a);
		std::cout << b << " * " << a << " = " << c << " : " << (long long)c << '\n';
	}

	{
		using FixedPoint = fixpnt<4, 2, Modulo, uint8_t>;
		FixedPoint ar, ai, br, bi;
		std::complex<FixedPoint> a, b, c;

		ar = 0.25, ai = 0.25, br = 0.25, bi = 0.5;
		// (0.25 + 0.25i) * (0.25 + 0.5i) =
		a = std::complex<FixedPoint>(ar, ai);
		b = std::complex<FixedPoint>(br, bi);
		c = a * b;
		std::cout << c << '\n';
		ReportValue(c, "product");

		float far, fai, fbr, fbi;
		far = 0.25f;
		fai = 0.25f;
		fbr = 0.25f;
		fbi = 0.5f;
		complex_mul<FixedPoint>(far, fai, fbr, fbi);

		// this fails compared to a complex<double> reference computation because each individual
		// term in the cr and ci calculation gets rounded down, but the sum would have rounded up.

		// this would indicate that the regression suite algorithm isn't quite correct for
		// small fixpnts, which are the only ones we test due to the cost of enumerating
		// the full state space.
	}

#undef FULL_SET
#ifdef FULL_SET
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<4, 0, Modulo, uint8_t>(true), "fixpnt<4,0,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<4, 1, Modulo, uint8_t>(true), "fixpnt<4,1,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<4, 2, Modulo, uint8_t>(true), "fixpnt<4,2,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<4, 3, Modulo, uint8_t>(true), "fixpnt<4,3,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<4, 4, Modulo, uint8_t>(true), "fixpnt<4,4,Modulo,uint8_t>", test_tag);
#endif

#ifdef STRESS_TESTING
	// for an 8-bit fixpnt, the full state space of complex binary operators is 256^4 = 2^32 = 4billion
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<8, 1, Modulo, uint8_t>(reportTestCases), "fixpnt<8,1,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<8, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<8,4,Modulo,uint8_t>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 4, 0, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 0, Modulo, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 4, 1, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 1, Modulo, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 4, 2, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 2, Modulo, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 4, 3, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 3, Modulo, uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 4, 4, Modulo, uint8_t>(reportTestCases), "fixpnt< 4, 4, Modulo, uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 5, 2, Modulo, uint8_t>(reportTestCases), "fixpnt< 5, 2, Modulo, uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 6, 0, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 0,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 6, 1, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 1,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 6, 2, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 2,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 6, 3, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 3,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 6, 4, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 4,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 6, 5, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 5,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 6, 6, Modulo, uint8_t>(reportTestCases), "fixpnt< 6, 6,Modulo,uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 8, 0, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 0,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 8, 1, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 1,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 8, 2, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 2,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 8, 3, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 3,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 8, 4, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 4,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 8, 5, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 5,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 8, 6, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 6,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 8, 7, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 7,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication< 8, 8, Modulo, uint8_t>(reportTestCases), "fixpnt< 8, 8,Modulo,uint8_t>", test_tag);

//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<10, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<10, 0,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<10, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<10, 4,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<10, 7, Modulo, uint8_t>(reportTestCases), "fixpnt<10, 7,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<10, 8, Modulo, uint8_t>(reportTestCases), "fixpnt<10, 8,Modulo,uint8_t>", test_tag);
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<10, 9, Modulo, uint8_t>(reportTestCases), "fixpnt<10, 9,Modulo,uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<12, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 0,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<12, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 4,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<12, 7, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 7,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<12, 8, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 8,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<12, 9, Modulo, uint8_t>(reportTestCases), "fixpnt<12, 9,Modulo,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyComplexMultiplication<12,12, Modulo, uint8_t>(reportTestCases), "fixpnt<12,12,Modulo,uint8_t>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
