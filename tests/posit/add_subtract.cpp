// create_small_posit_lookup_table.cpp

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/*
	Testing the reciprocal nature of positive and negative posits
*/


int main(int argc, char** argv)
{
	posit<5, 0> p1, p2, p3, p4, p8, p16; 
	p1 = 1;
	p2 = 2;
	p3 = p1 + p2;

	p4 = 4;
	p8 = 8;
	p16 = 16;

	cout << "Result = " << p3 << endl;

	return 0;
}
