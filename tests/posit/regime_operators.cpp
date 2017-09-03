// create_small_posit_lookup_table.cpp

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/*
Regime operators tests the regime manipulation of the posit class
*/

int main(int argc, char** argv)
{
	const size_t nbits = 64;
	const size_t es = 0;
    posit<nbits,es> p;
	try {
		for (int i = 0; i < 2*nbits-1; i++) {
			int k = i - nbits + 1;
			p.reset();
			cout << "k = " << setw(3) << k; 
			cout << " # Pattern bits = " << setw(3) << p.assign_regime_pattern(k);
			cout << " Regime = " << to_binary(p.get()) << endl;
		}
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
