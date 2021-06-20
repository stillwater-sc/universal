// addition.cpp: test suite runner for addition on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_ADD
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/verification/test_status.hpp>
//#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/utility/bit_cast.hpp>
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

template<typename Cfloat>
void testCfloatOrderedSet() {
	std::vector<Cfloat> set;
	GenerateOrderedCfloatSet<Cfloat>(set);
	for (auto v : set) {
		std::cout << to_binary(v) << " : " << v << '\n';
	}
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;
	std::string tag = "Addition failed: ";

#if MANUAL_TESTING

	{
		float fa = 0.0f; // .03125f;
//		float fb = std::numeric_limits<float>::signaling_NaN();
//		float fb = 0.0625f;
//		float fb = 7.625f;
		float fb = std::numeric_limits<float>::quiet_NaN();
		cfloat < 8, 2, uint8_t > a, b, c, cref;
		a = fa;
		b = fb;
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << '\n';

		c = 7.65625f;
		GenerateTestCase< cfloat<8, 2, uint8_t>, float>(fa, fb);
	}

	{
		cfloat<8, 2> c(SpecificValue::maxpos);
		cfloat<9, 2> d(SpecificValue::maxpos);
		std::cout << to_binary(c) << " : " << c << '\n';
		std::cout << to_binary(d) << " : " << d << '\n';
		d.setbits(0x0fa);
		std::cout << to_binary(d) << " : " << d << '\n';
		d.setbits(0x0fb);
		std::cout << to_binary(d) << " : " << d << '\n';

		std::cout << '\n';
		d = float(c);
		++d;
		std::cout << to_binary(d) << " : " << d << '\n';

		{
			cfloat<8,2> c(SpecificValue::maxneg);
			cfloat<9,2> d;
			d = double(c);
			std::cout << to_binary(d) << " : " << d << '\n';
			--d;
			std::cout << to_binary(d) << " : " << d << '\n';

		}
	}

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

	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatAddition< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(true), "cfloat<8,2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatAddition< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(true), "cfloat<8,4,uint8_t,subnormals,supernormals,!saturating>", "addition");

	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else
	cout << "classic floating-point addition validation" << endl;

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
