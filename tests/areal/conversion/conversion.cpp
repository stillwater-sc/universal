// mod_conversion.cpp: functional tests for areal conversions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the areal template environment
// first: enable general or specialized configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0

// minimum set of include files to reflect source code dependencies
#include <universal/areal/areal.hpp>
#include <universal/areal/manipulators.hpp>
#include <universal/areal/math_functions.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_suite_arithmetic.hpp>

// generate specific test case that you can trace with the trace conditions
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::areal<nbits, es> a, b, cref, result;
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


template<size_t nbits, size_t es>
void GenerateArealComparisonTable(std::string& tag) {
	using namespace std;
	using namespace sw::universal;
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	areal<nbits, es> a;
	areal<nbits+1, es+1> next;
	cout << "  areal<" << nbits + 1 << "," << es << ">      |    areal<" << nbits << ", " << es << ">" << endl;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.set_raw_bits(i);
		next.set_raw_bits(2*i);
		cout << to_binary(next) << ' ' << setw(10) << next << "  |  " << to_binary(a) << ' ' << setw(15) << a << endl;
		next.set_raw_bits(2 * i + 1);
		cout << to_binary(next) << ' ' << setw(10) << next << "  |  " << endl;
	}
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "conversion: ";

#if MANUAL_TESTING

	ReportRanges<12, 3>(cout);

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<areal<4, 1>, areal<5,1>>(tag, bReportIndividualTestCases), tag, "areal<4,1>");

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<areal<8, 2>, areal<9,2>>(tag, bReportIndividualTestCases), tag, "areal<8,2>");

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<areal<12, 3>, areal<13,3>>(tag, bReportIndividualTestCases), tag, "areal<12,3>");

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING

	cout << "Fixed-point conversion validation" << endl;


	nrOfFailedTestCases = ReportTestResult(VerifyConversion<areal<4, 1, uint8_t>, areal<5, 1, uint8_t>>(tag, bReportIndividualTestCases), tag, "areal<4,1,uint8_t>");

//	nrOfFailedTestCases = ReportTestResult(VerifyConversion< areal<8, 2, uint8_t>, areal<9, 2, uint8_t>>(tag, bReportIndividualTestCases), tag, "areal<8,2,uint8_t>");
//	nrOfFailedTestCases = ReportTestResult(VerifyConversion< areal<8, 3, uint8_t>, areal<9, 4, uint8_t>>(tag, bReportIndividualTestCases), tag, "areal<8,3,uint8_t>");

//	nrOfFailedTestCases = ReportTestResult(VerifyConversion< areal<12, 2, uint8_t>, areal<13, 2, uint8_t>>(tag, bReportIndividualTestCases), tag, "areal<12,2,uint8_t>");
//	nrOfFailedTestCases = ReportTestResult(VerifyConversion< areal<12, 3, uint8_t>, areal<13, 3, uint8_t>>(tag, bReportIndividualTestCases), tag, "areal<12,3,uint8_t>");
//	nrOfFailedTestCases = ReportTestResult(VerifyConversion< areal<12, 4, uint8_t>, areal<13, 4, uint8_t>>(tag, bReportIndividualTestCases), tag, "areal<12,4,uint8_t>");

//	nrOfFailedTestCases = ReportTestResult(VerifyConversion< areal<16, 3, uint8_t>, areal<17, 3, uint8_t>>(tag, bReportIndividualTestCases), tag, "areal<16,3,uint16_t>");
//	nrOfFailedTestCases = ReportTestResult(VerifyConversion< areal<16, 4, uint8_t>, areal<17, 4, uint8_t>>(tag, bReportIndividualTestCases), tag, "areal<16,4,uint16_t>");
//	nrOfFailedTestCases = ReportTestResult(VerifyConversion< areal<16, 8, uint8_t>, areal<17, 8, uint8_t>>(tag, bReportIndividualTestCases), tag, "areal<16,8,uint16_t>");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_arithmetic_exception& err) {
	std::cerr << "Uncaught areal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_internal_exception& err) {
	std::cerr << "Uncaught areal internal exception: " << err.what() << std::endl;
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


/*
  Value relationships between areal<nbits+1,es+1> and areal<nbits,es> we'll use for validation

  To generate:
  	GenerateFixedPointComparisonTable<4, 0>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 1>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 2>(std::string("-"));
	

 */
