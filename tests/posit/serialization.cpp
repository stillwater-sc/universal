// serialization.cpp: functional tests for serialization functions of posits
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#define POSIT_ENABLE_LITERALS 1
#include "universal/posit/posit.hpp"
#include "universal/posit/posit_manipulators.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/posit_math_helpers.hpp"

#define MANUAL_TESTING 1
#define STRESS_TESTING 0


template<size_t nbits, size_t es>
void VerifyToBinary() {
	using namespace sw::unum;
	constexpr size_t NR_VALUES = (1 << nbits);

	posit<nbits, es> p;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		p.set_raw_bits(i);
		std::cout << hex_format(p) << ' ' << color_print(p) << ' ' << to_binary(p) << ' ' << p << std::endl;
	}
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "serialization failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	bool csv = false;
	GeneratePositTable<4, 0>(cout, csv);
	VerifyToBinary<4, 0>();

	nrOfFailedTestCases = 0; // nullify accumulated test failures in manual testing

#else

	cout << "Posit serialization validation" << endl;


#if STRESS_TESTING
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

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
