//  values.cpp : tests on values in scientific notation (sign, scale, fraction)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/trace_constants.hpp"
#include "../../posit/bit_functions.hpp"
#include "../../posit/value.hpp"

using namespace std;

void TestConversionResult(bool bValid, string descriptor)
{
	if (!bValid) {
		cout << descriptor << " conversions FAIL" << endl;
	}
	else {
		cout << descriptor << " conversions PASS" << endl;
	}
}

template<size_t fbits>
bool ValidateValue() {
	const int NR_TEST_CASES = 12;
	float input[NR_TEST_CASES] = {
		0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
	};
	float golden_scales[NR_TEST_CASES] = {
		0, 0, 1, 2, 3,  4,  5,  6,   7,   8,   9,   10
	};
	float golden_answer[NR_TEST_CASES] = {
		0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
	};

	bool bValid = true;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		value<fbits> v;
		v = input[i];
		if (fabs(v.to_double() - golden_answer[i]) > 0.00000001) {
			cerr << "FAIL [" << setw(2) << i << "] input " << input[i] << " ref = " << golden_answer[i] << " != " << setw(5) << v << endl;
			bValid = false;
		}
	}
	for (int i = 2; i < NR_TEST_CASES; i++) {
		value<fbits> v;
		v = 1.0 / input[i];
		if (fabs(v.to_double() - (1.0 / golden_answer[i])) > 0.00000001) {
			cerr << "FAIL [" << setw(2) << NR_TEST_CASES + i << "] input " << 1.0 / input[i] << " ref = " << 1.0 / golden_answer[i] << " != " << setw(5) << v << endl;
			bValid = false;
		}
	}
	return bValid;
}

int main()
try {
	const size_t nbits = 32;
	int nrOfFailedTestCases = 0;

	value<nbits> v1(-0.125f), v2(1.5f);
	cout << v1 << endl;
	cout << v2 << endl;

	int64_t n1, n2;
	n1 =  1234567890123456;
	n2 = -123456789012345;
	v1 = n1;
	v2 = n2;
	cout << setprecision(10) << v1 << endl;
	cout << v2 << endl;

	value<nbits> v3(n1), v4(n2);
	cout << v3 << endl << v4 << endl;

	cout << "Value configuration validation" << endl;
	TestConversionResult(ValidateValue<8>(), "value<8>");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
