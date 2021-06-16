// mod_multiplication.cpp: test suite runner for arbitrary configuration fixed-point modulo multiplication
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/number/fixpnt/fixpnt_impl.hpp>
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/number/fixpnt/math_functions.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

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

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular multiplication failed: ";

#if MANUAL_TESTING

	{
		blockbinary<8> a, b;
		a.set_raw_bits(0x02);
		b.set_raw_bits(0x80);
		blockbinary<16> c;
		c = urmul2(a, b);
		cout << a << " * " << b << " = " << c << " : " << (long long)c << endl;
		c = urmul2(b, a);
		cout << b << " * " << a << " = " << c << " : " << (long long)c << endl;
	}

	float fa = -8.0f;
	float fb = 0.125f;
	GenerateTestCase<8, 4>(fa, fb);
	GenerateTestCase<8, 4>(fb, fa);

	// generate individual testcases to hand trace/debug
	GenerateTestCase<4, 1>(-0.5f, -3.5f);
	GenerateTestCase<4, 1>(-3.5f, -0.5f);

	//	GenerateTestCase<8, 1>(0.5f, 0.5f);
	GenerateTestCase<8, 1>(0.5f, -32.0f);
	GenerateTestCase<8, 1>(-64.0f, 0.5f);
	GenerateTestCase<8, 1>(0.0f, -64.0f);
	GenerateTestCase<8, 1>(1.5f, -16.0f);
	GenerateTestCase<8, 1>(1.5f, -64.0f);
	GenerateTestCase<8, 1>(-64.0f, -63.5f);
	GenerateTestCase<8, 1>(-63.5f, -64.0f);
	GenerateTestCase<8, 1>(-64.0f, -63.0f);
	GenerateTestCase<8, 1>(-64.0f, -62.5f);

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 1, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<8,1,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 4, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<8,4,Modulo,uint8_t>", "multiplication");

#if STRESS_TESTING
	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 0, Modulo, uint8_t>(true), "fixpnt<4,0,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 1, Modulo, uint8_t>(true), "fixpnt<4,1,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 2, Modulo, uint8_t>(true), "fixpnt<4,2,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 3, Modulo, uint8_t>(true), "fixpnt<4,3,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 4, Modulo, uint8_t>(true), "fixpnt<4,4,Modulo,uint8_t>", "multiplication");
#endif

	nrOfFailedTestCases = 0; // ignore any failures in MANUAL mode
#else

	cout << "Fixed-point modular multiplication validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 0, Modulo, uint8_t>(true), "fixpnt<4,0,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 1, Modulo, uint8_t>(true), "fixpnt<4,1,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 2, Modulo, uint8_t>(true), "fixpnt<4,2,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 3, Modulo, uint8_t>(true), "fixpnt<4,3,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<4, 4, Modulo, uint8_t>(true), "fixpnt<4,4,Modulo,uint8_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<6, 0, Modulo, uint8_t>(true), "fixpnt<6,0,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<6, 1, Modulo, uint8_t>(true), "fixpnt<6,1,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<6, 2, Modulo, uint8_t>(true), "fixpnt<6,2,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<6, 3, Modulo, uint8_t>(true), "fixpnt<6,3,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<6, 4, Modulo, uint8_t>(true), "fixpnt<6,4,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<6, 5, Modulo, uint8_t>(true), "fixpnt<6,5,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<6, 6, Modulo, uint8_t>(true), "fixpnt<6,6,Modulo,uint8_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 0, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<8,0,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 1,Modulo,uint8_t>(bReportIndividualTestCases), "fixpnt<8,1,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 2,Modulo,uint8_t>(bReportIndividualTestCases), "fixpnt<8,2,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 3,Modulo,uint8_t>(bReportIndividualTestCases), "fixpnt<8,3,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 4,Modulo,uint8_t>(bReportIndividualTestCases), "fixpnt<8,4,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 5,Modulo,uint8_t>(bReportIndividualTestCases), "fixpnt<8,5,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 6,Modulo,uint8_t>(bReportIndividualTestCases), "fixpnt<8,6,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 7,Modulo,uint8_t>(bReportIndividualTestCases), "fixpnt<8,7,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 8,Modulo,uint8_t>(bReportIndividualTestCases), "fixpnt<8,8,Modulo,uint8_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 0, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<10,0,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 4, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<10,4,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 7, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<10,7,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 8, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<10,8,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<10, 9, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<10,9,Modulo,uint8_t>", "multiplication");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 0, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<12,0,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 4, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<12,4,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 7, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<12,7,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 8, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<12,8,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 9, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<12,9,Modulo,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<12, 12, Modulo, uint8_t>(bReportIndividualTestCases), "fixpnt<12,12,Modulo,uint8_t>", "multiplication");


#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
