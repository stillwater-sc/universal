#pragma once
// sqrt_tables.hpp: specialized fixed-point configurations 
//                  to support efficient sqrt for small fixpnts
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// need a better code generator for the small posits up to nbits = 8
// TODO: find if there is any structure in these tables across nbits and es

template<size_t nbits, size_t rbits, bool arithmetic, typename bt>
void GenerateSqrtTable() {
	constexpr unsigned int NR_VALUES = (unsigned(1) << (nbits - 1)); // no need for negative posits

	std::cout << std::setprecision(20);
	fixpnt<nbits, rbits, arithmetic, bt> a;
	for (unsigned int i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		double ref = std::sqrt(double(a));
		fixpnt<nbits, rbits, arithmetic, bt> csqrt(ref);
		std::cout << to_binary(a) << " " << to_binary(csqrt) << "      " << a << " " << csqrt << " ref: " << ref << std::endl;
	}
	std::cout << std::setprecision(5);
}

}} // namespace sw::universal
