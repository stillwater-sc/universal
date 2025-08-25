// api.cpp : test suite runner for the class interface of the floatcascade type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/internal/floatcascade/floatcascade.hpp>
#include <universal/verification/test_suite.hpp>


int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "floatcascade API demonstration";
	std::string test_tag    = "api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// construction
	{
		floatcascade<3> fc1; // default constructor
		fc1.set(1.0);        // set to a double
		std::cout << "fc1: " << to_tuple(fc1) << '\n';

		floatcascade<3> fc2(2.0); // construct from double
		std::cout << "fc2: " << to_tuple(fc2) << '\n';
	}

	// component access
	{
		floatcascade<4> fc1;
		fc1[0] = 1.0;
		fc1[1] = 1.0e-16;
		fc1[2] = 1.0e-32;
		fc1[3] = 1.0e-48;
		std::cout << "fc1: " << to_tuple(fc1) << " ~ " << fc1.to_double() << '\n';
	}

	{
		std::array<double, 3> tuple { 1.0471975511965976, 1.994890429429456e-17, 1.1e-34 };
		floatcascade<3> fc1(tuple);
		std::string s = to_scientific(fc1, 3*17, false, false, true);
		// Output: "1.047197551196597631317786181170959025621414184570313e+0"
		std::cout << "fc1 : " << to_tuple(fc1) << " : " << to_scientific(fc1) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
