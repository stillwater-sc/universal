// basic_operators.cpp : examples of the basic arithmetic operators using posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit/posit.hpp>

// quick helper to report on a posit's specialness
template<unsigned nbits, unsigned es>
void checkSpecialCases(sw::universal::posit<nbits, es> p) {
	std::cout << "posit is " << (p.iszero() ? "zero " : "non-zero ") << (p.ispos() ? "positive " : "negative ") << (p.isnar() ? "Not a Real" : "Its a Real") << std::endl;
}

// Demonstrate basic arithmetic with posit numbers
int main()
try {
	using namespace sw::universal;	// standard namespace for posits

	const unsigned nbits = 16;
	const unsigned es = 1;
	posit<nbits, es> p1, p2, p3, p4, p5, p6;

	/* constexpr */ double minpos = double(posit<nbits, es>(SpecificValue::minpos));
	/* constexpr */ double maxpos = double(posit<nbits, es>(SpecificValue::maxpos));

	// the two special cases of a posit configuration: 0 and NaR
	p1 = 0;        checkSpecialCases(p1);
	p2 = INFINITY; checkSpecialCases(p2);

	p1 =  1.0;
	p2 = -1.0;
	p3 = p1 + p2;
	p4 = p2 - p1;
	p5 = p2 * p4;
	p6 = p5 / p4;

	std::cout << "p1          : " << std::setw(3) << p1 << '\n';
	std::cout << "p2          : " << std::setw(3) << p2 << '\n';
	std::cout << "p3 = p1 + p2: " << std::setw(3) << p3 << '\n';
	std::cout << "p4 = p2 - p1: " << std::setw(3) << p4 << '\n';
	std::cout << "p5 = p2 * p4: " << std::setw(3) << p5 << '\n';
	std::cout << "p6 = p5 / p4: " << std::setw(3) << p6 << '\n';

	std::cout << "minpos      : " << minpos << '\n';
	std::cout << "maxpos      : " << maxpos << '\n';

	p1 = 0; ++p1;         // another way to get to minpos
	p2 = INFINITY; --p2;  // another way to get to maxpos
	std::cout << "minpos      : " << pretty_print(p1) << '\n';
	std::cout << "maxpos      : " << pretty_print(p2) << '\n';

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
	std::cout << "+1+eps      : " << pretty_print(p1) << '\n';
	std::cout << "+1-eps      : " << pretty_print(p2) << '\n';

	p1 = -1.0; ++p1;
	p2 = -1.0; --p2;
	std::cout << "-1+eps      : " << pretty_print(p1) << '\n';
	std::cout << "-1-eps      : " << pretty_print(p2) << '\n';

	std::cout << std::endl;

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
