//  basic_operators.cpp : examples of the basic arithmetic operators using posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <posit>

// quick helper to report on a posit's specialness
template<size_t nbits, size_t es>
void checkSpecialCases(sw::unum::posit<nbits, es> p) {
	std::cout << "posit is " << (p.isZero() ? "zero " : "non-zero ") << (p.isPositive() ? "positive " : "negative ") << (p.isNaR() ? "Not a Real" : "Its a Real") << std::endl;
}

// Demonstrate basic arithmetic with posit numbers
int main()
try {
	using namespace std;
	using namespace sw::unum;	// standard namespace for posits

	const size_t nbits = 16;
	const size_t es = 1;
	posit<nbits, es> p1, p2, p3, p4, p5, p6;

	double minpos = minpos_value<nbits, es>();
	double maxpos = maxpos_value<nbits, es>();

	// the two special cases of a posit configuration: 0 and NaR
	p1 = 0;        checkSpecialCases(p1);
	p2 = INFINITY; checkSpecialCases(p2);

	p1 =  1.0;
	p2 = -1.0;
	p3 = p1 + p2;
	p4 = p2 - p1;
	p5 = p2 * p4;
	p6 = p5 / p4;

	cout << "p1          : " << setw(3) << p1 << '\n';
	cout << "p2          : " << setw(3) << p2 << '\n';
	cout << "p3 = p1 + p2: " << setw(3) << p3 << '\n';
	cout << "p4 = p2 - p1: " << setw(3) << p4 << '\n';
	cout << "p5 = p2 * p4: " << setw(3) << p5 << '\n';
	cout << "p6 = p5 / p4: " << setw(3) << p6 << '\n';

	cout << "minpos      : " << minpos << '\n';
	cout << "maxpos      : " << maxpos << '\n';

	p1 = 0; ++p1;         // another way to get to minpos
	p2 = INFINITY; --p2;  // another way to get to maxpos
	cout << "minpos      : " << pretty_print(p1) << '\n';
	cout << "maxpos      : " << pretty_print(p2) << '\n';

	/*
	pretty_print(posit) will print the different segments of a posit
	        s = sign
			r = regime
			e = exponent
			f = fraction
			q = quadrant of the projective circle in which the posit lies
			v = value of the posit
	minpos : s0 r000000000000001 e f qSE v3.7252902984619141e-09
	maxpos : s0 r111111111111111 e f qNE v268435456
	*/

	p1 = 1.0; ++p1;
	p2 = 1.0; --p2;
	cout << "+1+eps      : " << pretty_print(p1) << '\n';
	cout << "+1-eps      : " << pretty_print(p2) << '\n';

	p1 = -1.0; ++p1;
	p2 = -1.0; --p2;
	cout << "-1+eps      : " << pretty_print(p1) << '\n';
	cout << "-1-eps      : " << pretty_print(p2) << '\n';

	cout << endl;

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
