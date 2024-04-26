// assignment.cpp: test suite runner for fixed-point assignments from native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

// PositiveTestCases and NegativeTestCases need to_hex function for native floating-point
#include <universal/native/ieee754.hpp>

void PositiveTestCases() {
	using namespace sw::universal;

	float fa, fb, fc, fd;
	fixpnt<8, 4> a, b, c, d;

	std::cout << "POSITIVE TEST CASES\n";
	a.setbits(0x14);
	b.setbits(0x15);
	c.setbits(0x16);
	d.setbits(0x17);
	fa = float(a);
	fb = float(b);
	fc = float(c);
	fd = float(d);
	std::cout << to_binary(fa) << ' ' << fa << ' ' << to_binary(a) << ' ' << a << '\n';
	std::cout << to_binary(fb) << ' ' << fb << ' ' << to_binary(b) << ' ' << b << '\n';
	std::cout << to_binary(fc) << ' ' << fc << ' ' << to_binary(c) << ' ' << c << '\n';
	std::cout << to_binary(fd) << ' ' << fd << ' ' << to_binary(d) << ' ' << d << '\n';

	std::cout << to_hex(fa) << '\n';
	std::cout << to_hex(fb) << '\n';
	std::cout << to_hex(fc) << '\n';
	std::cout << to_hex(fd) << '\n';

	float eps[24] = { 0.0f };
	for (int i = 23; i >= 0; --i) {
		eps[i] = 1.0f / float(1 << i);
	}

	std::cout << to_binary(eps[20]) << '\n';
	std::cout << to_binary(eps[21]) << '\n';
	std::cout << to_binary(eps[22]) << '\n';
	std::cout << to_binary(eps[23]) << '\n';

	float mashup;
	fixpnt<8, 4> fixedPoint;
	std::cout << "fa + eps" << '\n';
	/*
	for (int i = 5; i < 9; ++i) {
		mashup = fa + eps[i];
		fixedPoint = mashup;
		cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	}
	*/

	mashup = fa + eps[5];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fa + eps[5] + eps[6];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fa + eps[5] + eps[20];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fa + eps[6];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';

	std::cout << "fb + eps" << '\n';
	mashup = fb + eps[5];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fb + eps[5] + eps[6];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fb + eps[5] + eps[20];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fb + eps[6];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
}

