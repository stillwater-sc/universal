// smoke_add.cpp: generate smoke tests for addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <chrono>
#include <ctime>

#include <posit>
#include "../tests/posit_test_helpers.hpp"
#include "qa_helpers.hpp"

using namespace std;

// Generate smoke tests for different posit configurations
// Usage: qa_smoke_add 16/24/32/48/64
int main(int argc, char** argv)
try {
	typedef std::numeric_limits< double > dbl;
	cerr << "double max_digits10 " << dbl::max_digits10 << endl;

	int posit_size = 32;  // default
	if (argc == 2) {
		posit_size = std::stoi(argv[1]);
	}
	cerr << "Generating smoke tests for posit addition" << endl;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	float upper_limit = int64_t(1) << 17;
	using namespace std::chrono;
	steady_clock::time_point t1 = steady_clock::now();

	switch (posit_size) {
	case 16:
		nrOfFailedTestCases = sw::qa::SmokeTestAddition<16, 1>("smoke testing", bReportIndividualTestCases);
		break;
	case 24:
		nrOfFailedTestCases = sw::qa::SmokeTestAddition<24, 1>("smoke testing", bReportIndividualTestCases);
		break;
	case 32:
		nrOfFailedTestCases = sw::qa::SmokeTestAddition<32, 2>("smoke testing", bReportIndividualTestCases);
		break;
	case 48:
		nrOfFailedTestCases = sw::qa::SmokeTestAddition<48, 2>("smoke testing", bReportIndividualTestCases);
		break;
	case 64:
		nrOfFailedTestCases = sw::qa::SmokeTestAddition<64, 3>("smoke testing", bReportIndividualTestCases);
		break;
	default:
		nrOfFailedTestCases = 1;
	}
	

	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	double elapsed = time_span.count();
	std::cout << "It took " << elapsed << " seconds." << std::endl;
	std::cout << "Performance " << (uint32_t)(upper_limit / (1000000 * elapsed)) << " Msamples/s" << std::endl;
	std::cout << std::endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
