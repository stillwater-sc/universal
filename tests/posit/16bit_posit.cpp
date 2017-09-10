// Functionality tests for standard 16-bit posits

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/*
Standard posits with nbits = 16 have 1 exponent bit.
*/

int main(int argc, char** argv)
{
	cout << "Standard posit<16,1> configuration tests" << endl;
	try {
        const size_t nbits = 16;
        const size_t es = 1;
        posit<nbits,es> p;

		cout << spec_to_string(p) << endl;
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
