// conversion.cpp: functional tests for fixed-point conversions
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

template<size_t nbits, size_t rbits>
void GenerateFixedPointValues() {
	constexpr size_t NR_TEST_CASES = (size_t(1) << nbits);
	sw::unum::fixpnt<nbits, rbits> a;
	for (size_t i = 0; i < NR_TEST_CASES; ++i) {
		a.set_raw_bits(i);
		std::cout << to_binary(a) << " | " << a << std::endl;
	}
}


// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "conversion failed: ";

#if MANUAL_TESTING

	fixpnt<4, 1> f4_1;
	fixpnt<8, 1> f8_1;
	f4_1 = 4.5f;
	cout << to_binary(f4_1) << " " << f4_1 << endl;
	f8_1 = f4_1;
	cout << to_binary(f8_1) << " " << f8_1 << endl;

	ReportFixedPointRanges<4, 0, Modular>(cout);
	ReportFixedPointRanges<4, 1, Modular>(cout);
	ReportFixedPointRanges<4, 2, Modular>(cout);
	ReportFixedPointRanges<4, 3, Modular>(cout);
	ReportFixedPointRanges<4, 4, Modular>();

	return 0;

	GenerateFixedPointValues<4, 0>();
	GenerateFixedPointValues<4, 1>();

	return 0;

	nrOfFailedTestCases = ReportTestResult(ValidateModularConversion<4, 0>(tag, bReportIndividualTestCases), tag, "posit<4,0>");
	//nrOfFailedTestCases = ReportTestResult(ValidateModularConversion<4, 1>(tag, bReportIndividualTestCases), tag, "posit<4,1>");
	//nrOfFailedTestCases = ReportTestResult(ValidateModularConversion<4, 2>(tag, bReportIndividualTestCases), tag, "posit<4,2>");
	//nrOfFailedTestCases = ReportTestResult(ValidateModularConversion<4, 3>(tag, bReportIndividualTestCases), tag, "posit<4,3>");
	//nrOfFailedTestCases = ReportTestResult(ValidateModularConversion<4, 4>(tag, bReportIndividualTestCases), tag, "posit<4,4>");

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else

	cout << "Fixed-point conversion validation" << endl;

//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 0>(tag, bReportIndividualTestCases), "fixpnt<8,0>", "addition");

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
