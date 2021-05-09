// function_truncate.cpp: test suite runner for truncation functions trunc, round, floor, and ceil
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#define POSIT_ENABLE_LITERALS 1
#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/manipulators.hpp>
#include <universal/number/posit/math/truncate.hpp>
#include <universal/number/quire/exceptions.hpp>  // math library might use quire
#include <universal/verification/posit_math_test_suite.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

template<size_t nbits, size_t es>
int VerifyFloor(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	constexpr size_t NR_VALUES = (1 << nbits);
	int nrOfFailedTestCases = 0;

	posit<nbits, es> p;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		p.setBits(i);
		long l1 = long(sw::universal::floor(p));
		// generate the reference
		float f = float(p);
		long l2 = long(std::floor(f));
		if (l1 != l2) {
			++nrOfFailedTestCases;
			if (bReportIndividualTestCases) 
				ReportOneInputFunctionError("floor", "floor",
					p, posit<nbits, es>(l1), posit<nbits, es>(l2));
		}
	}
	return nrOfFailedTestCases;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "truncation failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases = ReportTestResult(VerifyFloor<6, 0>(bReportIndividualTestCases), "floor", "floor<4,0>()");

	nrOfFailedTestCases = 0; // nullify accumulated test failures in manual testing

#else

	cout << "Posit truncation function validation" << endl;


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
