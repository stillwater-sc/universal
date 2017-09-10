// Functionality tests for standard 8-bit posits

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/*
Standard posits with nbits = 8 have no exponent bits.
*/

int main(int argc, char** argv)
{
	cout << "Standard posit<8,0> configuration tests" << endl;
	try {
        const size_t nbits = 8;
        const size_t es = 0;
        posit<nbits,es> p;

		cout << spec_to_string(p) << endl;
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
