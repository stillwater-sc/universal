// mod_complex_sub.cpp: test suite runner for arbitrary configuration fixed-point complex subtraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <complex>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in fixed_point.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::fixpnt<nbits, rbits> a, b, cref, result;
	a = _a;
	b = _b;
	result = a + b;
	ref = _a + _b;
	cref = ref;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << a << " + " << b << " = " << result << " (reference: " << cref << ")   " ;
	std::cout << (cref == result ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// enumerate all complex subtraction cases for an fixpnt<nbits,rbits> configuration
template<size_t nbits, size_t rbits, bool arithmetic, typename BlockType>
int VerifyComplexSubtraction(bool reportTestCases) {
	using namespace sw::universal;
	using FixedPoint = fixpnt<nbits, rbits, arithmetic, BlockType>;
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	FixedPoint maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	FixedPoint ar{ 0 }, ai{ 0 }, br{ 0 }, bi{ 0 };
	std::complex<FixedPoint> a, b, result, ref;
	int nrOfFailedTests = 0;
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
					dc = da - db;
					ref = std::complex<FixedPoint>(dc.real(), dc.imag());

#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
					// catching overflow
					try {
						result = a - b;
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
					result = a - b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION


					if (result.real() != ref.real() || result.imag() != ref.imag()) {
						nrOfFailedTests++;
						if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, result, ref);
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, result, ref);
					}
					if (nrOfFailedTests > 100) return nrOfFailedTests;
				}
			}
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}


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
#define HARDWARE_ACCELERATION 0
#define HARDWARE_ACCELERATION 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixed-point complex subtraction validation";
	std::string test_tag    = "complex modular addition";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 1, Modulo, uint8_t>(true), "fixpnt<4,1,Modulo,uint8_t>", "subtraction");

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 0, Modulo, uint8_t>(true), "fixpnt<4,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 1, Modulo, uint8_t>(true), "fixpnt<4,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 2, Modulo, uint8_t>(true), "fixpnt<4,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 3, Modulo, uint8_t>(true), "fixpnt<4,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 4, Modulo, uint8_t>(true), "fixpnt<4,4,Modulo,uint8_t>", "subtraction");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	std::cout << "Fixed-point complex modulo subtraction validation\n";

#if REGRESSION_LEVEL_1
	// 4-bits: 2^16 arithmetic combinations
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<4,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 1, Modulo, uint8_t>(reportTestCases), "fixpnt<4,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 2, Modulo, uint8_t>(reportTestCases), "fixpnt<4,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 3, Modulo, uint8_t>(reportTestCases), "fixpnt<4,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<4,4,Modulo,uint8_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_2
	// 5-bits: 2^20 arithmetic combinations
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<5, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<5,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<5, 1, Modulo, uint8_t>(reportTestCases), "fixpnt<5,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<5, 2, Modulo, uint8_t>(reportTestCases), "fixpnt<5,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<5, 3, Modulo, uint8_t>(reportTestCases), "fixpnt<5,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<5, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<5,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<5, 5, Modulo, uint8_t>(reportTestCases), "fixpnt<5,1,Modulo,uint8_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_3
	// 6-bits: 2^24 arithmetic combinations
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<6, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<6,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<6, 1, Modulo, uint8_t>(reportTestCases), "fixpnt<6,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<6, 2, Modulo, uint8_t>(reportTestCases), "fixpnt<6,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<6, 3, Modulo, uint8_t>(reportTestCases), "fixpnt<6,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<6, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<6,4,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<6, 5, Modulo, uint8_t>(reportTestCases), "fixpnt<6,5,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<6, 6, Modulo, uint8_t>(reportTestCases), "fixpnt<6,6,Modulo,uint8_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_4
#if HARDWARE_ACCELERATION
	// an 8bit base type in complex arithmetic yields 2^16 possibilities
    // and 2^32 arithmetic combinations

    // the following test scenarios are only feasible with hardware acceleration
    // 8-bits: 2^32 arithmetic combinations
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<8,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 1, Modulo, uint8_t>(reportTestCases), "fixpnt<8,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 2, Modulo, uint8_t>(reportTestCases), "fixpnt<8,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 3, Modulo, uint8_t>(reportTestCases), "fixpnt<8,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<8,4,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 5, Modulo, uint8_t>(reportTestCases), "fixpnt<8,5,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 6, Modulo, uint8_t>(reportTestCases), "fixpnt<8,6,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 7, Modulo, uint8_t>(reportTestCases), "fixpnt<8,7,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 8, Modulo, uint8_t>(reportTestCases), "fixpnt<8,8,Modulo,uint8_t>", "subtraction");

	// 10-bits: 2^40 arithmetic combinations
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<10, 3, Modulo, uint8_t>(reportTestCases), "fixpnt<10,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<10, 5, Modulo, uint8_t>(reportTestCases), "fixpnt<10,5,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<10, 7, Modulo, uint8_t>(reportTestCases), "fixpnt<10,7,Modulo,uint8_t>", "subtraction");

	// 11-bits: 2^44 arithmetic combinations
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<11, 3, Modulo, uint8_t>(reportTestCases), "fixpnt<11,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<11, 5, Modulo, uint8_t>(reportTestCases), "fixpnt<11,5,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<11, 7, Modulo, uint8_t>(reportTestCases), "fixpnt<11,7,Modulo,uint8_t>", "subtraction");

	// 12-bits: 2^48 arithmetic combinations
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<12, 0, Modulo, uint8_t>(reportTestCases), "fixpnt<12,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<12, 4, Modulo, uint8_t>(reportTestCases), "fixpnt<12,4,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<12, 8, Modulo, uint8_t>(reportTestCases), "fixpnt<12,8,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<12, 12, Modulo, uint8_t>(reportTestCases), "fixpnt<12,12,Modulo,uint8_t>", "subtraction");
#endif // HARDWARE_ACCELERATION
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