void NegativeTestCases() {
	using namespace sw::universal;

	float fa, fb, fc, fd;
	fixpnt<8, 4> a, b, c, d;

	std::cout << "NEGATIVE TEST CASES\n";
	a.setbits(~0x14u + 1u);
	b.setbits(~0x15u + 1u);
	c.setbits(~0x16u + 1u);
	d.setbits(~0x17u + 1u);
	fa = float(a);
	fb = float(b);
	fc = float(c);
	fd = float(d);
	std::cout << to_binary(fa) << ' ' << fa << ' ' << to_binary(a) << ' ' << a << '\n';
	std::cout << to_binary(fb) << ' ' << fb << ' ' << to_binary(b) << ' ' << b << '\n';
	std::cout << to_binary(fc) << ' ' << fc << ' ' << to_binary(c) << ' ' << c << '\n';
	std::cout << to_binary(fd) << ' ' << fd << ' ' << to_binary(d) << ' ' << d << '\n';

	std::cout << to_hex(fa) << '\n';
	std::cout << to_hex(fb) << '\n';
	std::cout << to_hex(fc) << '\n';
	std::cout << to_hex(fd) << '\n';

	float eps[24] = { 0.0f };
	for (int i = 23; i >= 0; --i) {
		eps[i] = 1.0f / float(1 << i);
	}

	std::cout << to_binary(eps[20]) << '\n';
	std::cout << to_binary(eps[21]) << '\n';
	std::cout << to_binary(eps[22]) << '\n';
	std::cout << to_binary(eps[23]) << '\n';

	float mashup;
	fixpnt<8, 4> fixedPoint;
	std::cout << "fa - eps" << '\n';
	/*
	for (int i = 5; i < 9; ++i) {
		mashup = fa + eps[i];
		fixedPoint = mashup;
		std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	}
	*/

	mashup = fa - eps[5];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fa - eps[5] - eps[6];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fa - eps[5] - eps[20];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fa - eps[6];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';

	std::cout << "fb - eps" << '\n';
	mashup = fb - eps[5];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fb - eps[5] - eps[6];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fb - eps[5] - eps[20];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
	mashup = fb - eps[6];
	fixedPoint = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(fixedPoint) << ' ' << fixedPoint << ' ' << to_triple(mashup) << '\n';
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "Fixed-point modular assignment";
	std::string test_tag    = "modulo assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	PositiveTestCases();
	NegativeTestCases();

	{
		fixpnt<4, 1> a, b, c;
		// overflow test
		a = -4; std::cout << a << '\n';  // rounds to 3.5
		b = 4.0f;
		c = a * b;
		std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << " " << c << '\n';
	}

	{
		fixpnt<4, 1> a, b, c;
		// rounding test
		a = 0.5f; std::cout << a << '\n';
		b = 0.5f;
		c = a * b;
		std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << " " << c << '\n';
	}


	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 0, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<4,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 1, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<4,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 2, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<4,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 3, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<4,3,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 4, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<4,4,Modulo,uint8_t>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 4,  0, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 4, 0,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 4,  1, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 4, 1,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 4,  2, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 4, 2,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 4,  3, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 4, 3,Modulo,uint8_t >");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 6,  0, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 6, 0,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 6,  1, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 6, 1,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 6,  2, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 6, 2,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 6,  3, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 6, 3,Modulo,uint8_t >");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 8,  0, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 8, 0,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 8,  1, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 8, 1,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 8,  2, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 8, 2,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 8,  3, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 8, 3,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 8,  4, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 8, 4,Modulo,uint8_t >");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 8,  5, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 8, 5,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 8,  6, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 8, 6,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 8,  7, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 8, 7,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment< 8,  8, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt< 8, 8,Modulo,uint8_t >");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10,  0, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<10, 0,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10,  1, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<10, 1,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10,  2, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<10, 2,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10,  3, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<10, 3,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10,  4, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<10, 4,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<10,  5, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<10, 5,Modulo,uint8_t >");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<12,  0, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<12, 0,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<12,  1, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<12, 1,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<12,  2, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<12, 2,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<12,  3, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<12, 3,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<12,  4, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<12, 4,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<12,  5, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<12, 5,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<12,  6, Modulo, uint8_t, float>(reportTestCases), test_tag, "fixpnt<12, 6,Modulo,uint8_t >");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<14,  4, Modulo, uint8_t , float>(reportTestCases), test_tag, "fixpnt<14, 4,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<14,  8, Modulo, uint8_t , float>(reportTestCases), test_tag, "fixpnt<14, 8,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<14, 12, Modulo, uint8_t , float>(reportTestCases), test_tag, "fixpnt<14,12,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<14,  4, Modulo, uint16_t, float>(reportTestCases), test_tag, "fixpnt<14, 4,Modulo,uint16_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<14,  8, Modulo, uint16_t, float>(reportTestCases), test_tag, "fixpnt<14, 8,Modulo,uint16_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<14, 12, Modulo, uint16_t, float>(reportTestCases), test_tag, "fixpnt<14,12,Modulo,uint16_t>");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<16,  4, Modulo, uint8_t , float>(reportTestCases), test_tag, "fixpnt<16, 4,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<16,  8, Modulo, uint8_t , float>(reportTestCases), test_tag, "fixpnt<16, 8,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<16, 12, Modulo, uint8_t , float>(reportTestCases), test_tag, "fixpnt<16,12,Modulo,uint8_t >");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<16,  4, Modulo, uint16_t, float>(reportTestCases), test_tag, "fixpnt<16, 4,Modulo,uint16_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<16,  8, Modulo, uint16_t, float>(reportTestCases), test_tag, "fixpnt<16, 8,Modulo,uint16_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<16, 12, Modulo, uint16_t, float>(reportTestCases), test_tag, "fixpnt<16,12,Modulo,uint16_t>");
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
