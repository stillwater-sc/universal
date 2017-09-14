// Round-to-nearest tests for posits

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

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

void ReportPositScales() {
	// print scales of different posit configurations
	// useed = 2^(2^es) and thus is just a function of the exponent configuration
	// maxpos = useed^(nbits-2)
	// minpos = useed^(2-nbits)
	posit<3, 0> p3_0;
	posit<4, 0> p4_0;
	posit<4, 1> p4_1;
	posit<5, 0> p5_0;
	posit<5, 1> p5_1;
	posit<5, 2> p5_2;
	posit<6, 0> p6_0;
	posit<6, 1> p6_1;
	posit<6, 2> p6_2;
	posit<6, 3> p6_3;
	posit<7, 0> p7_0;
	posit<7, 1> p7_1;
	posit<7, 2> p7_2;
	posit<7, 3> p7_3;
	posit<8, 0> p8_0;
	posit<8, 1> p8_1;
	posit<8, 2> p8_2;
	posit<8, 3> p8_3;
	posit<9, 0> p9_0;
	posit<9, 1> p9_1;
	posit<9, 2> p9_2;
	posit<9, 3> p9_3;
	posit<10, 0> p10_0;
	posit<10, 1> p10_1;
	posit<10, 2> p10_2;
	posit<10, 3> p10_3;
	cout << "Posit specificiation examples and their ranges:" << endl;
	cout << spec_to_string(p3_0) << endl;
	cout << spec_to_string(p4_0) << endl;
	cout << spec_to_string(p4_1) << endl;
	cout << spec_to_string(p5_0) << endl;
	cout << spec_to_string(p5_1) << endl;
	cout << spec_to_string(p5_2) << endl;
	cout << spec_to_string(p6_0) << endl;
	cout << spec_to_string(p6_1) << endl;
	cout << spec_to_string(p6_2) << endl;
	cout << spec_to_string(p6_3) << endl;
	cout << spec_to_string(p7_0) << endl;
	cout << spec_to_string(p7_1) << endl;
	cout << spec_to_string(p7_2) << endl;
	cout << spec_to_string(p7_3) << endl;
	cout << spec_to_string(p8_0) << endl;
	cout << spec_to_string(p8_1) << endl;
	cout << spec_to_string(p8_2) << endl;
	cout << spec_to_string(p8_3) << endl;
	cout << spec_to_string(p9_0) << endl;
	cout << spec_to_string(p9_1) << endl;
	cout << spec_to_string(p9_2) << endl;
	cout << spec_to_string(p9_3) << endl;
	cout << spec_to_string(p10_0) << endl;
	cout << spec_to_string(p10_1) << endl;
	cout << spec_to_string(p10_2) << endl;
	cout << spec_to_string(p10_3) << endl;


	cout << endl;
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
{
	//ReportPositScales();

	try {

		posit<4, 1> p;
		double v = 0.156252f;
		cout << v << " " << hexfloat << v << defaultfloat << endl;
		p = v;
		cout << p << endl;

		/*
		TestPositRounding(ValidateFloatRoundingPosit_4_0(), "posit<4,0>", "float rounding");
		TestPositRounding(ValidateDoubleRoundingPosit_4_0(), "posit<4,0>", "double rounding");
		TestPositRounding(ValidateFloatRoundingPosit_4_1(), "posit<4,1>", "float rounding");
		TestPositRounding(ValidateDoubleRoundingDownPosit_4_1(), "posit<4,1>", "double rounding down");
		TestPositRounding(ValidateDoubleRoundingUpPosit_4_1(), "posit<4,1>", "double rounding up");
		*/
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
