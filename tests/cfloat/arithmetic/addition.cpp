// addition.cpp: test suite runner for addition on arbitrary reals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

// generate specific test case that you can trace with the trace conditions in bfloat.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename BfloatConfiguration, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	constexpr size_t nbits = BfloatConfiguration::nbits;
	BfloatConfiguration a, b, sum, ref;
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

	bfloat < 8, 2, uint8_t > a, b, c, cref;
	a = 0.3125f;
	b = 0.5f;
	c = a + b;
	std::cout << a << " + " << b << " = " << c << '\n';
	std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << '\n';
	// FAIL              0.03125 + 0.5 != 0.53125 golden reference is               0.5625 b0.00.10001 vs b0.00.10010
	GenerateTestCase< bfloat<8, 2, uint8_t>, float>(0.03125f, 0.5f);

	std::cout << "single precision IEEE-754\n";
	float f = 1.06125f;
	test754functions(f);
	std::cout << "double precision IEEE-754\n";
	double d = 1.06125;
	test754functions(d);

	{
		float f0 = 0.5f;
		float f1 = 0.5625f;
		float f2 = 0.53125f;
		bfloat<8, 2> s;
		s = f0; std::cout << to_binary(s) << " : " << s << '\n';
		s = f1; std::cout << to_binary(s) << " : " << s << '\n';
		s = f2; std::cout << to_binary(s) << " : " << s << '\n';
	}
	{
		float f1 = 0.5625f;
		float f2 = 0.53125f;
		bfloat<32, 8> s;
		s = f1; 
		std::cout << to_binary(s) << " : " << s << '\n';
		s = f2; 
		std::cout << to_binary(s) << " : " << s << '\n';
	}
	{
		float f1 = 0.5625f;
		float f2 = 0.53125f;
		bfloat<64, 11> s;
		s = f1;
		std::cout << to_binary(s) << " : " << s << '\n';
		s = f2; 
		std::cout << to_binary(s) << " : " << s << '\n';
	}
	{
		double f0 = 0.5f;
		double f1 = 0.5625f;
		double f2 = 0.53125f;
		bfloat<8, 2> s;
		s = f0; std::cout << to_binary(s) << " : " << s << '\n';
		s = f1; std::cout << to_binary(s) << " : " << s << '\n';
		s = f2; std::cout << to_binary(s) << " : " << s << '\n';
	}
	{
		double f1 = 0.5625f;
		double f2 = 0.53125f;
		bfloat<32, 8> s;
		s = f1;
		std::cout << to_binary(s) << " : " << s << '\n';
		s = f2; 
		std::cout << to_binary(s) << " : " << s << '\n';
	}
	{
		double f1 = 0.5625f;
		double f2 = 0.53125f;
		bfloat<64, 11> s;
		s = f1;
		std::cout << to_binary(s) << " : " << s << '\n';
		s = f2; 
		std::cout << to_binary(s) << " : " << s << '\n';
	}
	return 0;


	a.setzero();
//	b.setnan(NAN_TYPE_SIGNALLING);
	b.setnan(NAN_TYPE_QUIET);
	b.setbits(0x7f);
	c = a + b;
	float _a = float(a);
	float _b = float(b);
	float _c = _a + _b;
	cref = c;
	std::cout << c << " vs " << _c << " vs " << cref << std::endl;
	if (cref == c) std::cout << "PASS\n";

//	a.constexprClassParameters();

	// generate individual testcases to hand trace/debug
	GenerateTestCase< bfloat<8, 2, uint8_t>, float>(1.0f, 1.0f);

	GenerateTestCase< bfloat<16, 8, uint16_t>, double>(INFINITY, INFINITY);

	nrOfFailedTestCases += ReportTestResult(VerifyAddition< bfloat<8, 2, uint8_t> >(true), "bfloat<8,2,uint8_t>", "addition");

	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else
	cout << "Arbitrary Real addition validation" << endl;

	bool bReportIndividualTestCases = false;

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(bReportIndividualTestCases), "bfloat<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(bReportIndividualTestCases), "bfloat<8,4>", "addition");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<10, 4>(bReportIndividualTestCases), "bfloat<10,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<16, 8>(bReportIndividualTestCases), "bfloat<16,8>", "addition");
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::bfloat_divide_by_zero& err) {
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
