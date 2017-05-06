// bfp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main()
{
	Posit p1(16, 1);
	Posit p2(16, 1);

	p1.set(1.0f);
	p2.set(2.0f);

	Posit p3(16, 1);
	p3 = p1.add(p2);

	std::cout << "p1 = " << p1.getFloat() << std::endl;
	std::cout << "p2 = " << p2.getFloat() << std::endl;
	std::cout << "p3 = " << p3.getFloat() << std::endl;

    return 0;
}

