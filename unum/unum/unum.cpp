// unum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

template<size_t nbits, size_t es>
void extract(uint64_t f, int fes, int fms) {
	int exponentBias = SHIFT(fes - 1) - 1;
	int16_t exponent = (f >> fms) & ((1 << fes) - 1);
	uint64_t mantissa = (f & ((1ULL << fms) - 1));

	cout << " mantissa : " << mantissa << " exponent : " << exponent << " bias " << exponentBias << endl;

	// clip exponent
	int rmin = SHIFT(es) * (2 - nbits);
	int rmax = SHIFT(es) * (nbits - 2);
	int rf = MIN(MAX(exponent - exponentBias, rmin), rmax);

	cout << "rmin " << rmin << " rmax " << rmax << " rf " << rf << endl;

	bool rsign = f >> (fes + fms);
	int rreg = rf >> es;
	int rexp = rf - SHIFT(es) * rreg;

	cout << "rsign " << rsign << " rreg " << rreg << " rexp " << rexp << endl;
}

int main()
{
	posit<16,1> p1, p2, p3, p4, p5, p6;

	cin >> p1 >> p2;

	cout << "p1 is " << (p1.isZero() ? "zero " : "non-zero ") << (p1.isPositive() ? "positive " : "negative ") << (p1.isInfinite() ? "+-infinite" : "not infinite") << endl;
	cout << "p2 is " << (p2.isZero() ? "zero " : "non-zero ") << (p2.isPositive() ? "positive " : "negative ") << (p2.isInfinite() ? "+-infinite" : "not infinite") << endl;

	long long aLong = -65536;
	p2 = aLong;
	p3 = p1 + p2;
	p4 = p2 - p1;
	p5 = p2 * p3;
	p6 = p5 / p3;

	p1.Info();
	cout << "p1: " << p1 << "\n";
	cout << "p2: " << p2 << "\n";
	cout << "p3: " << p3 << "\n";
	cout << "p4: " << p4 << "\n";
	cout << "p5: " << p5 << "\n";
	cout << "p6: " << p6 << "\n";
	cout << "p1++ " << p1++ << " " << p1 << "\n";
	cout << "++p1 " << ++p1 << "\n";
	cout << "p1-- " << p1-- << " " << p1 << "\n";
	cout << "--p1 " << --p1 << "\n";

	union {
		float f1 = 1.0f;
		uint32_t bits;
	};
	extract<16,1>(bits, 8, 23);
    return 0;
}



