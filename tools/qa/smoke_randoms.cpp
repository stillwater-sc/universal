// smoke_sub.cpp: generate smoke tests for subtraction
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

template<size_t nbits, size_t es>
int GenerateSmokeTests(bool bReportIndividualTestCases) {
	int nrOfFailedTestCases = 0;
	nrOfFailedTestCases = sw::qa::SmokeTestRandoms<nbits, es>("random smoke testing", sw::qa::OPCODE_ADD, 100);
	nrOfFailedTestCases = sw::qa::SmokeTestRandoms<nbits, es>("random smoke testing", sw::qa::OPCODE_SUB, 100);
	nrOfFailedTestCases = sw::qa::SmokeTestRandoms<nbits, es>("random smoke testing", sw::qa::OPCODE_MUL, 100);
	nrOfFailedTestCases = sw::qa::SmokeTestRandoms<nbits, es>("random smoke testing", sw::qa::OPCODE_DIV, 100);
	return nrOfFailedTestCases;
}

// Generate smoke tests for different posit configurations
// Usage: qa_smoke_randoms 16/24/32/48/64
int main(int argc, char** argv)
try {
	typedef std::numeric_limits< double > dbl;
	cerr << "double max digits " << dbl::max_digits10 << endl;

	int posit_size = 32;  // default
	if (argc == 2) {
		posit_size = std::stoi(argv[1]);
	}
	cerr << "Generating random smoke tests for posits of size " << posit_size << endl;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	float upper_limit = int64_t(1) << 17;
	using namespace std::chrono;
	steady_clock::time_point t1 = steady_clock::now();

	switch (posit_size) {
	case 16:
		nrOfFailedTestCases = GenerateSmokeTests<16, 1>(bReportIndividualTestCases);
		break;
	case 24:
		nrOfFailedTestCases = GenerateSmokeTests<24, 1>(bReportIndividualTestCases);
		break;
	case 32:
		nrOfFailedTestCases = GenerateSmokeTests<32, 2>(bReportIndividualTestCases);
		break;
	case 48:
		nrOfFailedTestCases = GenerateSmokeTests<48, 2>(bReportIndividualTestCases);
		break;
	case 64:
		nrOfFailedTestCases = GenerateSmokeTests<64, 3>(bReportIndividualTestCases);
		break;
	default:
		nrOfFailedTestCases = 1;
	}


	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	double elapsed = time_span.count();
	std::cout << "It took " << elapsed << " seconds." << std::endl;
	std::cout << "Performance " << (uint32_t)(upper_limit / (1000 * elapsed)) << " Ksamples/s" << std::endl;
	std::cout << std::endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
