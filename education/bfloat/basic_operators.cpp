// basic_operators.cpp : examples of the basic arithmetic operators using bfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/cfloat/cfloat>
// quick helper to report on a bfloat's specialness
template<size_t nbits, size_t es, typename bt>
void checkSpecialCases(sw::universal::bfloat<nbits, es, bt> b) {
	std::cout << "bfloat is " << (b.iszero() ? "zero " : "non-zero ") << (b.ispos() ? "positive " : "negative ") << (b.isnan() ? "Not a Number" : "Its a Real") << std::endl;
}

// Demonstrate basic arithmetic with bfloat numbers
int main()
try {
	using namespace std;
	using namespace sw::universal;	// standard namespace for bfloat

	constexpr size_t nbits = 16;
	constexpr size_t es = 5;
	using bt = uint16_t;  // storage block type
	using Real = bfloat<nbits, es, bt>;   // construct the Real number we want
	Real b1, b2, b3, b4, b5, b6;

	/* constexpr */ Real minpos; // minpos(minpos);
	/* constexpr */ Real maxpos; // maxpos();

	// the three special cases of a bfloat configuration: 0, +-Inf, and +-NaN
	b1 = 0;        checkSpecialCases(b1);
	b2 = INFINITY; checkSpecialCases(b2);
	b3 = NAN;      checkSpecialCases(b3);

	b1 =  1.0;
	b2 = -1.0;
	b3 = b1 + b2;
	b4 = b2 - b1;
	b5 = b2 * b4;
	b6 = b5 / b4;

	cout << "b1          : " << setw(3) << b1 << '\n';
	cout << "b2          : " << setw(3) << b2 << '\n';
	cout << "b3 = b1 + b2: " << setw(3) << b3 << '\n';
	cout << "b4 = b2 - b1: " << setw(3) << b4 << '\n';
	cout << "b5 = b2 * b4: " << setw(3) << b5 << '\n';
	cout << "b6 = b5 / b4: " << setw(3) << b6 << '\n';

	cout << "minpos      : " << minpos << '\n';
	cout << "maxpos      : " << maxpos << '\n';

	b1 = 0; ++b1;         // another way to get to minpos
	b2 = INFINITY; --b2;  // another way to get to maxpos
	cout << "minpos      : " << pretty_print(b1) << '\n';
	cout << "maxpos      : " << pretty_print(b2) << '\n';

	/*
	pretty_print(posit) will print the different segments of bfloat
	        s = sign
		e = exponent
		f = fraction
		q = quadrant of the projective circle in which the real lies
		v = value of the bfloat
	minpos : s0 r000000000000001 e f qSE v3.7252902984619141e-09
	maxpos : s0 r111111111111111 e f qNE v268435456
	*/

	b1 = 1.0; ++b1;
	b2 = 1.0; --b2;
	cout << "+1+eps      : " << pretty_print(b1) << '\n';
	cout << "+1-eps      : " << pretty_print(b2) << '\n';

	b1 = -1.0; ++b1;
	b2 = -1.0; --b2;
	cout << "-1+eps      : " << pretty_print(b1) << '\n';
	cout << "-1-eps      : " << pretty_print(b2) << '\n';

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
