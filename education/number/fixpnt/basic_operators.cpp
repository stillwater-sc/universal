// basic_operators.cpp : examples of the basic arithmetic operators using fixpnts
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>

// quick helper to report on a fixpnt's specialness
template<unsigned nbits, unsigned rbits>
void checkSpecialCases(sw::universal::fixpnt<nbits, rbits> fp) {
	std::cout << "fixpnt is " << (fp.iszero() ? "zero " : "non-zero ") << (fp.ispos() ? "positive " : "negative ") << std::endl;
}

// Demonstrate basic arithmetic with fixpnt numbers
int main()
try {
	using namespace sw::universal;	// standard namespace for fixpnt

	const unsigned nbits = 16;
	const unsigned rbits = 8;
	using Fixpnt = fixpnt<nbits, rbits>;
	Fixpnt p1, p2, p3, p4, p5, p6;

//	/* constexpr */ double minpos = minpos_value<nbits, rbits>();
//	/* constexpr */ double maxpos = maxpos_value<nbits, rbits>();

	p1 = 0;        checkSpecialCases(p1);

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

//	cout << "minpos      : " << minpos << '\n';
//	cout << "maxpos      : " << maxpos << '\n';

	p1 = 0; ++p1;         // another way to get to minpos
	p2 = INFINITY; --p2;  // another way to get to maxpos
//	cout << "minpos      : " << pretty_print(p1) << '\n';
//	cout << "maxpos      : " << pretty_print(p2) << '\n';

	/*
	pretty_print(fixpnt) will print the different segments of a fixpnt
	*/

	p1 = 1.0; ++p1;
	p2 = 1.0; --p2;
//	cout << "+1+eps      : " << pretty_print(p1) << '\n';
//	cout << "+1-eps      : " << pretty_print(p2) << '\n';

	p1 = -1.0; ++p1;
	p2 = -1.0; --p2;
//	cout << "-1+eps      : " << pretty_print(p1) << '\n';
//	cout << "-1-eps      : " << pretty_print(p2) << '\n';

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
