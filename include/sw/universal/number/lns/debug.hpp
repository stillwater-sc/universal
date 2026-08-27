#pragma once
// debug.hpp: introspection for lns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 3 of the lns headers (#1334). lns_impl.hpp DECLARES debugConstexprParameters()
// and the LnsArithmeticStatistics printer but does not define them, so the arithmetic
// core needs no <iostream>. Include this header to call either.
#include <iostream>
#include <universal/native/integers.hpp>   // to_binary on a native limb, used by the dump
#include <universal/number/lns/core.hpp>
#include <universal/number/lns/manipulators.hpp>   // type_tag / to_binary used by the dump

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
void lns<nbits, rbits, bt, xtra...>::debugConstexprParameters() {
	std::cout << "constexpr parameters for " << type_tag(*this) << '\n';
	std::cout << "scaling               " << scaling << '\n';
	std::cout << "bitsInByte            " << bitsInByte << '\n';
	std::cout << "bitsInBlock           " << bitsInBlock << '\n';
	std::cout << "nrBlocks              " << nrBlocks << '\n';
	std::cout << "storageMask           " << to_binary(storageMask, bitsInBlock) << '\n';
	std::cout << "MSU                   " << MSU << '\n';
	std::cout << "MSU_MASK              " << to_binary(MSU_MASK, bitsInBlock) << '\n';
	std::cout << "MSB_UNIT              " << MSB_UNIT << '\n';
	std::cout << "SPECIAL_BITS_TOGETHER " << (SPECIAL_BITS_TOGETHER ? "yes" : "no") << '\n';
	std::cout << "SIGN_BIT_MASK         " << to_binary(SIGN_BIT_MASK, bitsInBlock) << '\n';
	std::cout << "MSB_BIT_MASK          " << to_binary(MSB_BIT_MASK, bitsInBlock) << '\n';
	std::cout << "BLOCK_MSB_MASK        " << to_binary(BLOCK_MSB_MASK, bitsInBlock) << '\n';
	std::cout << "MSU_ZERO              " << to_binary(MSU_ZERO, bitsInBlock) << '\n';
	std::cout << "MSU_NAN               " << to_binary(MSU_NAN, bitsInBlock) << '\n';
	std::cout << "maxShift              " << maxShift << '\n';
	std::cout << "leftShift             " << leftShift << '\n';
	std::cout << "min_exponent          " << min_exponent << '\n';
	std::cout << "max_exponent          " << max_exponent << '\n';
}

inline std::ostream& operator<<(std::ostream& ostr, const LnsArithmeticStatistics& stats) {
	ostr << "Conversions                     : " << stats.conversionEvents << '\n';
	return ostr;
}

}} // namespace sw::universal
