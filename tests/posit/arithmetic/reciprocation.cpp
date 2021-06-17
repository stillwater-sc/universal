// arithmetic_reciprocate.cpp: test suite runner for posit arithmetic reciprocation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define POSIT_VERBOSE_OUTPUT executing an reciprocate the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_RECIPROCATE
#define POSIT_TRACE_CONVERSION

// minimum set of include files to reflect source code dependencies
#include <universal/number/posit/posit_impl.hpp>
#include <universal/number/posit/numeric_limits.hpp>
#include <universal/number/posit/specializations.hpp>
// posit type manipulators such as pretty printers
#include <universal/number/posit/manipulators.hpp>
#include <universal/verification/posit_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in posit.hpp
// Most bugs are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty reference;
	sw::universal::posit<nbits, es> pa, pref, preciprocal;
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
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	cout << "Posit reciprocate validation" << endl;

	std::string tag = "Reciprocation failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	posit<5, 0> p1(0.75);
	posit<5, 0> p1_reciprocal;
	posit<5, 0> p2(0.75), p2_reciprocal;

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
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<3, 0>(tag, true), "posit<3,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<4, 0>(tag, true), "posit<4,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<5, 0>(tag, true), "posit<5,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<6, 0>(tag, true), "posit<6,0>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<5, 1>(tag, true), "posit<5,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<6, 1>(tag, true), "posit<6,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<7, 1>(tag, true), "posit<7,1>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<8, 2>(tag, true), "posit<8,2>", "reciprocation");

#else
	tag = "Reciprocation failed: ";
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<3, 0>(bReportIndividualTestCases), "posit<3,0>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<4, 0>(bReportIndividualTestCases), "posit<4,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<4, 1>(bReportIndividualTestCases), "posit<4,1>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<5, 0>(bReportIndividualTestCases), "posit<5,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<5, 1>(bReportIndividualTestCases), "posit<5,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<5, 2>(bReportIndividualTestCases), "posit<5,2>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<6, 0>(bReportIndividualTestCases), "posit<6,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<6, 1>(bReportIndividualTestCases), "posit<6,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<6, 2>(bReportIndividualTestCases), "posit<6,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<6, 3>(bReportIndividualTestCases), "posit<6,3>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<7, 0>(bReportIndividualTestCases), "posit<7,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<7, 1>(bReportIndividualTestCases), "posit<7,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<7, 2>(bReportIndividualTestCases), "posit<7,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<7, 3>(bReportIndividualTestCases), "posit<7,3>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<7, 4>(bReportIndividualTestCases), "posit<7,4>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<8, 0>(bReportIndividualTestCases), "posit<8,0>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<8, 1>(bReportIndividualTestCases), "posit<8,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<8, 2>(bReportIndividualTestCases), "posit<8,2>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<8, 3>(bReportIndividualTestCases), "posit<8,3>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<8, 4>(bReportIndividualTestCases), "posit<8,4>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<8, 5>(bReportIndividualTestCases), "posit<8,5>", "reciprocation");

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<10, 1>(bReportIndividualTestCases), "posit<10,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<12, 1>(bReportIndividualTestCases), "posit<12,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<14, 1>(bReportIndividualTestCases), "posit<14,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<16, 1>(bReportIndividualTestCases), "posit<16,1>", "reciprocation");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<18, 1>(bReportIndividualTestCases), "posit<18,1>", "reciprocation");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<20, 1>(bReportIndividualTestCases), "posit<20,1>", "reciprocation");

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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