//  basic_operators.cpp : conversion operators for positive regime of posit numbers
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

void BasicOperators() {
	posit<16, 1> p1, p2, p3, p4, p5, p6;

	p1.Range();

	p1 = 0;  checkSpecialCases(p1);
	p1 = 1;  checkSpecialCases(p1);
	p2 = 2;  checkSpecialCases(p2);

	p3 = p1 + p2;
	p4 = p2 - p1;
	p5 = p2 * p3;
	p6 = p5 / p3;

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

	// negative regime
	p1 = -1; checkSpecialCases(p1);
}


int main()
{
	BasicOperators();
    
	return 0;
}



