// unum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main()
{
	Posit<5,3,8> p1, p2, p3, p4, p5, p6;

	std::cin >> p1 >> p2;

	p2 = 2;
	p3 = p1 + p2;
	p4 = p2 - p1;
	p5 = p2 * p3;
	p6 = p5 / p3;

	std::cout << "p1: " << p1 << "\n";
	std::cout << "p2: " << p2 << "\n";
	std::cout << "p3: " << p3 << "\n";
	std::cout << "p4: " << p4 << "\n";
	std::cout << "p5: " << p5 << "\n";
	std::cout << "p6: " << p6 << "\n";
	std::cout << "p1++ " << p1++ << " " << p1 << "\n";
	std::cout << "++p1 " << ++p1 << "\n";
	std::cout << "p1-- " << p1-- << " " << p1 << "\n";
	std::cout << "--p1 " << --p1 << "\n";
    return 0;
}

