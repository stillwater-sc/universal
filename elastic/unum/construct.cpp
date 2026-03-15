// construct.cpp: functional tests to construct unum Type I values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/unum/unum.hpp>
#include <universal/number/unum/manipulators.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "unum Type I construction tests";
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, true);

	std::cout << "*** unum configurations\n";
	std::cout << unum_range<2, 2>() << '\n';
	std::cout << unum_range<2, 3>() << '\n';
	std::cout << unum_range<3, 3>() << '\n';
	std::cout << unum_range<3, 4>() << '\n';

	std::cout << "\n*** type tags\n";
	std::cout << type_tag(unum<2, 2>{}) << '\n';
	std::cout << type_tag(unum<3, 3>{}) << '\n';
	std::cout << type_tag(unum<3, 4>{}) << '\n';

	std::cout << "\n*** compile-time constants\n";
	std::cout << "unum<2,2> maxbits: " << unum<2, 2>::maxbits << '\n';
	std::cout << "unum<3,3> maxbits: " << unum<3, 3>::maxbits << '\n';
	std::cout << "unum<3,4> maxbits: " << unum<3, 4>::maxbits << '\n';
	std::cout << "unum<4,5> maxbits: " << unum<4, 5>::maxbits << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
