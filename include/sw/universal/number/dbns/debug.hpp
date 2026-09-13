#pragma once
// debug.hpp: introspection for dbns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 3 of the dbns headers (#1334). dbns_impl.hpp declares
// debugConstexprParameters() but does not define it, so the core needs no <iostream>.
// Its definition lives here. Include this header to call it; forgetting to is a link
// error naming the missing function, which is the better failure -- the same shape
// blocktriple's constexprClassParameters() got in #1388.
//
// The DBNS_TRACE_CONVERSION switch needs no help from this header: it guards its own
// <iostream> include in dbns_impl.hpp, because the traced code is inside the class.
#include <iostream>
#include <universal/number/dbns/core.hpp>
#include <universal/number/dbns/manipulators.hpp>   // type_tag, used in the report
#include <universal/native/integers.hpp>           // to_binary(Integer, bool, int) on the masks
//
// NOTE, unchanged from main: the mask reports below call to_binary(mask, bitsInBlock),
// and that overload's second parameter is `bool bNibbleMarker`, not a width -- so
// bitsInBlock has always been read as `true` and the width has always defaulted to 0.
// The output is what it has always been; correcting it would be a behaviour change and
// belongs in its own commit, not in a layering refactor.

namespace sw { namespace universal {

template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
void dbns<nbits, fbbits, bt, xtra...>::debugConstexprParameters() {
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
	std::cout << "FB_MASK               " << to_binary(FB_MASK, bitsInBlock) << '\n';
	std::cout << "SB_MASK               " << to_binary(SB_MASK, bitsInBlock) << '\n';
}

}} // namespace sw::universal
