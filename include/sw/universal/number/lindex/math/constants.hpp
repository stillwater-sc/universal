#pragma once
// constants.hpp: working-precision constants of the lindex math library (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <type_traits>
#include <universal/number/dd_cascade/math/constants/dd_cascade_constants.hpp>
#include <universal/number/qd_cascade/math/constants/qd_cascade_constants.hpp>

namespace sw { namespace universal { namespace lindex_math {

// ln 2 and ln 10 in the working type W, to its precision
template<typename W>
W ln2() noexcept {
	if constexpr (std::is_same_v<W, double>) return 0.69314718055994530942;
	else if constexpr (std::is_same_v<W, dd_cascade>) return ddc_ln2;
	else return qdc_ln2;
}
template<typename W>
W ln10() noexcept {
	if constexpr (std::is_same_v<W, double>) return 2.30258509299404568402;
	else if constexpr (std::is_same_v<W, dd_cascade>) return ddc_ln10;
	else return qdc_ln10;
}

// e^t for the signed t = (negative ? -1 : +1) phi(|t|), with |t| an unrounded psi point:
// one level up when t >= 0, and e^-phi(|t|) at level 0 below, where a |t| past W is inf and
// e^-inf is 0
template<typename L, typename W>
typename L::template outcome<W> exp_of(const level_index::point<W>& t, bool negative) noexcept {
	using std::exp;
	if (!negative) return L::template up_a_level<W>(t);
	const W T = level_index::phi(t);
	return L::template from_point<W>(level_index::psi(W(exp(-T))));
}

}}}  // namespace sw::universal::lindex_math
