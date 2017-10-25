//  fractions.cpp : tests on posit fractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

int main()
try
{
	const size_t nbits = 5;
	const size_t es = 0;

	posit<nbits,es> pa, pb, psum, pres;
	float fa, fb, fres;
	fa = -0.125f;
	fb = 1.5f;
	pa = fa;
	pb = fb;
	fres = fa + fb;
	psum = pa + pb;
	pres = fres;
	cout << pa << " " << pb << " " << psum << " " << pres << " " << fres << endl;

	cout << components_to_string(pa) << endl;
	cout << components_to_string(pb) << endl;
	cout << components_to_string(pres) << endl;

	cout << "Fraction tests" << endl;

	fraction<nbits - 2> f1, f2;
	bitset<nbits - 1> r1, r2, sum;
	bitset<nbits - 2> result_fraction;
	int f1_scale = pa.scale();
	f1 = pa.get_fraction();
	f1.normalize(r1); // hidden bit is implicit
	f2 = pb.get_fraction();
	int f2_scale = pb.scale();
	f2.denormalize(f1_scale - f2_scale, r2);
	cout << f1 << " " << r1 << endl;
	cout << f2 << " " << r2 << endl;
	bool carry = add_unsigned<nbits - 1>(r1, r2, sum);
	cout << "carry " << carry << " sum " << sum << endl;
	if (carry) {

	}
	else {
		sum <<= 1;
		for (int i = 0; i < nbits - 2; i++) {
			result_fraction[i] = sum[i + 1];
		}
	}
	cout << "fraction " << result_fraction << endl;

	return 0;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
