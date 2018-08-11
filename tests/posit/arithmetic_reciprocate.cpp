// arithmetic_reciprocate.cpp: functional tests for arithmetic reciprocation
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

// when you define POSIT_VERBOSE_OUTPUT executing an reciprocate the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_RECIPROCATE
#define POSIT_TRACE_CONVERSION
// minimum set of include files to reflect source code dependencies
// enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#include "../../posit/posit.hpp"
#include "../../posit/posit_decoded.hpp"		// old reference design for validation/debug
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in posit.hpp
// Most bugs are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty reference;
	sw::unum::posit_decoded<nbits, es> pa, pref, preciprocal;
	pa = a;
	reference = (Ty)1.0 / a;
	pref = reference;
	preciprocal = pa.reciprocate();
	std::cout << "input " << a << " reference 1/fa " << reference << " pref " << double(pref) << '(' << pref << ") result " << double(preciprocal) << '(' << preciprocal << ')' << std::endl;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	cout << "Posit reciprocate validation" << endl;

	std::string tag = "Reciprocation failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	posit<5, 0> p1(0.75);
	posit<5, 0> p1_reciprocal;
	posit_decoded<5, 0> p2(0.75), p2_reciprocal;

	p2_reciprocal = p2.reciprocate();
	p1_reciprocal = p1.reciprocate();

	cout << "posit    : " << to_string(p1_reciprocal) << endl;
	cout << "reference: " << double(p2_reciprocal) << endl;

	GenerateTestCase<4, 0, double>(0.75);
	GenerateTestCase<5, 0, double>(0.75);
	GenerateTestCase<6, 0, double>(0.75);
	GenerateTestCase<16, 0, double>(0.75);
	posit<16, 0> p(1 / 0.75);
	cout << p.get() << " " << pretty_print(p, 17) << endl;

	tag = "Manual Testing: ";
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<3, 0>(tag, true), "posit<3,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<4, 0>(tag, true), "posit<4,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<5, 0>(tag, true), "posit<5,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 0>(tag, true), "posit<6,0>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<5, 1>(tag, true), "posit<5,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 1>(tag, true), "posit<6,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 1>(tag, true), "posit<7,1>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 2>(tag, true), "posit<8,2>", "reciprocation");

#else
	tag = "Reciprocation failed: ";
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "reciprocation");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<18, 1>(tag, bReportIndividualTestCases), "posit<18,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(ValidateReciprocation<20, 1>(tag, bReportIndividualTestCases), "posit<20,1>", "reciprocation");

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::posit_internal_exception& err) {
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