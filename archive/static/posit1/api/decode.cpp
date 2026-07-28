// decode.cpp: test suite runner of the posit decode method
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_DECODE
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>

/*
  Posit values are a combination of
  1) a scaling factor, useed, 
  2) an exponent, e, and 
  3) a fraction, f.
  For small posits, it is cleaner to have a lookup mechanism to obtain the value.
  This is valuable for conversion operators from posit to int.
*/

template<unsigned nbits, unsigned es>
void ReportDecodeError(const std::string& test_case, const sw::universal::posit<nbits, es>& actual, double golden_value) {
		std::cerr << test_case << " actual " << actual << " required " << golden_value << std::endl;
}

// TODO this is not generalized yet as the golden values change for each posit config: this is a <4,0> config
template<size_t nbits, size_t es>
int ValidateDecode() {
	const int NR_TEST_CASES = 16;
	float golden_values[NR_TEST_CASES] = {
		0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f, INFINITY, -4.0f, -2.0f, -1.5f, -1.0f, -0.75f, -0.5f, -0.25f, 
	};

	int nrOfFailedTestCases = 0;
	sw::universal::posit<4, 0> pa;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pa.setbits(uint64_t(i));
		if (fabs(double(pa) - golden_values[i]) > 0.0001) {
			ReportDecodeError("Posit<4,0> decode failed: ", pa, golden_values[i]);
			nrOfFailedTestCases++;
		}
	}
	return nrOfFailedTestCases;
}

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(ValidateDecode<4, 0>(), "b2p", "decode");

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
