#pragma once
// debug.hpp: introspection for areal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 3 of the areal headers (#1334). areal_impl.hpp declares
// constexprClassParameters() but does not define it, so the core needs no <iostream>.
// Its definition lives here. Include this header to call it; forgetting to is a link
// error naming the missing function, which is the better failure -- the same shape
// blocktriple's constexprClassParameters() got in #1388.
//
// The TRACE_CONVERSION switch needs no help from this header: it guards its own
// <iostream> include in areal_impl.hpp, because the traced code is inside the class.
#include <iostream>
#include <universal/number/areal/core.hpp>
#include <universal/native/integers.hpp>   // to_binary(Integer, int, bool) on the masks

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, typename bt>
void areal<nbits, es, bt>::constexprClassParameters() const {
	std::cout << "nbits             : " << nbits << '\n';
	std::cout << "es                : " << es << std::endl;
	std::cout << "ALLONES           : " << to_binary(ALLONES, bitsInBlock, true) << '\n';
	std::cout << "BLOCK_MASK        : " << to_binary(BLOCK_MASK, bitsInBlock, true) << '\n';
	std::cout << "nrBlocks          : " << nrBlocks << '\n';
	std::cout << "bits in MSU       : " << bitsInMSU << '\n';
	std::cout << "MSU               : " << MSU << '\n';
	std::cout << "MSU MASK          : " << to_binary(MSU_MASK, bitsInBlock, true) << '\n';
	std::cout << "SIGN_BIT_MASK     : " << to_binary(SIGN_BIT_MASK, bitsInBlock, true) << '\n';
	std::cout << "LSB_BIT_MASK      : " << to_binary(LSB_BIT_MASK, bitsInBlock, true) << '\n';
	std::cout << "MSU CAPTURES E    : " << (MSU_CAPTURES_E ? "yes\n" : "no\n");
	std::cout << "EXP_SHIFT         : " << EXP_SHIFT << '\n';
	std::cout << "MSU EXP MASK      : " << to_binary(MSU_EXP_MASK, bitsInBlock, true) << '\n';
	std::cout << "EXP_BIAS          : " << EXP_BIAS << '\n';
	std::cout << "MAX_EXP           : " << MAX_EXP << '\n';
	std::cout << "MIN_EXP_NORMAL    : " << MIN_EXP_NORMAL << '\n';
	std::cout << "MIN_EXP_SUBNORMAL : " << MIN_EXP_SUBNORMAL << '\n';
}

}} // namespace sw::universal
