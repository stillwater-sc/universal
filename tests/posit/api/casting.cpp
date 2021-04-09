// casting.cpp : test suite runner for casting operators between posit configurations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// if you want to trace the posit intermediate results
#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_CONVERT
// enable the ability to use literals in binary logic and arithmetic operators
#define POSIT_ENABLE_LITERALS 1
// enable fast posits
#define POSIT_FAST_POSIT_64_3 1
#include <universal/number/posit/posit>
#include <universal/verification/posit_test_suite.hpp>

template<size_t nbits, size_t es>
unsigned ValidateCasting(const std::string& tag, bool bReportIndividualTestCases) {
	unsigned nrOfFailedTestCases = 0;

	return nrOfFailedTestCases;
}

// generate specific test case that you can trace with the trace conditions in posit.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es>
void GenerateTestCase(float input, float reference, const sw::universal::posit<nbits, es>& presult) {
	if (fabs(double(presult) - reference) > 0.000000001) 
		ReportConversionError("test_case", "=", input, reference, presult);
	else
		ReportConversionSuccess("test_case", "=", input, reference, presult);
	std::cout << std::endl;
}

template<size_t nbits, size_t es>
void GenerateTestCase(double input, double reference, const sw::universal::posit<nbits, es>& presult) {
	if (fabs(double(presult) - reference) > 0.000000001)
		ReportConversionError("test_case", "=", input, reference, presult);
	else
		ReportConversionSuccess("test_case", "=", input, reference, presult);
	std::cout << std::endl;
}

void TestCase1() {
	using namespace std;
	using namespace sw::universal;

	posit<64, 1> p;
	p.set_raw_bits(0x7B32352A00000013);

	cout << color_print(p) << " " << p << endl;

	unsigned long ul = 0x80000000;
	posit<32, 2> p32_2 = ul;
	cout << color_print(p32_2) << " " << pretty_print(p32_2) << " " << hex_print(p32_2) << endl;

	// Posit = 0x7B32352A00000013
	// Stillwater’s result = 0x434C8D4A
	// Expected result = 0x434C8D4B

	float f = float(p);
	double d = double(p);
	//	long double ld = (long double)(p);

	p = f;
	cout << color_print(p) << " " << p << endl;

	uint32_t fh = *(uint32_t*)&f;
	uint64_t dh = *(uint64_t*)&d;
	cout << "SP Float = " << hexfloat << f << " " << defaultfloat << f << " " << fixed << f << " " << hex << fh << endl;
	cout << "DP Float = " << hexfloat << d << " " << defaultfloat << d << " " << hex << dh << endl;

	f = float(d);
	cout << "SP Float = " << hexfloat << f << " " << defaultfloat << f << " " << fixed << f << " " << hex << fh << endl;

	// s rrrrr e ffff'ffff'ffff'ffff'ffff'ffff'ffff'ffff'ffff'ffff'ffff'ffff'ffff'ffff'f
	// 0 11110 1 1001'1001'0001'1010'1001'0101'0000'0000'0000'0000'0000'0000'0000'1001'1 +204.552
	// 0 11110 1 1001'1001'0001'1010'1001'0100'0000'0000'0000'0000'0000'0000'0000'0000'0 +204.552
}
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;


	std::string tag = "Conversion test";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	// manual exhaustive testing
	tag = "Manual Testing";

	using T = sw::universal::posit<64, 3>;
	auto val = T(9.01);
	auto product = val * std::numeric_limits<T>::min();
	cout << "val     : " << color_print(val) << " : " << val << endl;
	cout << "product : " << color_print(product) << " : " << product << endl;

	return EXIT_SUCCESS;

#else

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	cout << "Posit casting validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateConversion< 8, 0>(tag, bReportIndividualTestCases), "posit<9,3>", "conversion");


#if STRESS_TESTING

#endif // STRESS_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING

}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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

