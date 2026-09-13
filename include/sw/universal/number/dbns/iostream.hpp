#pragma once
// iostream.hpp: stream insertion and extraction for dbns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the dbns headers (#1334): the <iostream> half. manipulators.hpp is the
// <iomanip> half -- everything that turns a dbns into a std::string. Self-contained.
//
// operator<< and operator>> were in-class friend DEFINITIONS in dbns_impl.hpp. An
// in-class friend definition pins <iostream> into every translation unit that includes
// the class, which is what blockdigit and floatcascade were fixed for. They go through
// dbns's public double conversion and assignment and need no private access, so they are
// ordinary namespace-scope templates here rather than friends there.
//
// This header includes only core.hpp -- no cycle for #pragma once to mask (#1427).
#include <iostream>      // std::ostream, std::istream
#include <universal/number/dbns/core.hpp>

namespace sw { namespace universal {

// the DbnsArithmeticStatistics report, declared in dbns_impl.hpp
inline std::ostream& operator<<(std::ostream& ostr, const DbnsArithmeticStatistics stats) {
	ostr << "Conversions                     : " << stats.conversionEvents << '\n';
	ostr << "Exponent Overflow During Search : " << stats.exponentOverflowDuringSearch << '\n';
	ostr << "Rounding Successes              : " << (stats.conversionEvents - stats.roundingFailure) << '\n';
	ostr << "Rounding Failures               : " << stats.roundingFailure << '\n';
	return ostr;
}

template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
inline std::ostream& operator<<(std::ostream& ostr, const dbns<nbits, fbbits, bt, xtra...>& r) {
	ostr << double(r);
	return ostr;
}

template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
inline std::istream& operator>>(std::istream& istr, dbns<nbits, fbbits, bt, xtra...>& r) {
	double d;
	istr >> d;
	r = d;
	return istr;
}

}} // namespace sw::universal
