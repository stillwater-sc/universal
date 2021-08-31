// logarithm.cpp: test suite runner for the logarithm functions (log2, log10, ln)
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default library configuration
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/verification/fixpnt_math_test_suite.hpp>

// generate specific test case 
template<size_t nbits, size_t rbits, bool arithmetic, typename bt, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::fixpnt<nbits, rbits, arithmetic, bt> pa, pref, plog;
	pa = a;
	ref = std::log(a);
	pref = ref;
	plog = sw::universal::log(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> log(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(pa) << " -> log( " << pa << ") = " << sw::universal::to_binary(plog) << " (reference: " << sw::universal::to_binary(pref) << ")   " ;
	std::cout << (pref == plog ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0
#define GENERATE_LOG_TABLES 0

int main()
try {
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "fixpnt log() function failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, Saturating, uint8_t, float>(4.0f);

#if GENERATE_LOG_TABLES
	GenerateLogarithmTable<3, 0>();
	GenerateLogarithmTable<4, 0>();
	GenerateLogarithmTable<4, 1>();
	GenerateLogarithmTable<5, 0>();
	GenerateLogarithmTable<5, 1>();
	GenerateLogarithmTable<5, 2>();
	GenerateLogarithmTable<6, 0>();
	GenerateLogarithmTable<6, 1>();
	GenerateLogarithmTable<6, 2>();
	GenerateLogarithmTable<6, 3>();
	GenerateLogarithmTable<7, 0>();
#endif

	// manual exhaustive test
	using FixedPoint = fixpnt<10, 5, Saturating, uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(bReportIndividualTestCases), type_tag<FixedPoint>(), "log");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2< FixedPoint >(bReportIndividualTestCases), type_tag<FixedPoint>(), "log2");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10< FixedPoint >(bReportIndividualTestCases), type_tag<FixedPoint>(), "log10");

#else

	std::cout << "fixpnt log() function validation\n";


#if STRESS_TESTING
	// nbits=64 requires long double compiler support

	{
		using FixedPoint = fixpnt<10, 5, Saturating, uint8_t>;
		nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(bReportIndividualTestCases), type_tag(FixedPoint()), "log");
		nrOfFailedTestCases += ReportTestResult(VerifyLog2< FixedPoint >(bReportIndividualTestCases), type_tag(FixedPoint()), "log2");
		nrOfFailedTestCases += ReportTestResult(VerifyLog10< FixedPoint >(bReportIndividualTestCases), type_tag(FixedPoint()), "log10");
	}
	{
		using FixedPoint = fixpnt<12, 6, Saturating, uint8_t>;
		nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(bReportIndividualTestCases), type_tag(FixedPoint), "log");
	}
	{
		using FixedPoint = fixpnt<14, 7, Saturating, uint8_t>;
		nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(bReportIndividualTestCases), type_tag(FixedPoint), "log");
	}
	{
		using FixedPoint = fixpnt<16, 8, Saturating, uint8_t>;
		nrOfFailedTestCases += ReportTestResult(VerifyLog< FixedPoint >(bReportIndividualTestCases), type_tag(FixedPoint), "log");
	}
	
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
