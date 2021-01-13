// assignment.cpp: functional tests for assignments of native types to areals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the areal template environment
// first: enable general or specialized configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable areal arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0

// minimum set of include files to reflect source code dependencies
#include <universal/areal/areal.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/areal/manipulators.hpp>
#include <universal/areal/math_functions.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_suite_arithmetic.hpp>

void PositiveTestCases() {
	using namespace std;
	using namespace sw::universal;

	{
		float fa, fb, fc, fd;
		areal<8, 2> a, b, c, d;  // es = 2 -> e = 1 => 2^0 regime, so normal number all fraction 1 + f/2^fbits)

		std::cout << "POSITIVE TEST CASES\n";
		a.set_raw_bits(0x24);  // sign = 0, es = 01, f = 0'010, u = 0
		b.set_raw_bits(0x25);
		c.set_raw_bits(0x26);
		d.set_raw_bits(0x27);
		fa = float(a);
		fb = float(b);
		fc = float(c);
		fd = float(d);
		std::cout << to_binary(fa) << ' ' << fa << ' ' << to_binary(a) << ' ' << a << std::endl;
		std::cout << to_binary(fb) << ' ' << fb << ' ' << to_binary(b) << ' ' << b << std::endl;
		std::cout << to_binary(fc) << ' ' << fc << ' ' << to_binary(c) << ' ' << c << std::endl;
		std::cout << to_binary(fd) << ' ' << fd << ' ' << to_binary(d) << ' ' << d << std::endl;

//		std::cout << to_hex(fa) << std::endl;
//		std::cout << to_hex(fb) << std::endl;
//		std::cout << to_hex(fc) << std::endl;
//		std::cout << to_hex(fd) << std::endl;
	}

	{
		float eps[24] = { 0.0f };
		for (int i = 23; i >= 0; --i) {
			eps[i] = 1.0f / float(1 << i);
		}

		std::cout << to_binary(eps[20]) << std::endl;
		std::cout << to_binary(eps[21]) << std::endl;
		std::cout << to_binary(eps[22]) << std::endl;
		std::cout << to_binary(eps[23]) << std::endl;
	}

}

void NegativeTestCases() {
	using namespace std;
	using namespace sw::universal;

	float fa, fb, fc, fd;
	areal<8, 2> a, b, c, d;   // es = 2 -> e = 1 => 2^0 regime, so normal number all fraction 1 + f/2^fbits)

	std::cout << "NEGATIVE TEST CASES\n";
	a.set_raw_bits(~0x24 + 1);   // sign = 0, es = 01, f = 0'010, u = 0
	b.set_raw_bits(~0x25 + 1);
	c.set_raw_bits(~0x26 + 1);
	d.set_raw_bits(~0x27 + 1);
	fa = float(a);
	fb = float(b);
	fc = float(c);
	fd = float(d);
	std::cout << to_binary(fa) << ' ' << fa << ' ' << to_binary(a) << ' ' << a << std::endl;
	std::cout << to_binary(fb) << ' ' << fb << ' ' << to_binary(b) << ' ' << b << std::endl;
	std::cout << to_binary(fc) << ' ' << fc << ' ' << to_binary(c) << ' ' << c << std::endl;
	std::cout << to_binary(fd) << ' ' << fd << ' ' << to_binary(d) << ' ' << d << std::endl;

	//	std::cout << to_hex(fa) << std::endl;
	//	std::cout << to_hex(fb) << std::endl;
	//	std::cout << to_hex(fc) << std::endl;
	//	std::cout << to_hex(fd) << std::endl;
}

void Mashups() {
	using namespace sw::universal;

	areal<8, 2> a;
	a.set_raw_bits(~0x24 + 1);
	float fa = float(a);
	float fb = 0.0f;

	float eps[24] = { 0.0f };
	for (int i = 23; i >= 0; --i) {
		eps[i] = 1.0f / float(1 << i);
	}

	std::cout << to_binary(eps[20]) << std::endl;
	std::cout << to_binary(eps[21]) << std::endl;
	std::cout << to_binary(eps[22]) << std::endl;
	std::cout << to_binary(eps[23]) << std::endl;

	float mashup;
	areal<8, 2> af;
	std::cout << "fa - eps" << std::endl;
	/*
	for (int i = 5; i < 9; ++i) {
		mashup = fa + eps[i];
		a = mashup;
		cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(a) << ' ' << a << ' ' << to_triple(mashup) << endl;
	}
	*/

	mashup = fa - eps[5];
	af = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(af) << ' ' << af << ' ' << to_triple(mashup) << std::endl;
	mashup = fa - eps[5] - eps[6];
	af = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(af) << ' ' << af << ' ' << to_triple(mashup) << std::endl;
	mashup = fa - eps[5] - eps[20];
	af = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(af) << ' ' << af << ' ' << to_triple(mashup) << std::endl;
	mashup = fa - eps[6];
	af = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(af) << ' ' << af << ' ' << to_triple(mashup) << std::endl;

	std::cout << "fb - eps" << std::endl;
	mashup = fb - eps[5];
	af = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(af) << ' ' << af << ' ' << to_triple(mashup) << std::endl;
	mashup = fb - eps[5] - eps[6];
	af = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(af) << ' ' << af << ' ' << to_triple(mashup) << std::endl;
	mashup = fb - eps[5] - eps[20];
	af = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(af) << ' ' << af << ' ' << to_triple(mashup) << std::endl;
	mashup = fb - eps[6];
	af = mashup;
	std::cout << to_binary(mashup) << ' ' << mashup << ' ' << to_binary(af) << ' ' << af << ' ' << to_triple(mashup) << std::endl;
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "AREAL assignment: ";

#if MANUAL_TESTING

	PositiveTestCases();
	NegativeTestCases();

	{
		areal<5, 2> a, b, c;
		// overflow test
		a = -4; std::cout << a << endl;  // rounds to 3.5
		b = 4.0f;
		c = a * b;
		std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << " " << c << endl;
	}

	{
		areal<5, 2> a, b, c;
		// rounding test
		a = 0.5f; std::cout << a << endl;
		b = 0.5f;
		c = a * b;
		std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << " " << c << endl;
	}


	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<sw::universal::areal<5, 2,  uint8_t>, float >(bReportIndividualTestCases), tag, "areal<5,2,uint8_t>");
	
#if STRESS_TESTING

	// manual exhaustive test

#endif

#else
	cout << "AREAL assignment validation" << endl;

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<4, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<4,1,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<6, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<6,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<6, 2,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<6,2,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<8, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<8,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<8, 2,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<8,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<8, 3,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<8,3,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<10, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<10,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<10, 2,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<10,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::areal<10, 3,  uint8_t>, float>(bReportIndividualTestCases), tag, "areal<10,3,uint8_t>");


#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_arithmetic_exception& err) {
	std::cerr << "Uncaught areal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_internal_exception& err) {
	std::cerr << "Uncaught areal internal exception: " << err.what() << std::endl;
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
