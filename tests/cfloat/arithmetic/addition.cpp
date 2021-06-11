// addition.cpp: test suite runner for addition on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#define BLOCKTRIPLE_VERBOSE_OUTPUT
#define BLOCKTRIPLE_TRACE_ADD
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

// generate specific test case that you can trace with the trace conditions in cfloat.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename cfloatConfiguration, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	constexpr size_t nbits = cfloatConfiguration::nbits;
	cfloatConfiguration a, b, sum, ref;
	a = _a;
	b = _b;
	sum = a + b;
	// generate the reference
	Ty reference = _a + _b;
	ref = reference;

	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << reference << std::endl;
	std::cout << a << " + " << b << " = " << sum << " (reference: " << ref << ")   ";
	std::cout << to_binary(a, true) << " + " << to_binary(b, true) << " = " << to_binary(sum, true) << " (reference: " << to_binary(ref, true) << ")   ";
	std::cout << (ref == sum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<typename Real>
void test754functions(Real value) {
	using namespace std;
	using namespace sw::universal;
	cout << to_hex(value) << '\n';
	cout << to_binary(value) << '\n';
	cout << to_triple(value) << '\n';
	cout << to_base2_scientific(value) << '\n';
	cout << color_print(value) << '\n';
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;
	std::string tag = "Addition failed: ";

#if MANUAL_TESTING

	// FAIL              0.03125 +               0.0625 !=              0.09375 golden reference is             -0.15625 b0.00.00011 vs b1.00.00101
	// FAIL              0.03125 +              -0.0625 !=             -0.03125 golden reference is              0.21875 b1.00.00001 vs b0.00.00111
	{
		float fa = 0.03125f;
//		float fb = std::numeric_limits<float>::signaling_NaN();
		float fb = 0.0625f;
//		float fb = -0.0625f;
		cfloat < 8, 2, uint8_t > a, b, c, cref;
		a = fa;
		b = fb;
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << '\n';

		std::cout << '\n';
		b = -fb;
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << '\n';

		//GenerateTestCase< cfloat<8, 2, uint8_t>, float>(fa, fb);
	}
	return 0;

#ifdef LATER

	std::cout << "single precision IEEE-754\n";
	float f = 1.06125f;
	test754functions(f);
	std::cout << "double precision IEEE-754\n";
	double d = 1.06125;
	test754functions(d);


	// generate individual testcases to hand trace/debug
	GenerateTestCase< cfloat<8, 2, uint8_t>, float>(1.0f, 1.0f);

	GenerateTestCase< cfloat<16, 8, uint16_t>, double>(INFINITY, INFINITY);
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< cfloat<8, 2, uint8_t> >(true), "cfloat<8,2,uint8_t>", "addition");

	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else
	cout << "Arbitrary Real addition validation" << endl;

	bool bReportIndividualTestCases = false;

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(bReportIndividualTestCases), "cfloat<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(bReportIndividualTestCases), "cfloat<8,4>", "addition");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<10, 4>(bReportIndividualTestCases), "cfloat<10,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<16, 8>(bReportIndividualTestCases), "cfloat<16,8>", "addition");
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_divide_by_zero& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
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
