// numbers_irrational.cpp: experiments with irrational numbers and their approximations
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "universal/posit/posit.hpp"
#include "universal/posit/posit_manipulators.hpp"
#include "../test_helpers.hpp"
#include "../posit_test_helpers.hpp"

/*
The most 'irrational' number of all is the golden ration, phi: phi = 1 + 1/phi
The second most is sqrt(2), which has a continued expansion of 1 + 1/(2 + 1/(2 + 1/(2 + ...)))
Pi is not that rational, like to find out what the 1 + 1/(3 + ... continued fraction yields
*/

/*
 we can generate the golden ratio by different means: 
 direct eval: phi = 1/2 + sqrt(5)/2
 continued fraction: pick x, calc 1/x, add 1, repeat
 evaluate the ration of the last two numbers of a Fibonacci sequence

 phi at 156 digits 
 1.61803398874989484820458683436563811772030917980576286213544862270526046281890244970720720418939113748475408807538689175212663386222353693179318006076672635
 */
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<4, 1>("Increment failed", bReportIndividualTestCases), "posit<4,1>", "prefix ++posit");

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
