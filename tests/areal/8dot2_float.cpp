// 8dot2_float.cpp: Functionality tests for 8bit precision floats
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
// TODO: the dependency on posit exceptions and trace_constants need to be removed
#include "universal/posit/exceptions.hpp"
#include "universal/posit/trace_constants.hpp"
#include "universal/bitblock/bitblock.hpp"
#include "universal/areal/areal.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "areal_test_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in areal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::unum::areal<nbits, es> pa, pb, pref, psum;
	pa = a;
	pb = b;
	ref = a + b;
	pref = ref;
	psum = pa + pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " + " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " + " << pb.get() << " = " << psum.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == psum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	// const size_t RND_TEST_CASES = 0;  // no randoms, 8-bit posits can be done exhaustively

	const size_t nbits = 8;
	const size_t es = 2;

	int nrOfFailedTestCases = 0;
	std::string tag = " areal<8,2>";

	cout << "Standard areal<8,2> configuration tests" << endl;

	areal<nbits, es> r;
	r = 0.0;
	cout << r << endl;

	float a = 1.0f;
	float b = 1.0f;
	GenerateTestCase<nbits, es, float>(a, b);

#if 0
	bool bReportIndividualTestCases = false;

	// logic tests
	nrOfFailedTestCases += ReportTestResult(ValidateArealLogicEqual             <nbits, es>(), tag, "    ==         ");
	nrOfFailedTestCases += ReportTestResult(ValidateArealLogicNotEqual          <nbits, es>(), tag, "    !=         ");
	nrOfFailedTestCases += ReportTestResult(ValidateArealLogicLessThan          <nbits, es>(), tag, "    <          ");
	nrOfFailedTestCases += ReportTestResult(ValidateArealLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         ");
	nrOfFailedTestCases += ReportTestResult(ValidateArealLogicGreaterThan       <nbits, es>(), tag, "    >          ");
	nrOfFailedTestCases += ReportTestResult(ValidateArealLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         ");
	// conversion tests
	nrOfFailedTestCases += ReportTestResult( ValidateIntegerConversion<nbits, es>(tag, bReportIndividualTestCases), tag, "integer assign ");
	nrOfFailedTestCases += ReportTestResult( ValidateConversion       <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign   ");
	// arithmetic tests
	nrOfFailedTestCases += ReportTestResult( ValidateAddition         <nbits, es>(tag, bReportIndividualTestCases), tag, "add            ");
	nrOfFailedTestCases += ReportTestResult( ValidateSubtraction      <nbits, es>(tag, bReportIndividualTestCases), tag, "subtract       ");
	nrOfFailedTestCases += ReportTestResult( ValidateMultiplication   <nbits, es>(tag, bReportIndividualTestCases), tag, "multiply       ");
	nrOfFailedTestCases += ReportTestResult( ValidateDivision         <nbits, es>(tag, bReportIndividualTestCases), tag, "divide         ");
	nrOfFailedTestCases += ReportTestResult( ValidateNegation         <nbits, es>(tag, bReportIndividualTestCases), tag, "negate         ");
	nrOfFailedTestCases += ReportTestResult( ValidateReciprocation    <nbits, es>(tag, bReportIndividualTestCases), tag, "reciprocate    ");
#endif

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
