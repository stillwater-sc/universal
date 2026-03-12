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

	return 0;
}

// calculate the significant of a generalized quire
template<typename NumberType, unsigned capacity, typename LimbType>
inline blockbinary<quire_traits<NumberType>::half_range, LimbType, BinaryNumberType::Unsigned>
significant(const quire<NumberType, capacity, LimbType>& q) {

	return blockbinary<quire_traits<NumberType>::half_range + 1, LimbType, BinaryNumberType::Unsigned>{};
}

// get the fraction bits of a generalized quire
template<typename NumberType, unsigned capacity, typename LimbType>
inline blockbinary<quire_traits<NumberType>::half_range, LimbType, BinaryNumberType::Unsigned>
extract_fraction(const quire<NumberType, capacity, LimbType>& q) {

	return blockbinary<quire_traits<NumberType>::half_range + 1, LimbType, BinaryNumberType::Unsigned>{};
}

//////////////////////////////////////////////////////////////////////////

}} // namespace sw::universal
