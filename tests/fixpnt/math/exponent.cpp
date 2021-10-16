// exponent.cpp: test suite runner for exponent (exp, exp2, exp10) functions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// use default number system library configuration
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_math_test_suite.hpp>

// Background: http://numbers.computation.free.fr/Constants/E/e.html
//
// generate digits of Euler's number
void GenerateEulersNumber() {
	int N = 9009, a[9009], x = 0;
	for (int n = N - 1; n > 0; --n) {
		a[n] = 1;
	}
	a[1] = 2;
	while (N > 9) {
		int n = N--;
		while (--n) {
			a[n] = x % n;
			x = 10 * a[n - 1] + x / n;
		}
		std::cout << x;
	}
	std::cout << std::endl;
}

// generate specific test case
template<size_t nbits, size_t rbits, bool arithmetic, typename bt, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::fixpnt<nbits, rbits, arithmetic, bt> pa, pref, pexp;
	pa = a;
	ref = std::exp(a);
	pref = ref;
	pexp = sw::universal::exp(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> exp(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << to_binary(pa) << " -> exp( " << pa << ") = " << to_binary(pexp) << " (reference: " << to_binary(pref) << ")   ";
	std::cout << (pref == pexp ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0
#define GENERATE_EXPONENT_TABLES 0

int main()
try {
	using namespace sw::universal;

//	GenerateEulersNumber();  // 9000 digits of e

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "fixpnt exp() failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, Saturating, uint8_t, float>(4.0f);

#if GENERATE_EXPONENT_TABLES

	GenerateExponentTable<5, 1>();
	GenerateExponentTable<5, 2>();
	GenerateExponentTable<6, 1>();
	GenerateExponentTable<6, 2>();
	GenerateExponentTable<6, 3>();
#endif

	// manual exhaustive test
	using FixedPoint = fixpnt<8, 2, Saturating, uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyExp<FixedPoint>(bReportIndividualTestCases), type_tag(FixedPoint), "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<FixedPoint>(bReportIndividualTestCases), type_tag(FixedPoint) "exp2");

#else

	std::cout << "fixpnt exponential function validation\n";

	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt< 8, 2, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<8,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt< 8, 3, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<8,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt< 9, 2, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<9,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt<10, 2, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<10,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt<10, 3, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<10,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt<12, 4, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<12,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt<16, 5, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<16,5>", "exp");

	// base-2 exponent testing
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<8, 2, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<8,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<8, 3, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<8,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<9, 2, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<9,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<10, 2, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<10,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<10, 3, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<10,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<12, 4, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<12,4>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<16, 5, Saturating, uint8_t> >(bReportIndividualTestCases), "fixpnt<16,5>", "exp2");


#if STRESS_TESTING
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
//catch (const sw::universal::fixpnt_quire_exception& err) {
//	std::cerr << "Uncaught fixpnt quire exception: " << err.what() << std::endl;
//	return EXIT_FAILURE;
//}
catch (const sw::universal::fixpnt_internal_exception& err) {
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
