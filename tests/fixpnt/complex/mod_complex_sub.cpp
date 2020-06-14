// mod_complex_sub.cpp: functional tests for arbitrary configuration fixed-point complex subtraction
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <complex>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/fixpnt/fixed_point.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/fixpnt/fixpnt_manipulators.hpp>
#include <universal/fixpnt/math_functions.hpp>
#include "../../utils/fixpnt_test_suite.hpp"

// generate specific test case that you can trace with the trace conditions in fixed_point.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::unum::fixpnt<nbits, rbits> a, b, cref, result;
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
int VerifyComplexSubtraction(const std::string& tag, bool bReportIndividualTestCases) {
	using namespace std;
	using namespace sw::unum;
	using FixedPoint = fixpnt<nbits, rbits, arithmetic, BlockType>;
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	FixedPoint fpmaxpos, fpmaxneg;
	maxpos<nbits, rbits, arithmetic, BlockType>(fpmaxpos);
	maxneg<nbits, rbits, arithmetic, BlockType>(fpmaxneg);
	int nrOfFailedTests = 0;
	FixedPoint ar, ai, br, bi, resultr, resulti;
	complex<FixedPoint> a, b, result, ref;

	complex<double> da, db, dc;
	for (size_t i = 0; i < NR_VALUES; i++) {
		ar.set_raw_bits(i);
		for (size_t j = 0; j < NR_VALUES; j++) {
			ar.set_raw_bits(j);
			a = complex<FixedPoint>(ar, ai);
			da = complex<double>(double(ar), double(ai));

			// generate all the right sides
			for (size_t k = 0; k < NR_VALUES; ++k) {
				br.set_raw_bits(k);
				for (size_t l = 0; l < NR_VALUES; ++l) {
					bi.set_raw_bits(l);
					b = complex<FixedPoint>(br, bi);
					db = complex<double>(double(br), double(bi));
					dc = da - db;
					ref = complex<FixedPoint>(dc.real(), dc.imag());

#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
					// catching overflow
					try {
						result = a - b;
					}
					catch (...) {
						if (ref.real() > fpmaxpos || ref.imag() > fpmaxpos ||
							ref.real() < fpmaxneg || ref.imag() < fpmaxneg) {
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
						if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, ref, result);
					}
					else {
						//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, ref, result);
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


// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "complex modulo subtraction failed: ";

#if MANUAL_TESTING

	bReportIndividualTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 1, Modulo, uint8_t>("Manual Testing", bReportIndividualTestCases), "fixpnt<4,1,Modulo,uint8_t>", "subtraction");

#if STRESS_TESTING
	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 0, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 1, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 2, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 3, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 4, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,4,Modulo,uint8_t>", "subtraction");
#endif

#else

	cout << "Fixed-point complex modulo subtraction validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<4, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,4,Modulo,uint8_t>", "subtraction");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,1,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,2,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,4,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,5,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 6, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,6,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 7, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,7,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<8, 8, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,8,Modulo,uint8_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<10, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<10,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<10, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<10,5,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<10, 7, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<10,7,Modulo,uint8_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<11, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<11,3,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<11, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<11,5,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<11, 7, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<11,7,Modulo,uint8_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<12, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<12,0,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<12, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<12,4,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<12, 8, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<12,8,Modulo,uint8_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexSubtraction<12, 12, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<12,12,Modulo,uint8_t>", "subtraction");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_internal_exception& err) {
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
