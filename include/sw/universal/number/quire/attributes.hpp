#pragma once
// attributes.hpp: functions to query generalized quire attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// functions to provide details about properties of a quire configuration

// get the sign of the generalized quire
template<typename NumberType, unsigned capacity, typename LimbType>
constexpr inline bool sign(const quire<NumberType, capacity, LimbType>& q) {
	return q.isneg();
}

// calculate the scale of a generalized quire
template<typename NumberType, unsigned capacity, typename LimbType>
inline int scale(const quire<NumberType, capacity, LimbType>& q) {
	return q.scale();
}

//////////////////////////////////////////////////////////////////////////

}} // namespace sw::universal
