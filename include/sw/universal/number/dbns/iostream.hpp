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
	// d is INITIALISED, which the version this was moved from was not. Extraction has
	// three outcomes and they are not symmetric:
	//   success                 -> num_get stores the value
	//   good stream, bad input  -> num_get stores 0 and sets failbit (required since C++11)
	//   already-failed stream   -> the sentry fails, num_get never runs, d is UNTOUCHED
	// Only the third path was ever a problem, and there it read an indeterminate value.
	// Initialising to 0 removes that without altering either defined path: bad input
	// already yielded 0, and now a bad stream does too, which is the consistent answer.
	//
	// Guarding the assignment instead -- `if (istr >> d) r = d;` -- would leave r
	// untouched on BOTH failure paths, changing the defined one, and Universal's number
	// types are deliberately trivially constructible, so an untouched r is whatever was
	// on the stack. Whether extraction failure should leave the target alone is a real
	// design question; it is #1450's, not this refactor's.
	double d{ 0 };
	istr >> d;
	r = d;
	return istr;
}

}} // namespace sw::universal
