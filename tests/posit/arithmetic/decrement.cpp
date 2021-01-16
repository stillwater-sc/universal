// decrement.cpp: test suite runner for decrement operator
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/posit/posit.hpp>
#include <universal/posit/manipulators.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING
	constexpr size_t nbits = 5;
	constexpr size_t es = 0;
	using Scalar = posit<nbits, es>;
	const std::string positConfig = "posit<5,0>";
	std::vector< Scalar > set;
	GenerateOrderedPositSet<nbits, es>(set);
	for_each (begin(set), end(set), [](const Scalar& s){
		std::cout << s.get() << " " << s << std::endl;
	});

	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<nbits, es>("Decrement failed", bReportIndividualTestCases), positConfig, "operator++");

#else
	// Note: increment/decrement depend on the 2's complement ordering of the posit encoding
	// This implies that this functionality is independent of the <nbits,es> configuration of the posit.
	// Otherwise stated, an enumeration of tests for different posit configurations is a bit superfluous.

	// DECREMENT tests
	cout << endl << "DECREMENT tests" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<3, 0>("Decrement failed", bReportIndividualTestCases), "posit<3,0>", "operator--");

	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<4, 0>("Decrement failed", bReportIndividualTestCases), "posit<4,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<4, 1>("Decrement failed", bReportIndividualTestCases), "posit<4,1>", "operator--");

	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<5, 0>("Decrement failed", bReportIndividualTestCases), "posit<5,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<5, 1>("Decrement failed", bReportIndividualTestCases), "posit<5,1>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<5, 2>("Decrement failed", bReportIndividualTestCases), "posit<5,2>", "operator--");

	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<6, 0>("Decrement failed", bReportIndividualTestCases), "posit<6,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<6, 1>("Decrement failed", bReportIndividualTestCases), "posit<6,1>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<6, 2>("Decrement failed", bReportIndividualTestCases), "posit<6,2>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<6, 3>("Decrement failed", bReportIndividualTestCases), "posit<6,3>", "operator--");

	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<7, 0>("Decrement failed", bReportIndividualTestCases), "posit<7,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<7, 1>("Decrement failed", bReportIndividualTestCases), "posit<7,1>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<7, 2>("Decrement failed", bReportIndividualTestCases), "posit<7,2>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<7, 3>("Decrement failed", bReportIndividualTestCases), "posit<7,3>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<7, 4>("Decrement failed", bReportIndividualTestCases), "posit<7,4>", "operator--");

	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<8, 0>("Decrement failed", bReportIndividualTestCases), "posit<8,0>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<8, 1>("Decrement failed", bReportIndividualTestCases), "posit<8,1>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<8, 2>("Decrement failed", bReportIndividualTestCases), "posit<8,2>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<8, 3>("Decrement failed", bReportIndividualTestCases), "posit<8,3>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<8, 4>("Decrement failed", bReportIndividualTestCases), "posit<8,4>", "operator--");
	nrOfFailedTestCases += ReportTestResult(VerifyDecrement<8, 5>("Decrement failed", bReportIndividualTestCases), "posit<8,5>", "operator--");

#endif // MANUAL_TESTING

	// long running
	if (argc == 2 && std::string(argv[1]) == std::string("-l")) {
		// AD/DA adapted data path configurations
		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<10, 0>("Decrement failed", bReportIndividualTestCases), "posit<10,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<10, 1>("Decrement failed", bReportIndividualTestCases), "posit<10,1>", "operator--");

		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<12, 0>("Decrement failed", bReportIndividualTestCases), "posit<12,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<12, 1>("Decrement failed", bReportIndividualTestCases), "posit<12,1>", "operator--");

		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<14, 0>("Decrement failed", bReportIndividualTestCases), "posit<14,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<14, 1>("Decrement failed", bReportIndividualTestCases), "posit<14,1>", "operator--");

		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<15, 0>("Decrement failed", bReportIndividualTestCases), "posit<15,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<15, 1>("Decrement failed", bReportIndividualTestCases), "posit<15,1>", "operator--");

		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<16, 0>("Decrement failed", bReportIndividualTestCases), "posit<16,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<16, 1>("Decrement failed", bReportIndividualTestCases), "posit<16,1>", "operator--");
		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<16, 2>("Decrement failed", bReportIndividualTestCases), "posit<16,2>", "operator--");

		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<18, 0>("Decrement failed", bReportIndividualTestCases), "posit<18,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<18, 1>("Decrement failed", bReportIndividualTestCases), "posit<18,1>", "operator--");
		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<18, 2>("Decrement failed", bReportIndividualTestCases), "posit<18,2>", "operator--");

		nrOfFailedTestCases += ReportTestResult(VerifyDecrement<20, 1>("Decrement failed", bReportIndividualTestCases), "posit<20,1>", "operator--");

		// legit float replacement
		//nrOfFailedTestCases += ReportTestResult(VerifyDecrement<24, 1>("Decrement failed", bReportIndividualTestCases), "posit<24,1>", "operator--");
		//nrOfFailedTestCases += ReportTestResult(VerifyDecrement<28, 1>("Decrement failed", bReportIndividualTestCases), "posit<28,2>", "operator--");

		// legit double replacement
		//nrOfFailedTestCases += ReportTestResult(VerifyDecrement<32, 2>("Decrement failed", bReportIndividualTestCases), "posit<32,2>", "operator--");
	}

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
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
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
