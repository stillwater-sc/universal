// unum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

int main()
{
	posit<16,1> p1, p2, p3, p4, p5, p6;

	cin >> p1 >> p2;

	cout << "p1 is " << (p1.isZero() ? "zero " : "non-zero ") << (p1.isPositive() ? "positive " : "negative ") << (p1.isInfinite() ? "+-infinite" : "not infinite") << endl;
	cout << "p2 is " << (p2.isZero() ? "zero " : "non-zero ") << (p2.isPositive() ? "positive " : "negative ") << (p2.isInfinite() ? "+-infinite" : "not infinite") << endl;


	p2 = -2;
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
    return 0;
}

