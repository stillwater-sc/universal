//  values.cpp : tests on values in scientific notation (sign, scale, fraction)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <posit>

using namespace std;
using namespace sw::unum;

void TestConversionResult(bool bValid, string descriptor)
{
	if (!bValid) {
		std::cout << descriptor << " conversions FAIL" << std::endl;
	}
	else {
		std::cout << descriptor << " conversions PASS" << std::endl;
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
		if (fabs(double(v) - golden_answer[i]) > 0.00000001) {
			cerr << "FAIL [" << setw(2) << i << "] input " << input[i] << " ref = " << golden_answer[i] << " != " << setw(5) << v << endl;
			bValid = false;
		}
	}
	for (int i = 2; i < NR_TEST_CASES; i++) {
		value<fbits> v;
		v = 1.0 / input[i];
		if (fabs(double(v) - (1.0 / golden_answer[i])) > 0.00000001) {
			cerr << "FAIL [" << setw(2) << NR_TEST_CASES + i << "] input " << 1.0 / input[i] << " ref = " << 1.0 / golden_answer[i] << " != " << setw(5) << v << endl;
			bValid = false;
		}
	}
	return bValid;
}

template<size_t fbits>
void PrintValue(float f, const value<fbits>& v) {
	cout << "float: " << setw(fbits) << f << components(v) << endl;
}

int main()
try {
	const size_t nbits = 32;
	int nrOfFailedTestCases = 0;

	value<nbits> v1(-0.125f), v2(1.5f);
	cout << v1 << endl;
	cout << v2 << endl;

	long long n1, n2;
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

	cout << "Conversion values of importance" << endl;
	/*
	no exp left : geo-dw d          0.125  result          0.0625  scale = -4  k = -2  exp = -  0001 00010          0.0625     PASS
	no rounding alltaken u          0.125  result             0.5  scale = -1  k = -1  exp = 1  0011 00100            0.25 FAIL
	no rounding alltaken u           0.25  result               1  scale =  0  k = -1  exp = 0  0100 00100            0.25 FAIL
	no rounding alltaken d           0.25  result            0.25  scale = -2  k = -1  exp = 0  0010 00100            0.25     PASS
	no rounding alltaken u          -0.25  result           -0.25  scale=  -2  k=  -1  exp=   0  1110 11100           -0.25     PASS
	no rounding alltaken d          -0.25  result              -1  scale=   0  k=  -1  exp=   0  1100 11100           -0.25 FAIL
	no rounding alltaken d         -0.125  result            -0.5  scale=  -1  k=  -1  exp=   1  1101 11100           -0.25 FAIL
	no exp left:  geo-dw u         -0.125  result         -0.0625  scale=  -4  k=  -2  exp=   -  1111 11110         -0.0625     PASS
	*/
	float f;
	value<23> v;
	f =  0.12499f; v = f; PrintValue(f, v);
	f =  0.12500f; v = f; PrintValue(f, v);
	f =  0.12501f; v = f; PrintValue(f, v);
	f =  0.24999f; v = f; PrintValue(f, v);
	f =  0.25000f; v = f; PrintValue(f, v);
	f =  0.25001f; v = f; PrintValue(f, v);
	f = -0.25001f; v = f; PrintValue(f, v);
	f = -0.25000f; v = f; PrintValue(f, v);
	f = -0.24999f; v = f; PrintValue(f, v);
	f = -0.12501f; v = f; PrintValue(f, v);
	f = -0.12500f; v = f; PrintValue(f, v);
	f = -0.12499f; v = f; PrintValue(f, v);

	cout << "Rounding" << endl;
	bitblock<8> fraction;
	fraction = 0x55;
	value<8> r8(false, 0, fraction, false, false);
	cout << "Value is " << r8 << " components are " << components(r8) << endl;
	value<7> r7 = r8.round_to<7>();
	cout << "Value is " << r7 << " components are " << components(r7) << endl;
	value<6> r6 = r8.round_to<6>();
	cout << "Value is " << r6 << " components are " << components(r6) << endl;
	value<5> r5 = r8.round_to<5>();
	cout << "Value is " << r5 << " components are " << components(r5) << endl;
	value<4> r4 = r8.round_to<4>();
	cout << "Value is " << r4 << " components are " << components(r4) << endl;
	value<3> r3 = r8.round_to<3>();
	cout << "Value is " << r3 << " components are " << components(r3) << endl;
	value<2> r2 = r8.round_to<2>();
	cout << "Value is " << r2 << " components are " << components(r2) << endl;
	value<1> r1 = r8.round_to<1>();
	cout << "Value is " << r1 << " components are " << components(r1) << endl;
	value<0> r0 = r8.round_to<0>();
	cout << "Value is " << r0 << " components are " << components(r0) << endl;

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
