#pragma once
// debug.hpp: introspection for cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 3 of the cfloat headers (#1334). cfloat_impl.hpp DECLARES
// constexprClassParameters() but does not define it, so the arithmetic core needs no
// <iostream>. Include this header to call it, or ReportCfloatClassParameters().
#include <iostream>
#include <typeinfo>

#include <universal/internal/blockbinary/manipulators.hpp>   // to_binary on blockbinary
#include <universal/native/integers.hpp>                     // to_binary on a native limb
#include <universal/internal/blocktriple/blocktriple_debug.hpp>   // blocktriple introspection + tracing
#include <universal/number/cfloat/core.hpp>
#include <universal/number/cfloat/manipulators.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
void cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>::constexprClassParameters() const noexcept {
	std::cout << "-------------------------------------------------------------\n";
	std::cout << "type              : " << typeid(*this).name() << '\n';
	std::cout << "nbits             : " << nbits << '\n';
	std::cout << "es                : " << es << std::endl;
	std::cout << "hasSubnormals     : " << (hasSubnormals ? "true" : "false") << '\n';
	std::cout << "hasMaxExpValues   : " << (hasMaxExpValues ? "true" : "false") << '\n';
	std::cout << "isSaturating      : " << (isSaturating ? "true" : "false") << '\n';
	std::cout << "ALL_ONES          : " << to_binary(ALL_ONES, 0, true) << '\n';
	std::cout << "BLOCK_MASK        : " << to_binary(BLOCK_MASK, 0, true) << '\n';
	std::cout << "nrBlocks          : " << nrBlocks << '\n';
	std::cout << "bits in MSU       : " << bitsInMSU << '\n';
	std::cout << "MSU               : " << MSU << '\n';
	std::cout << "MSU MASK          : " << to_binary(MSU_MASK, 0, true) << '\n';
	std::cout << "SIGN_BIT_MASK     : " << to_binary(SIGN_BIT_MASK, 0, true) << '\n';
	std::cout << "LSB_BIT_MASK      : " << to_binary(LSB_BIT_MASK, 0, true) << '\n';
	std::cout << "MSU CAPTURES_EXP  : " << (MSU_CAPTURES_EXP ? "yes\n" : "no\n");
	std::cout << "EXP_SHIFT         : " << EXP_SHIFT << '\n';
	std::cout << "MSU EXP MASK      : " << to_binary(MSU_EXP_MASK, 0, true) << '\n';
	std::cout << "ALL_ONE_MASK_ES   : " << to_binary(ALL_ONES_ES) << '\n';
	std::cout << "EXP_BIAS          : " << EXP_BIAS << '\n';
	std::cout << "MAX_EXP           : " << MAX_EXP << '\n';
	std::cout << "MIN_EXP_NORMAL    : " << MIN_EXP_NORMAL << '\n';
	std::cout << "MIN_EXP_SUBNORMAL : " << MIN_EXP_SUBNORMAL << '\n';
	std::cout << "fraction Blocks   : " << fBlocks << '\n';
	std::cout << "bits in FSU       : " << bitsInFSU << '\n';
	std::cout << "FSU               : " << FSU << '\n';
	std::cout << "FSU MASK          : " << to_binary(FSU_MASK, 0, true) << '\n';
	std::cout << "topfbits          : " << topfbits << '\n';
	std::cout << "ALL_ONE_MASK_FR   : " << to_binary(ALL_ONES_FR) << '\n';
}


// convenience method to gain access to the values of the constexpr variables that govern the cfloat behavior
template<unsigned nbits, unsigned es, typename bt = uint8_t, bool hasSubnormals = false, bool hasMaxExpValues = false, bool isSaturating = false>
void ReportCfloatClassParameters() {
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> a;
	a.constexprClassParameters();
}

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
void cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>::showLimbs() const {
	for (unsigned b = 0; b < nrBlocks; ++b) {
		std::cout << to_binary(_block[nrBlocks - b - 1], sizeof(bt) * 8) << ' ';
	}
	std::cout << '\n';
}

}} // namespace sw::universal
