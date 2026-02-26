#pragma once
// sqrt_tables.hpp: specialized classic floating-point cfloat configurations 
//                  to support efficient sqrt for small cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// need a better code generator for the small posits up to nbits = 8
// TODO: find if there is any structure in these tables across nbits and es

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
void GenerateSqrtTable() {
	constexpr unsigned int NR_VALUES = (unsigned(1) << (nbits - 1)); // no need for negative posits

	std::cout << std::setprecision(20);
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> a;
	for (unsigned int i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		double ref = std::sqrt(double(a));
		cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> csqrt(ref);
		std::cout << to_binary(a) << " " << to_binary(csqrt) << "      " << a << " " << csqrt << " ref: " << ref << std::endl;
	}
	std::cout << std::setprecision(5);
}

// roots for cfloat<8,2>
//   v   r       v   r          high precision root
//  000 000      0   0     ref : 0
//  001 001      0.5 0.5   ref : 0.70710678118654757274
//	010 010      1   1     ref : 1
//	011 010      2   1     ref : 1.4142135623730951455
constexpr unsigned cfloat_8_2_roots[4] = { 0, 1, 2, 2 };


}} // namespace sw::universal
