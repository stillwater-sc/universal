// positive_regime.cpp : conversion operators for positive regime of posit numbers
//

#include "stdafx.h"
#include <sstream>

using namespace std;

#include "../../posit/posit_scale_factors.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"



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
		cout << p0 << endl;
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
	posit<5, 1> p0, p1, p2, p3, p4, p5, p6;
	double minpos, maxpos;

	p0.Range(&minpos, &maxpos);
	cout << "Minpos = " << setprecision(7) << minpos << endl;
	cout << "Maxpos = " << maxpos << setprecision(0) << endl;

	p0 = 0;  checkSpecialCases(p0);
	p1 = -1;  checkSpecialCases(p1);
	p2 = -2;  checkSpecialCases(p2);
	p3 = -4;  checkSpecialCases(p3);
	p4 = -8;  checkSpecialCases(p4);
	p5 = -16;  checkSpecialCases(p5);
	p6 = -32;  checkSpecialCases(p6);
}

// what are you trying to capture with this method? TODO
// return the position of the msb of the largest binary number representable by this posit?
// this would be maxpos
template<size_t nbits, size_t es>
unsigned int maxpos_scale_f()  {
	int maxpos_exponent = nbits - 2;
	return maxpos_exponent * (1 << es);
}

int main()
{
	//ConversionOperatorsPositiveRegime();
	const size_t nbits = 5;
	const size_t es = 0;

	long long value;
	unsigned int msb;
	unsigned int maxpos_scale = maxpos_scale_f<nbits, es>();

	value = 8;
	msb = findMostSignificantBit(value) - 1;
	if (msb > maxpos_scale) {
		cerr << "msb = " << msb << " and maxpos_scale() = " << maxpos_scale << endl;
		cerr << "Can't represent " << value << " with posit<" << nbits << "," << es << ">: maxpos = " << (1 << maxpos_scale) << endl;
	}
	// we need to find the regime for this rhs
	// regime represents a scale factor of useed ^ k, where k ranges from [-nbits-1, nbits-2]
	// regime @ k = 0 -> 1
	// regime @ k = 1 -> (1 << (1 << es) ^ 1 = 2
	// regime @ k = 2 -> (1 << (1 << es) ^ 2 = 4
	// regime @ k = 3 -> (1 << (1 << es) ^ 3 = 8
	// the left shift of the regime is simply k * 2^es
	// which means that the msb of the regime is simply k*2^es
	// TODO: do you want to calculate how many bits the regime is?
	// yes: because then you can figure out if you have exponent bits and fraction bits left.
    
	return 0;
}



