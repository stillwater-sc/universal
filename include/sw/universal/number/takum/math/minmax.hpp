// minmax.hpp: min/max functions for takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

namespace sw { namespace universal {

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> min(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	return (x < y) ? x : y;
}

template<unsigned nbits, unsigned rbits, typename bt>
takum<nbits, rbits, bt> max(const takum<nbits, rbits, bt>& x, const takum<nbits, rbits, bt>& y) {
	return (x < y) ? y : x;
}

}} // namespace sw::universal
