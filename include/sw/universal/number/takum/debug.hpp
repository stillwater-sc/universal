#pragma once
// debug.hpp: introspection for takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 3 of the takum headers (#1334). takum_impl.hpp DECLARES
// debugConstexprParameters() but does not define it, so the core needs no <iostream>.
#include <iostream>
#include <universal/native/integers.hpp>            // to_binary on a native limb
#include <universal/number/takum/core.hpp>
#include <universal/number/takum/manipulators.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
void takum<nbits, rbits, bt>::debugConstexprParameters() {
	std::cout << "constexpr parameters for " << type_tag(*this) << '\n';
	std::cout << "nbits                 " << nbits << '\n';
	std::cout << "rbits                 " << rbits << '\n';
	std::cout << "overhead              " << overhead << '\n';
	std::cout << "dr_bits               " << dr_bits << '\n';
	std::cout << "nr_dr_values          " << nr_dr_values << '\n';
	std::cout << "max_r                 " << max_r << '\n';
	std::cout << "maxCharBits           " << maxCharBits << '\n';
	std::cout << "min characteristic    " << min_characteristic() << '\n';
	std::cout << "max characteristic    " << max_characteristic() << '\n';
	std::cout << "bitsInBlock           " << bitsInBlock << '\n';
	std::cout << "nrBlocks              " << nrBlocks << '\n';
	std::cout << "MSU_MASK              " << to_binary(MSU_MASK, bitsInBlock) << '\n';
	std::cout << "SIGN_BIT_MASK         " << to_binary(SIGN_BIT_MASK, bitsInBlock) << '\n';
}

}} // namespace sw::universal
