// unum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

int main()
{
	const size_t nbits = 8;
	const size_t es = 2;

	int k_max = nbits - 1;
	int bound = (k_max << es);
	float upper_range = float(useed<nbits, es>());

	// regime component of the posit
	regime<nbits> test_regime;
	for (int scale = -bound; scale < bound; scale++) {
		int k = scale >> es;
		test_regime.assign_regime_pattern(k);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime.get() << endl;
	}
    
	// exponent component of the posit
	exponent<nbits, es> test_exponent;
	for (int scale = -bound; scale < bound; scale++) {
		int k = scale >> es;
		unsigned int nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		unsigned int nrOfExponentBits = test_exponent.assign_exponent_bits(scale, nrOfRegimeBits);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime.get() << " " << test_exponent << endl;
	}

	// fraction component of the posit

	return 0;
}



