#pragma once
// debug.hpp: introspection for posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 3 of the posit headers (#1334). posit_impl.hpp declares
// constexprClassParameters() and showLimbs() but does not define them, so it
// needs no <iostream>. Include this header to call either.
#include <iostream>
#include <universal/number/posit/posit_impl.hpp>
#include <universal/number/posit/manipulators.hpp>   // to_binary used by the dumps below

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt>
void posit<nbits, es, bt>::constexprClassParameters() const noexcept {
	std::cout << "-------------------------------------------------------------\n";
	std::cout << "type              : " << type_tag(*this) << '\n';
	std::cout << "nbits             : " << nbits << '\n';
	std::cout << "es                : " << es << std::endl;
	std::cout << "ALL_ONES          : " << to_binary(ALL_ONES, 0, true) << '\n';
	std::cout << "BLOCK_MASK        : " << to_binary(BLOCK_MASK, 0, true) << '\n';
	std::cout << "nrBlocks          : " << nrBlocks << '\n';
	std::cout << "bits in MSU       : " << bitsInMSU << '\n';
	std::cout << "MSU               : " << MSU << '\n';
	std::cout << "MSU MASK          : " << to_binary(MSU_MASK, 0, true) << '\n';
	std::cout << "SIGN_BIT_MASK     : " << to_binary(SIGN_BIT_MASK, 0, true) << '\n';
	std::cout << "LSB_BIT_MASK      : " << to_binary(LSB_BIT_MASK, 0, true) << '\n';
}
template<unsigned nbits, unsigned es, typename bt>
void posit<nbits, es, bt>::showLimbs() const {
	for (unsigned b = 0; b < nrBlocks; ++b) {
		std::cout << to_binary(_block[nrBlocks - b - 1], sizeof(bt) * 8) << ' ';
	}
	std::cout << '\n';
}

}} // namespace sw::universal
