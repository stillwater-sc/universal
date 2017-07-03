// positive_regime.cpp : conversion operators for positive regime of posit numbers
//

#include "stdafx.h"
#include <sstream>

#include "../../posit/posit_scale_factors.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

template<size_t nbits, size_t es>
void checkSpecialCases(posit<nbits, es> p) {
	cout << "posit is " << (p.isZero() ? "zero " : "non-zero ") << (p.isPositive() ? "positive " : "negative ") << (p.isInfinite() ? "+-infinite" : "not infinite") << endl;
}

void ConversionOperatorsPositiveRegime() {
	posit<5, 1> p0, p1, p2, p3, p4, p5, p6;
	double minpos, maxpos;

	p0.Range(&minpos, &maxpos);
	cout << "Minpos = " << setprecision(7) << minpos << endl;
	cout << "Maxpos = " << maxpos << setprecision(0) << endl;

	int64_t number = 1;
	for (int i = 0; i < 8; i++) {
		p0.from_longlong(number);
		cout << "number = " << number << p0 << endl;
		number <<= 1;
	}

	return;

	p0 = int(0);  checkSpecialCases(p0);
	p1 = char(1);  cout << "P1 " << p1 << endl;
	p2 = long(2);  cout << "P2 " << p2 << endl;
	p3 = 4;  cout << "P3 " << p3 << endl;
	p4 = 8;  cout << "P4 " << p4 << endl;
	p5 = 16;  cout << "P5 " << p5 << endl;
	p6 = (long long)(32);  cout << "P6 " << p6 << endl;
}

void ConversionOperatorsNegativeRegime() {
	posit<16, 1> p0, p1, p2, p3, p4, p5, p6;

	p0.Range();

	p0 = 0;  checkSpecialCases(p0);
	p1 = -1;  checkSpecialCases(p1);
	p2 = -2;  checkSpecialCases(p2);
	p3 = -4;  checkSpecialCases(p3);
	p4 = -8;  checkSpecialCases(p4);
	p5 = -16;  checkSpecialCases(p5);
	p6 = -32;  checkSpecialCases(p6);
}

int main()
{
	ConversionOperatorsPositiveRegime();
    
	return 0;
}



