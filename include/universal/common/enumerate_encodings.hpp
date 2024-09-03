#pragma once
// enumerate_encodings.hpp: enumerate the ordered encodings of an arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// enumerate the valid encodings of a number system
template<typename NumberType>
void EnumerateValidEncodingsViaIncrement(std::ostream& ostr, double lowerbound = 0.0f, double upperbound = 0.0f) {
	//constexpr unsigned nbits = NumberType::nbits;

	NumberType maxneg(SpecificValue::maxneg), maxpos(SpecificValue::maxpos);
	NumberType a, ub;

	if (upperbound == lowerbound) {
		a = maxneg;
		ub = maxpos;
	}
	else {
		a = lowerbound;
		ub = upperbound;
	}
	while (a <= ub) {
		ostr << to_binary(a, true) << " : " << a << '\n';
		++a;
	}
}

template<typename NumberType>
void EnumerateValidEncodingsViaDecrement(std::ostream& ostr, double upperbound = 0.0f, double lowerbound = 0.0f) {
	//constexpr unsigned nbits = NumberType::nbits;

	NumberType maxneg(SpecificValue::maxneg), maxpos(SpecificValue::maxpos);
	NumberType a, lb;

	if (upperbound == lowerbound) {
		a = maxpos;
		lb = maxneg;
	}
	else {
		a = upperbound;
		lb = lowerbound;
	}
	while (a >= lb) {
		ostr << to_binary(a, true) << " : " << a << '\n';
		--a;
	}
}

}} // namespace sw::universal
