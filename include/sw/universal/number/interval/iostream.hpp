#pragma once
// iostream.hpp: stream insertion and extraction for interval
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the interval headers (#1334): the <iostream> half. manipulators.hpp is the
// string-producing half; this header includes only core.hpp.
#include <cctype>
#include <iostream>
#include <istream>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>

#include <universal/number/interval/core.hpp>

namespace sw { namespace universal {

// stream output
template<typename Scalar>
inline std::ostream& operator<<(std::ostream& ostr, const interval<Scalar>& v) {
	return ostr << '[' << v._lo << ", " << v._hi << ']';
}

namespace interval_detail {
	// One bound of "[lo, hi]": the text up to the next ',' or ']'. inf, infinity, nan and nar, with
	// an optional sign and in any case, are what operator<< prints for unbounded and empty
	// intervals, and neither std::istream's operator>> for double (on libstdc++) nor posit's reads
	// all of them, so they map to Scalar's own values here; anything else is parsed by Scalar's
	// operator>>, which must take the whole token (CodeRabbit). inf maps only to a real infinity:
	// posit's infinity() is maxpos, and reading "inf" as maxpos would claim an unbounded interval
	// that is finite.
	template<typename Scalar>
	bool read_bound(std::istream& istr, Scalar& out) {
		std::string tok;
		istr >> std::ws;
		char ch;
		while (istr.get(ch)) {
			if (ch == ',' || ch == ']') { istr.unget(); break; }
			tok += ch;
		}
		while (!tok.empty() && std::isspace(static_cast<unsigned char>(tok.back()))) tok.pop_back();
		if (tok.empty()) return false;
		std::string t;
		for (char c : tok) t += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		const bool negative = (t[0] == '-');
		const std::string body = (t[0] == '-' || t[0] == '+') ? t.substr(1) : t;
		if (body == "inf" || body == "infinity") {
			if (!(std::numeric_limits<Scalar>::has_infinity && std::numeric_limits<Scalar>::infinity() > std::numeric_limits<Scalar>::max())) return false;
			out = negative ? Scalar(-std::numeric_limits<Scalar>::infinity()) : std::numeric_limits<Scalar>::infinity();
			return true;
		}
		if (body == "nan" || body == "nar") {
			if (!std::numeric_limits<Scalar>::has_quiet_NaN) return false;
			out = std::numeric_limits<Scalar>::quiet_NaN();
			return true;
		}
		std::istringstream in(tok);
		Scalar v{};
		in >> v;
		if (in.fail()) return false;
		in >> std::ws;
		if (!in.eof()) return false;   // trailing text inside the bound
		out = v;
		return true;
	}
}

// stream input: [lo, hi], the format operator<< writes, including unbounded bounds and the empty
// interval [nan, nan]. A malformed token, a wrong delimiter, lo > hi, or a single NaN bound sets
// failbit and leaves v unchanged; it used to overwrite v whatever was read (#1971).
template<typename Scalar>
inline std::istream& operator>>(std::istream& istr, interval<Scalar>& v) {
	Scalar lo{}, hi{};
	char open = 0, comma = 0, close = 0;
	bool ok = static_cast<bool>(istr >> open) && open == '[';
	ok = ok && interval_detail::read_bound(istr, lo);
	ok = ok && static_cast<bool>(istr >> comma) && comma == ',';
	ok = ok && interval_detail::read_bound(istr, hi);
	ok = ok && static_cast<bool>(istr >> close) && close == ']';
	if (ok) {
		// Scalar's own classification: posit's NaR compares equal to itself
		using std::isnan;
		const bool loNaN = isnan(lo), hiNaN = isnan(hi);
		ok = (loNaN == hiNaN) && !(hi < lo);   // both NaN is the empty interval; one NaN is not an interval
	}
	if (!ok) {
		istr.setstate(std::ios_base::failbit);
		return istr;
	}
	v._lo = lo;
	v._hi = hi;
	return istr;
}

}} // namespace sw::universal
