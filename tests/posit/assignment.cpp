// assignment.cpp : tests for native type literal assignments for posits
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "universal/posit/posit.hpp"
#include "universal/posit/manipulators.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/posit_math_helpers.hpp"

#define FLOAT_TABLE_WIDTH 20

template<size_t nbits, size_t es, typename Ty>
void ReportAssignmentError(const std::string& test_case, const std::string& op, const sw::universal::posit<nbits, es>& pref, const sw::universal::posit<nbits, es>& presult, const Ty& value) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(FLOAT_TABLE_WIDTH) << value
		<< " != "
		<< std::setw(FLOAT_TABLE_WIDTH) << pref << " instead it yielded "
		<< std::setw(FLOAT_TABLE_WIDTH) << presult
		<< " " << presult.get() << " vs " << pref.get() << std::endl;
}

template<size_t nbits, size_t es, typename Ty>
void ReportAssignmentSuccess(const std::string& test_case, const std::string& op, const sw::universal::posit<nbits, es>& pref, const sw::universal::posit<nbits, es>& presult, const Ty& value) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(FLOAT_TABLE_WIDTH) << value
		<< " == "
		<< std::setw(FLOAT_TABLE_WIDTH) << presult << " reference value is "
		<< std::setw(FLOAT_TABLE_WIDTH) << pref
		<< "               posit fields " << sw::universal::pretty_print(presult) << std::endl;
}

template<size_t nbits, size_t es, typename Ty>
Ty GenerateValue(const sw::universal::posit<nbits,es>& p) {
	Ty value = 0;
	if (std::numeric_limits<Ty>::is_exact) {
		if (std::numeric_limits<Ty>::is_signed) {
			value = (long long)(p);
		}
		else {
			value = (unsigned long long)(p);
		}
	}
	else {
		value = (long double)(p);
	}
	return value;
}

template<size_t nbits, size_t es, typename Ty>
int ValidateAssignment(bool bReportIndividualTestCases) {
	const size_t NR_POSITS = (size_t(1) << nbits);
	int nrOfFailedTestCases = 0;

	// use only valid posit values
	// posit_raw -> to value in Ty -> assign to posit -> compare posits
	sw::universal::posit<nbits, es> p, assigned;
	for (size_t i = 0; i < NR_POSITS; i++) {
		p.set_raw_bits(i); // std::cout << p.get() << endl;
		if (p.isnar() && std::numeric_limits<Ty>::is_exact) continue; // can't assign NaR for integer types
		Ty value = (Ty)(p);
		assigned = value;
		// TODO: how to make this work for integers: std::cout << p << " " << value << " " << assigned << std::endl;
		if (p != assigned) {
			nrOfFailedTestCases++;
			if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", p, assigned, value);
		}
		else {
			//if (bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", p, assigned, value);
		}
	}
	return nrOfFailedTestCases;
}

int main()
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;
	std::string tag = "Assignment";

	// 
	// TODO: How to make this work for integers
	// nrOfFailedTestCases = ReportTestResult(ValidateAssignment<3, 0, int>(bReportIndividualTestCases), tag, "posit<3,0> int");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<3, 0, float>(bReportIndividualTestCases), tag, "posit<3,0>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<4, 0, float>(bReportIndividualTestCases), tag, "posit<4,0>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<4, 1, float>(bReportIndividualTestCases), tag, "posit<4,1>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<5, 0, float>(bReportIndividualTestCases), tag, "posit<5,0>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<5, 1, float>(bReportIndividualTestCases), tag, "posit<5,1>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<5, 2, float>(bReportIndividualTestCases), tag, "posit<5,2>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<6, 0, float>(bReportIndividualTestCases), tag, "posit<6,0>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<6, 1, float>(bReportIndividualTestCases), tag, "posit<6,1>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<6, 2, float>(bReportIndividualTestCases), tag, "posit<6,2>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<6, 3, float>(bReportIndividualTestCases), tag, "posit<6,3>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<7, 0, float>(bReportIndividualTestCases), tag, "posit<7,0>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<7, 1, float>(bReportIndividualTestCases), tag, "posit<7,1>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<7, 2, float>(bReportIndividualTestCases), tag, "posit<7,2>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<7, 3, float>(bReportIndividualTestCases), tag, "posit<7,3>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<8, 0, float>(bReportIndividualTestCases), tag, "posit<8,0>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<8, 1, float>(bReportIndividualTestCases), tag, "posit<8,1>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<8, 2, float>(bReportIndividualTestCases), tag, "posit<8,2>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<8, 3, float>(bReportIndividualTestCases), tag, "posit<8,3>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<8, 4, float>(bReportIndividualTestCases), tag, "posit<8,4>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<9, 0, float>(bReportIndividualTestCases), tag, "posit<9,0>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<9, 1, float>(bReportIndividualTestCases), tag, "posit<9,1>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<9, 2, float>(bReportIndividualTestCases), tag, "posit<9,2>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<9, 3, float>(bReportIndividualTestCases), tag, "posit<9,3>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<9, 4, float>(bReportIndividualTestCases), tag, "posit<9,4>");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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



