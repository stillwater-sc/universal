// decode.cpp: functional tests of the posit decode method
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <sstream>
#include <vector>

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

/*
  Posit values are a combination of
  1) a scaling factor, useed, 
  2) an exponent, e, and 
  3) a fraction, f.
  For small posits, it is cleaner to have a lookup mechanism to obtain the value.
  This is valuable for conversion operators from posit to int.
*/

// TODO this is not generalized yet as the golden values change for each posit config: this is a <4,0> config
template<size_t nbits, size_t es>
int ValidateDecode() {
	const int NR_TEST_CASES = 16;
	float golden_values[NR_TEST_CASES] = {
		0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f, INFINITY, -4.0f, -2.0f, -1.5f, -1.0f, -0.75f, -0.5f, -0.25f, 
	};

	int nrOfFailedTestCases = 0;
	posit<4, 0> pa;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(uint64_t(i));
		if (fabs(pa.to_double() - golden_values[i]) > 0.0001) {
			ReportDecodeError("Posit<4,0> decode failed: ", pa, golden_values[i]);
			nrOfFailedTestCases++;
		}
	}
	return nrOfFailedTestCases;
}

int main(int argc, char** argv)
try
{
	const size_t nbits = 8;
	const size_t es = 1;
	posit<nbits, es> myPosit;
	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(ValidateDecode<4, 0>(), "b2p", "decode");

	return nrOfFailedTestCases;
}
catch (char* e) {
	cerr << e << endl;
	return -1;
}
