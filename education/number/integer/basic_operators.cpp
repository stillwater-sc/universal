// basic_operators.cpp : examples of the basic arithmetic operators using integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/integer/integer.hpp>

// quick helper to report on a posit's specialness
template<unsigned nbits, typename BlockType>
void checkSpecialCases(const sw::universal::integer<nbits, BlockType>& i) {
	std::cout << "integer is " << (i.iszero() ? "zero " : "non-zero ") << (i.ispos() ? "positive " : "negative ") << std::endl;
}

// Demonstrate basic arithmetic with integer numbers
int main()
try {
	using namespace sw::universal;	// standard namespace for integer<>

	using bt = uint8_t;
	const unsigned nbits = 16;
	using Integer = integer<nbits, bt>;
	Integer i1, i2, i3, i4, i5, i6;

	// TODO: enable constexpr
	/* constexpr */ Integer minpos(SpecificValue::minpos); // this would simply be the value '1'
	/* constexpr */ Integer maxpos(SpecificValue::maxpos);

	// the two special cases of a posit configuration: 0 and NaR
	i1 =  0;       checkSpecialCases(i1);
	i2 = -1;       checkSpecialCases(i2);
	i3 =  1;       checkSpecialCases(i3);

	i1 =  1.0;
	i2 = -1.0;
	i3 = i1 + i2;
	i4 = i2 - i1;
	i5 = i2 * i4;
	i6 = i5 / i4;

	std::cout << "i1          : " << std::setw(3) << i1 << '\n';
	std::cout << "i2          : " << std::setw(3) << i2 << '\n';
	std::cout << "i3 = i1 + i2: " << std::setw(3) << i3 << '\n';
	std::cout << "i4 = i2 - i1: " << std::setw(3) << i4 << '\n';
	std::cout << "i5 = i2 * i4: " << std::setw(3) << i5 << '\n';
	std::cout << "i6 = i5 / i4: " << std::setw(3) << i6 << '\n';

	std::cout << "minpos      : " << std::setw(10) << minpos << " : " << to_binary(minpos) << '\n';
	std::cout << "maxpos      : " << std::setw(10) << maxpos << " : " << to_binary(maxpos) << '\n';

	i1 = 0; ++i1;            // another way to get to minpos
	i2 = 0; --i2; i2 >>= 1;  // another way to get to maxpos
	std::cout << "minpos      : " << std::setw(10) << i1 << " : " << to_binary(i1) << '\n';
	std::cout << "maxpos      : " << std::setw(10) << i2 << " : " << to_binary(i2) << '\n';

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
