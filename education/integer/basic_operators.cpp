// basic_operators.cpp : examples of the basic arithmetic operators using integers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/integer/integer>

// quick helper to report on a posit's specialness
template<size_t nbits>
void checkSpecialCases(sw::universal::integer<nbits> i) {
	std::cout << "integer is " << (i.iszero() ? "zero " : "non-zero ") << (i.ispos() ? "positive " : "negative ") << std::endl;
}

// Demonstrate basic arithmetic with integer numbers
int main()
try {
	using namespace std;
	using namespace sw::universal;	// standard namespace for integer<>

	using bt = uint8_t;
	const size_t nbits = 16;
	using Integer = integer<nbits, bt>;
	Integer i1, i2, i3, i4, i5, i6;

	/* constexpr */ Integer minpos{ 1 };
	/* constexpr */ Integer maxpos = max_int<nbits, bt>();

	// the two special cases of a posit configuration: 0 and NaR
	i1 = 0;        checkSpecialCases(i1);
	i2 = INFINITY; checkSpecialCases(i2);

	i1 =  1.0;
	i2 = -1.0;
	i3 = i1 + i2;
	i4 = i2 - i1;
	i5 = i2 * i4;
	i6 = i5 / i4;

	cout << "i1          : " << setw(3) << i1 << '\n';
	cout << "i2          : " << setw(3) << i2 << '\n';
	cout << "i3 = i1 + i2: " << setw(3) << i3 << '\n';
	cout << "i4 = i2 - i1: " << setw(3) << i4 << '\n';
	cout << "i5 = i2 * i4: " << setw(3) << i5 << '\n';
	cout << "i6 = i5 / i4: " << setw(3) << i6 << '\n';

	cout << "minpos      : " << minpos << '\n';
	cout << "maxpos      : " << maxpos << '\n';

	i1 = 0; ++i1;         // another way to get to minpos
	i2 = INFINITY; --i2;  // another way to get to maxpos
	cout << "minpos      : " << to_binary(i1) << '\n';
	cout << "maxpos      : " << to_binary(i2) << '\n';

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
