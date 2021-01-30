// quarter_precision.cpp: test suite runner for quarter-precision floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma warning(disable : 4514)
#pragma warning(disable : 4710)
#include <iostream>
#include <iomanip>
// minimum set of include files to reflect source code dependencies
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

// generate specific test case that you can trace with the trace conditions in areal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename TestType, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	TestType a, b, rref, rresult;
	a = a;
	b = b;
	rresult = a + b;
	// generate the reference
	Ty ref = _a + _b;
	rref = ref;

	constexpr size_t nbits = a.nbits; // number system concept
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << a << " + " << b << " = " << rresult << " (reference: " << rref << ")   ";
	std::cout << (rref == rresult ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 0) {
		cout << argv[0] << endl;
	}
	// const size_t RND_TEST_CASES = 0;  // no randoms, 8-bit posits can be done exhaustively

	constexpr size_t nbits = 8;
	constexpr size_t es = 2;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;
	std::string tag = " areal<8,2>";

	cout << "Standard quarter precision areal<8,2> configuration tests" << endl;

#if MANUAL_TESTING

	using TestType = areal<nbits, es, uint8_t>;
	using ReferenceType = float;

	{
		areal<64, 8> r;
		cout << to_binary(r, AREAL_NIBBLE_MARKER) << endl;
	}

	TestType r;
	cout << to_string(r) << endl; // test if default constructor generates 0

	r.set_raw_bits(0b1000'0000);
	cout << to_binary(r) << endl;
	cout << (r.isneg() ? "negative\n" : "positive\n");

#if 0
	ReferenceType a = 1.0f;
	ReferenceType b = 1.0f;
	GenerateTestCase<TestType, ReferenceType>(a, b);

	// conversion to native types are explicit
	// int anint = r;
	int anint = int(r);


	// logic tests
	nrOfFailedTestCases += ReportTestResult(VerifyLogicEqual             <TestType>(), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicNotEqual          <TestType>(), tag, "    !=         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessThan          <TestType>(), tag, "    <          ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicLessOrEqualThan   <TestType>(), tag, "    <=         ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterThan       <TestType>(), tag, "    >          ");
	nrOfFailedTestCases += ReportTestResult(VerifyLogicGreaterOrEqualThan<TestType>(), tag, "    >=         ");
	// conversion tests
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerConversion<TestType>(tag, bReportIndividualTestCases), tag, "integer assign ");
	nrOfFailedTestCases += ReportTestResult( VerifyConversion       <TestType>(tag, bReportIndividualTestCases), tag, "float assign   ");
#endif

#else // MANUAL_TESTING

	// arithmetic tests
//	nrOfFailedTestCases += ReportTestResult( VerifyAddition         <TestType>(tag, bReportIndividualTestCases), tag, "add            ");
//	nrOfFailedTestCases += ReportTestResult( VerifySubtraction      <TestType>(tag, bReportIndividualTestCases), tag, "subtract       ");
//	nrOfFailedTestCases += ReportTestResult( VerifyMultiplication   <TestType>(tag, bReportIndividualTestCases), tag, "multiply       ");
//	nrOfFailedTestCases += ReportTestResult( VerifyDivision         <TestType>(tag, bReportIndividualTestCases), tag, "divide         ");
//	nrOfFailedTestCases += ReportTestResult( VerifyNegation         <TestType>(tag, bReportIndividualTestCases), tag, "negate         ");
//	nrOfFailedTestCases += ReportTestResult( VerifyReciprocation    <TestType>(tag, bReportIndividualTestCases), tag, "reciprocate    ");

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
