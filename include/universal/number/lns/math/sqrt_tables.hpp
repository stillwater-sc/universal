#pragma once
// sqrt_tables.hpp: specialized logarithmic floating-point 
//                  to support efficient sqrt for small lns configurations
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// need a better code generator for the small lns up to nbits = 8
// TODO: find if there is any structure in these tables across nbits and es

template<size_t nbits, size_t rbits, typename bt, auto... xtra>
void GenerateSqrtTable() {
	constexpr unsigned int NR_VALUES = (unsigned(1) << (nbits - 1)); // no need for negative posits

	std::cout << std::setprecision(20);
	lns<nbits, rbits, bt, xtra...> a;
	for (unsigned int i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		double ref = std::sqrt(double(a));
		lns<nbits, rbits, bt, xtra...> csqrt(ref);
		std::cout << to_binary(a) << " " << to_binary(csqrt) << "      " << a << " " << csqrt << " ref: " << ref << std::endl;
	}
	std::cout << std::setprecision(5);
}

// roots for lns<8,2>
//   v   r       v   r          high precision root
//  000 000      0   0     ref : 0
//  001 001      0.5 0.5   ref : 0.70710678118654757274
//	010 010      1   1     ref : 1
//	011 010      2   1     ref : 1.4142135623730951455
constexpr unsigned lns_8_2_roots[4] = { 0, 1, 2, 2 };


}} // namespace sw::universal
