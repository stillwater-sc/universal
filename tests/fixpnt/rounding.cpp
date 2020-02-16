// rounding.cpp: functional tests for fixed-point rounding 
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0

// minimum set of include files to reflect source code dependencies
#include "universal/fixpnt/fixed_point.hpp"
// fixed-point type manipulators such as pretty printers
#include "universal/fixpnt/fixpnt_manipulators.hpp"
#include "universal/fixpnt/math_functions.hpp"
#include "../utils/fixpnt_test_suite.hpp"

// generate specific test case that you can trace with the trace conditions in fixpnt.h
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

void setAccu(uint8_t accu[4], uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0) {
	accu[3] = b3;
	accu[2] = b2;
	accu[1] = b1;
	accu[0] = b0;
}

std::string roundingDecision(int roundingDirection) {
	std::stringstream ss;
	ss << (roundingDirection == 0 ? "tie" : (roundingDirection > 0 ? "up" : "down"));
	return ss.str();
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular assignment failed: ";

#if MANUAL_TESTING

	/*
accu= 0xFF81
FAIL                  0.5 *                -63.5 !=                -31.5 instead it yielded                -32.0 1100000.1 vs 1100000.0

accu= 0xFF83
FAIL                  0.5 *                -62.5 !=                -31.0 instead it yielded                -31.5 1100001.0 vs 1100000.1

accu= 0xFF85
FAIL                  0.5 *                -61.5 !=                -30.5 instead it yielded                -31.0 1100001.1 vs 1100001.0

accu= 0xFF87
FAIL                  0.5 *                -60.5 !=                -30.0 instead it yielded                -30.5 1100010.0 vs 1100001.1
	*/
	uint8_t accumulator[4];

	setAccu(accumulator, 0x00, 0x00, 0xFF, 0x81);

	fixpnt<8, 1> fp = 31.75;
	cout << to_binary(fp) << " " << fp << endl;

	cout << roundingDecision(sw::native::round(accumulator, 2, 0)) << endl;

	fixpnt<8, 1> a, b, c;
	a = 0.5f;
	b = -63.5f;
	c = a * b;
	cout << c << endl;

	//nrOfFailedTestCases = ReportTestResult(ValidateModularAssignment<4, 3, float>(bReportIndividualTestCases), tag, "posit<4,3>");
	
	// TODO: fixed-point is failing on pure fractional configurations
	//nrOfFailedTestCases = ReportTestResult(ValidateAssignment<4, 4, float>(bReportIndividualTestCases), tag, "posit<4,4>");

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else
	cout << "Fixed-point modular assignment validation" << endl;


//	nrOfFailedTestCases += ReportTestResult(VerifyModularAddition<8, 0>(tag, bReportIndividualTestCases), "fixpnt<8,0>", "addition");

#if STRESS_TESTING

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
