// assignment.cpp: test suite runner for fixed-point assignments from native types
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
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0

// minimum set of include files to reflect source code dependencies
#include <universal/number/fixpnt/fixpnt.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/number/fixpnt/math_functions.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

void PositiveTestCases() {
	using namespace std;
	using namespace sw::universal;

	float fa, fb, fc, fd;
	fixpnt<8, 4> a, b, c, d;

	cout << "POSITIVE TEST CASES\n";
	a.set_raw_bits(0x14);
	b.set_raw_bits(0x15);
	c.set_raw_bits(0x16);
	d.set_raw_bits(0x17);
	fa = float(a);
	fb = float(b);
	fc = float(c);
	fd = float(d);
	cout << to_binary(fa) << ' ' << fa << ' ' << to_binary(a) << ' ' << a << endl;
	cout << to_binary(fb) << ' ' << fb << ' ' << to_binary(b) << ' ' << b << endl;
	cout << to_binary(fc) << ' ' << fc << ' ' << to_binary(c) << ' ' << c << endl;
	cout << to_binary(fd) << ' ' << fd << ' ' << to_binary(d) << ' ' << d << endl;

	cout << to_hex(fa) << endl;
	cout << to_hex(fb) << endl;
	cout << to_hex(fc) << endl;
	cout << to_hex(fd) << endl;

	float eps[24] = { 0.0f };
	for (int i = 23; i >= 0; --i) {
		eps[i] = 1.0f / float(1 << i);
	}

	cout << to_binary(eps[20]) << endl;
	cout << to_binary(eps[21]) << endl;
	cout << to_binary(eps[22]) << endl;
	cout << to_binary(eps[23]) << endl;

	float mashup;
	fixpnt<8, 4> fixedPoint;
	cout << "fa + eps" << endl;
	/*
	for (int i = 5; i < 9; ++i) {
		mashup = fa + eps[i];
		fixedPoint = mashup;
		cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	}
	*/

	mashup = fa + eps[5];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fa + eps[5] + eps[6];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fa + eps[5] + eps[20];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fa + eps[6];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;

	cout << "fb + eps" << endl;
	mashup = fb + eps[5];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fb + eps[5] + eps[6];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fb + eps[5] + eps[20];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fb + eps[6];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
}

void NegativeTestCases() {
	using namespace std;
	using namespace sw::universal;

	float fa, fb, fc, fd;
	fixpnt<8, 4> a, b, c, d;

	cout << "NEGATIVE TEST CASES\n";
	a.set_raw_bits(~0x14u + 1u);
	b.set_raw_bits(~0x15u + 1u);
	c.set_raw_bits(~0x16u + 1u);
	d.set_raw_bits(~0x17u + 1u);
	fa = float(a);
	fb = float(b);
	fc = float(c);
	fd = float(d);
	cout << to_binary(fa) << ' ' << fa << ' ' << to_binary(a) << ' ' << a << endl;
	cout << to_binary(fb) << ' ' << fb << ' ' << to_binary(b) << ' ' << b << endl;
	cout << to_binary(fc) << ' ' << fc << ' ' << to_binary(c) << ' ' << c << endl;
	cout << to_binary(fd) << ' ' << fd << ' ' << to_binary(d) << ' ' << d << endl;

	cout << to_hex(fa) << endl;
	cout << to_hex(fb) << endl;
	cout << to_hex(fc) << endl;
	cout << to_hex(fd) << endl;

	float eps[24] = { 0.0f };
	for (int i = 23; i >= 0; --i) {
		eps[i] = 1.0f / float(1 << i);
	}

	cout << to_binary(eps[20]) << endl;
	cout << to_binary(eps[21]) << endl;
	cout << to_binary(eps[22]) << endl;
	cout << to_binary(eps[23]) << endl;

	float mashup;
	fixpnt<8, 4> fixedPoint;
	cout << "fa - eps" << endl;
	/*
	for (int i = 5; i < 9; ++i) {
		mashup = fa + eps[i];
		fixedPoint = mashup;
		cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	}
	*/

	mashup = fa - eps[5];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fa - eps[5] - eps[6];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fa - eps[5] - eps[20];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fa - eps[6];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;

	cout << "fb - eps" << endl;
	mashup = fb - eps[5];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fb - eps[5] - eps[6];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fb - eps[5] - eps[20];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
	mashup = fb - eps[6];
	fixedPoint = mashup;
	cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << endl;
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 0) { cout << argv[0] << endl; }

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular assignment: ";

#if MANUAL_TESTING

	PositiveTestCases();
	NegativeTestCases();

	{
		fixpnt<4, 1> a, b, c;
		// overflow test
		a = -4; cout << a << endl;  // rounds to 3.5
		b = 4.0f;
		c = a * b;
		cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << " " << c << endl;
	}

	{
		fixpnt<4, 1> a, b, c;
		// rounding test
		a = 0.5f; cout << a << endl;
		b = 0.5f;
		c = a * b;
		cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << " " << c << endl;
	}


	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 0, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 1, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 2, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 3, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,3,Modulo,uint8_t>");
	
	// TODO: fixed-point is failing on pure fractional configurations
	//nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 4,Modulo,uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,4,Modulo,uint8_t>");

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else
	cout << "Fixed-point modular assignment validation" << endl;

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 0, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 1, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 2, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 3, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<4,3,Modulo,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 0, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<6,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 1, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<6,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 2, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<6,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 3, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<6,3,Modulo,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 0, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<8,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 1, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<8,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 2, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<8,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 3, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<8,3,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 4, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<8,4,Modulo,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10, 0, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<10,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10, 1, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<10,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10, 2, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<10,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10, 3, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<10,3,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10, 4, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<10,4,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10, 5, Modulo, uint8_t, float>(bReportIndividualTestCases), tag, "fixpnt<10,5,Modulo,uint8_t>");

//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 0, Modulo,uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,0,Modulo,uint8_t>", "addition");

#if STRESS_TESTING

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
