// rounding.cpp: functional tests for rounding (projecting) values to posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

template<size_t nbits>
void transform_into_sign_scale_fraction(int64_t value) {
	bool _sign = (0x8000000000000000 & value);
	unsigned int _scale = findMostSignificantBit(value) - 1;
	uint64_t _fraction_without_hidden_bit = (value << (64 - _scale));
	std::bitset<nbits - 3> _fraction = copy_integer_fraction<nbits>(_fraction_without_hidden_bit);
	cout << "Value    " << value << endl << "Binary   " << to_binary(value) << endl;
	cout << "Sign     " << _sign << endl << "Scale    " << _scale << endl << "Fraction " << _fraction << endl;
}

void TestPositRounding(bool bValid, string posit_cfg, string op)
{
	if (!bValid) {
		cout << posit_cfg << " " << op << " FAIL" << endl;
	}
	else {
		cout << posit_cfg << " " << op << " PASS" << endl;
	}
}

/*
POSIT<4,0>
 #           Binary         k-value            sign          regime        exponent        fraction           value
 0:             0000              -3               1           0.125               -               0               0
 1:             0001              -2               1            0.25               -               0            0.25
 2:             0010              -1               1             0.5               -               0             0.5
 3:             0011              -1               1             0.5               -               1            0.75
 4:             0100               0               1               1               -               0               1
 5:             0101               0               1               1               -               1             1.5
 6:             0110               1               1               2               -               0               2
 7:             0111               2               1               4               -               0               4
 8:             1000               3              -1               8               -               0             inf
 9:             1001               2              -1               4               -               0              -4
10:             1010               1              -1               2               -               0              -2
11:             1011               0              -1               1               -               1            -1.5
12:             1100               0              -1               1               -               0              -1
13:             1101              -1              -1             0.5               -               1           -0.75
14:             1110              -1              -1             0.5               -               0            -0.5
15:             1111              -2              -1            0.25               -               0           -0.25
*/
bool ValidateFloatRoundingPosit_4_0()
{
	float input[9] = {
		0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f, 100.0f
	};
	float golden_answer[8] = {
		0.25f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f
	};

	bool bValid = true;
	posit<4, 0> p;
	float arithmetic_mean;
	for (int i = 0; i < 8; i++) {
		arithmetic_mean = float((input[i] + input[i + 1] - 0.0001) / 2.0f);
		//cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() - golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,0> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	for (int i = 0; i < 8; i++) {
		arithmetic_mean = float((-input[i] - input[i + 1] + 0.0001) / 2.0f);
		//cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() + golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,0> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	return bValid;
}
bool ValidateDoubleRoundingPosit_4_0()
{
	float input[9] = {
		0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f, 100.0f
	};
	float golden_answer[8] = {
		0.25f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f
	};

	bool bValid = true;
	posit<4, 0> p;
	double arithmetic_mean;
	for (int i = 0; i < 8; i++) {
		arithmetic_mean = (input[i] + input[i + 1] - 0.000005) / 2.0f;
		//cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() - golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,0> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	for (int i = 0; i < 8; i++) {
		arithmetic_mean = (-input[i] - input[i + 1] + 0.000005) / 2.0f;
		//cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() + golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,0> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	return bValid;
}

/*
POSIT<4,1>
 #           Binary         k-value            sign          regime        exponent        fraction           value
 0:             0000              -3               1        0.015625               0               -               0
 1:             0001              -2               1          0.0625               0               -          0.0625
 2:             0010              -1               1            0.25               0               -            0.25
 3:             0011              -1               1            0.25               1               -             0.5
 4:             0100               0               1               1               0               -               1
 5:             0101               0               1               1               1               -               2
 6:             0110               1               1               4               0               -               4
 7:             0111               2               1              16               0               -              16
 8:             1000               3              -1              64               0               -             inf
 9:             1001               2              -1              16               0               -             -16
10:             1010               1              -1               4               0               -              -4
11:             1011               0              -1               1               1               -              -2
12:             1100               0              -1               1               0               -              -1
13:             1101              -1              -1            0.25               1               -            -0.5
14:             1110              -1              -1            0.25               0               -           -0.25
15:             1111              -2              -1          0.0625               0               -         -0.0625
*/
bool ValidateFloatRoundingPosit_4_1()
{
	float input[9] = {
		0.0, 0.0625, 0.25, 0.5, 1.0, 2.0, 4.0, 16.0, 100.0f
	};
	float golden_answer[8] = {
		0.0625f, 0.0625f, 0.25, 0.5, 1.0, 2.0, 4.0, 16.0
	};

	bool bValid = true;
	posit<4, 1> p;
	float arithmetic_mean;
	for (int i = 0; i < 8; i++) {
		arithmetic_mean = float((input[i] + input[i + 1] - 0.0001) / 2.0f);
		//cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() - golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,1> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	for (int i = 0; i < 8; i++) {
		arithmetic_mean = float((-input[i] - input[i + 1] + 0.0001) / 2.0f);
		//cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() + golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,1> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	return bValid;
}
bool ValidateDoubleRoundingDownPosit_4_1()
{
	float input[9] = {
		0.0, 0.0625, 0.25, 0.5, 1.0, 2.0, 4.0, 16.0, 100.0f
	};
	float golden_answer[8] = {
		0.0625f, 0.0625, 0.25, 0.5, 1.0, 2.0, 4.0, 16.0
	};

	bool bValid = true;
	posit<4, 1> p;
	double arithmetic_mean;
	for (int i = 0; i < 8; i++) {
		arithmetic_mean = double((input[i] + input[i + 1] - 0.0001) / 2.0f);
		//cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() - golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,1> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	for (int i = 0; i < 8; i++) {
		arithmetic_mean = double((-input[i] - input[i + 1] + 0.0001) / 2.0f);
		//cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() + golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,1> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	return bValid;
}
bool ValidateDoubleRoundingUpPosit_4_1()
{
	float input[9] = {
		0.0, 0.0625, 0.25, 0.5, 1.0, 2.0, 4.0, 16.0, 100.0f
	};
	float golden_answer[8] = {
		0.0625f, 0.25, 0.5, 1.0, 2.0, 4.0, 16.0f, 16.0f
	};

	bool bValid = true;
	posit<4, 1> p;
	double arithmetic_mean;
	for (int i = 0; i < 8; i++) {
		arithmetic_mean = double((input[i] + input[i + 1] + 0.000005) / 2.0f);
		cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() - golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,1> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	for (int i = 0; i < 8; i++) {
		arithmetic_mean = double((-input[i] - input[i + 1] - 0.000005) / 2.0f);
		cout << setw(3) << i << " : arithmetic mean = " << arithmetic_mean << endl;
		p = arithmetic_mean;
		if (fabs(p.to_double() + golden_answer[i]) > 0.0001) {
			cerr << "Posit rounding failed: golden value = " << golden_answer[i] << " != posit<4,1> " << components_to_string(p) << endl;
			bValid = false;
		}
	}

	return bValid;
}

int main(int argc, char** argv)
try {
	int nrOfFailedTestCases = 0;

	//ReportPositScales();

		TestPositRounding(ValidateFloatRoundingPosit_4_0(), "posit<4,0>", "float rounding");
		TestPositRounding(ValidateDoubleRoundingPosit_4_0(), "posit<4,0>", "double rounding");
		TestPositRounding(ValidateFloatRoundingPosit_4_1(), "posit<4,1>", "float rounding");
		TestPositRounding(ValidateDoubleRoundingDownPosit_4_1(), "posit<4,1>", "double rounding down");
		TestPositRounding(ValidateDoubleRoundingUpPosit_4_1(), "posit<4,1>", "double rounding up");


	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
