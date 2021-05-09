// arithmetic_literals.cpp: test suite runner for the use of literals in posit equations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable general or specialized specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define POSIT_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_ADD
// forth: enable/disable the ability to use literals in binary logic and arithmetic operators
#define POSIT_ENABLE_LITERALS 1

// minimum set of include files to reflect source code dependencies
#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/numeric_limits.hpp>
#include <universal/number/posit/specializations.hpp>
#include <universal/number/posit/math_functions.hpp>
#include <universal/number/posit/manipulators.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
//#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>
#include <universal/verification/posit_math_test_suite.hpp>


// enumerate all addition cases for a posit configuration: is within 10sec till about nbits = 14
template<size_t nbits, size_t es>
int ValidateAdditionWithLiteral(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	sw::universal::posit<nbits, es> pa, pb, psum1, psum2, pref;

	double da, db;
	for (int i = 0; i < NR_POSITS; i++) {
		pa.setBits(i);
		da = double(pa);
		for (int j = 0; j < NR_POSITS; j++) {
			pb.setBits(j);
			db = double(pb);
			psum1 = pa + db;
			psum2 = da + pb;
			pref = da + db;
			if (psum1 != pref || psum2 != pref || psum1 != psum2) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", pa, pb, pref, psum1);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, pref, psum1);
			}
		}
	}

	return nrOfFailedTests;
}

// enumerate all subtraction cases for a posit configuration
template<size_t nbits, size_t es>
int ValidateSubtractionWithLiteral(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	sw::universal::posit<nbits, es> pa, pb, pdiff1, pdiff2, pref;

	double da, db;
	for (int i = 0; i < NR_POSITS; i++) {
		pa.setBits(i);
		da = double(pa);
		for (int j = 0; j < NR_POSITS; j++) {
			pb.setBits(j);
			db = double(pb);
			pdiff1 = pa - db;
			pdiff2 = da - pb;
			pref = da - db;
			if (pdiff1 != pref || pdiff2 != pref || pdiff1 != pdiff2) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", pa, pb, pref, pdiff1);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", pa, pb, pref, pdiff1);
			}
		}
	}

	return nrOfFailedTests;
}

// enumerate all multiplication cases for a posit configuration
template<size_t nbits, size_t es>
int ValidateMultiplicationWithLiteral(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	sw::universal::posit<nbits, es> pa, pb, pmul1, pmul2, pref;

	double da, db;
	for (int i = 0; i < NR_POSITS; i++) {
		pa.setBits(i);
		da = double(pa);
		for (int j = 0; j < NR_POSITS; j++) {
			pb.setBits(j);
			db = double(pb);
			pmul1 = pa * db;
			pmul2 = da * pb;
			pref = da * db;
			if (pmul1 != pref || pmul2 != pref || pmul1 != pmul2) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", pa, pb, pref, pmul1);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", pa, pb, pref, pmul1);
			}
		}
	}

	return nrOfFailedTests;
}

// enumerate all division cases for a posit configuration
template<size_t nbits, size_t es>
int ValidateDivisionWithLiteral(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	sw::universal::posit<nbits, es> pa, pb, pdiv1, pdiv2, pref;

	double da, db;
	for (int i = 0; i < NR_POSITS; i++) {
		pa.setBits(i);
		da = double(pa);
		for (int j = 0; j < NR_POSITS; j++) {
			pb.setBits(j);
			db = double(pb);
			pdiv1 = pa / db;
			pdiv2 = da / pb;
			pref = da / db;
			if (pdiv1 != pref || pdiv2 != pref || pdiv1 != pdiv2) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", pa, pb, pref, pdiv1);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, pref, pdiv1);
			}
		}
	}

	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pref, psum;
	pa = a;
	pb = b;
	ref = a + b;
	pref = ref;
	psum = pa + pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " + " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " + " << pb.get() << " = " << psum.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Arithmetic with literals failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<6, 3, double>(INFINITY, INFINITY);
	GenerateTestCase<8, 4, float>(0.5f, -0.5f);
	GenerateTestCase<3, 0>(0.5f, 1.0f);

	constexpr double m_pi = 3.14159265358979323846;

	posit<16, 1> p;
	p += m_pi;
	cout << p << endl;
	p -= m_pi;
	cout << p << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateAdditionWithLiteral<8, 2>("Manual Testing", true), "posit<8,2>", "addition with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtractionWithLiteral<8, 2>("Manual Testing", true), "posit<8,2>", "subtraction with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplicationWithLiteral<8, 2>("Manual Testing", true), "posit<8,2>", "multiplication with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateDivisionWithLiteral<8, 2>("Manual Testing", true), "posit<8,2>", "division with literal");

#else

	cout << "Posit addition validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateAdditionWithLiteral<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "addition with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateAdditionWithLiteral<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "addition with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateAdditionWithLiteral<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "addition with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateAdditionWithLiteral<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "addition with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateAdditionWithLiteral<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "addition with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateAdditionWithLiteral<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "addition with literal");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtractionWithLiteral<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "subtraction with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtractionWithLiteral<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "subtraction with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtractionWithLiteral<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "subtraction with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtractionWithLiteral<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "subtraction with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtractionWithLiteral<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "subtraction with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtractionWithLiteral<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "subtraction with literal");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplicationWithLiteral<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "multiplication with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplicationWithLiteral<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "multiplication with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplicationWithLiteral<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "multiplication with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplicationWithLiteral<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "multiplication with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplicationWithLiteral<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "multiplication with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplicationWithLiteral<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "multiplication with literal");

	nrOfFailedTestCases += ReportTestResult(ValidateDivisionWithLiteral<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "division with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateDivisionWithLiteral<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "division with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateDivisionWithLiteral<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "division with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateDivisionWithLiteral<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "division with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateDivisionWithLiteral<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "division with literal");
	nrOfFailedTestCases += ReportTestResult(ValidateDivisionWithLiteral<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "division with literal");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

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
