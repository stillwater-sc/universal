// basic_operators.cpp : examples of the basic arithmetic operators using classic cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/cfloat/cfloat.hpp>

// quick helper to report on a cfloat's specialness
template<unsigned nbits, unsigned es, typename bt>
void checkSpecialCases(sw::universal::cfloat<nbits, es, bt> b) {
	std::cout << "cfloat is " << (b.iszero() ? "zero " : "non-zero ") << (b.ispos() ? "positive " : "negative ") << (b.isnan() ? "Not a Number" : "Its a Real") << std::endl;
}

// Demonstrate basic arithmetic with cfloat numbers
int main()
try {
	using namespace sw::universal;	// standard namespace for cfloat

	constexpr unsigned nbits = 16;
	constexpr unsigned es = 5;
	using bt = uint16_t;  // storage block type
	using Real = cfloat<nbits, es, bt>;   // construct the Real number we want
	Real b1, b2, b3, b4, b5, b6;

	/* constexpr */ Real minpos; // minpos(minpos);
	/* constexpr */ Real maxpos; // maxpos();

	// the three special cases of a cfloat configuration: 0, +-Inf, and +-NaN
	b1 = 0;        checkSpecialCases(b1);
	b2 = INFINITY; checkSpecialCases(b2);
	b3 = NAN;      checkSpecialCases(b3);

	b1 =  1.0;
	b2 = -1.0;
	b3 = b1 + b2;
	b4 = b2 - b1;
	b5 = b2 * b4;
	b6 = b5 / b4;

	std::cout << "b1          : " << std::setw(3) << b1 << '\n';
	std::cout << "b2          : " << std::setw(3) << b2 << '\n';
	std::cout << "b3 = b1 + b2: " << std::setw(3) << b3 << '\n';
	std::cout << "b4 = b2 - b1: " << std::setw(3) << b4 << '\n';
	std::cout << "b5 = b2 * b4: " << std::setw(3) << b5 << '\n';
	std::cout << "b6 = b5 / b4: " << std::setw(3) << b6 << '\n';

	std::cout << "minpos      : " << minpos << '\n';
	std::cout << "maxpos      : " << maxpos << '\n';

	b1 = 0; ++b1;         // another way to get to minpos
	b2 = INFINITY; --b2;  // another way to get to maxpos
	std::cout << "minpos      : " << pretty_print(b1) << '\n';
	std::cout << "maxpos      : " << pretty_print(b2) << '\n';

	/*
	pretty_print(posit) will print the different segments of cfloat
	        s = sign
		e = exponent
		f = fraction
		q = quadrant of the projective circle in which the real lies
		v = value of the cfloat
	minpos : s0 r000000000000001 e f qSE v3.7252902984619141e-09
	maxpos : s0 r111111111111111 e f qNE v268435456
	*/

	b1 = 1.0; ++b1;
	b2 = 1.0; --b2;
	std::cout << "+1+eps      : " << pretty_print(b1) << '\n';
	std::cout << "+1-eps      : " << pretty_print(b2) << '\n';

	b1 = -1.0; ++b1;
	b2 = -1.0; --b2;
	std::cout << "-1+eps      : " << pretty_print(b1) << '\n';
	std::cout << "-1-eps      : " << pretty_print(b2) << '\n';

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
