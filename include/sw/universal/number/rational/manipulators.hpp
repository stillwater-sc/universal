#pragma once
// manipulators.hpp: definition of manipulation functions for rational types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the rational headers (#1334): the string-producing half. These build a
// std::string and hand it back, so they need <sstream> but not <iostream>; the stream
// insertion and extraction operators are in iostream.hpp.
//
// This header had NO includes at all while using std::string, std::stringstream and the
// stream base-format manipulators -- it compiled only behind a prior include.
#include <string>
#include <sstream>
#include <ios>          // std::oct / std::dec / std::hex, used by to_binary(blockdigit)
#include <cstddef>      // std::size_t, the parse() cursor
#include <cstdint>      // int64_t, the to_binary(blockdigit) fallback
#include <type_traits>  // std::is_same_v, used by type_tag

#include <universal/native/integers.hpp>                   // to_binary(Integer, bool), the base-2 fallback
#include <universal/internal/blockbinary/manipulators.hpp>   // to_binary/to_hex on blockbinary,
                                                             // moved off the arithmetic core (#1334)
#include <universal/number/rational/core.hpp>

namespace sw { namespace universal {

	// Generate a type tag for rational type
	template<unsigned nbits, typename Base, typename bt>
	std::string type_tag(const rational<nbits,Base,bt>& = {}) {
		std::stringstream s;
		if constexpr (std::is_same_v<Base, base2>) {
			s << "rational<" << nbits << ", base2, " << type_tag(bt()) << '>';
		}
		else if constexpr (std::is_same_v<Base, base8>) {
			s << "rational<" << nbits << ", base8>";
		}
		else if constexpr (std::is_same_v<Base, base10>) {
			s << "rational<" << nbits << ", base10>";
		}
		else if constexpr (std::is_same_v<Base, base16>) {
			s << "rational<" << nbits << ", base16>";
		}
		else {
			s << "rational<" << nbits << ", unknown_base>";
		}
		return s.str();
	}

template<unsigned nbits, unsigned base, typename bt>
inline std::string to_binary(const blockdigit<nbits, base, bt>& v, bool nibbleMarker = true) {
	if constexpr (base == 8) {
		std::stringstream s;
		for (unsigned i = 0; i < blockdigit<nbits, 8, bt>::ndigits; ++i) {
			if (nibbleMarker && (i > 0) && (i % 2 == 0)) s << '\'';
			s << std::oct << static_cast<unsigned>(v.digit(i));
		}
		return s.str();
	} else if constexpr (base == 10) {
		std::stringstream s;
		for (unsigned i = 0; i < blockdigit<nbits, 10, bt>::ndigits; ++i) {
			if (nibbleMarker && (i > 0) && (i % 2 == 0)) s << '\'';
			s << std::dec << static_cast<unsigned>(v.digit(i));
		}
		return s.str();
	} else if constexpr (base == 16) {
		std::stringstream s;
		for (unsigned i = 0; i < blockdigit<nbits, 16, bt>::ndigits; ++i) {
			if (nibbleMarker && (i > 0) && (i % 2 == 0)) s << '\'';
			s << std::hex << static_cast<unsigned>(v.digit(i));
		}
		return s.str();
	}
	else {
		return to_binary(static_cast<int64_t>(v), nibbleMarker);
	}
}

template<unsigned nbits, typename Base, typename bt>
inline std::string to_binary(const rational<nbits,Base,bt>& v, bool nibbleMarker = true) {
	std::stringstream s;
	s << to_binary(v.numerator(), nibbleMarker)
		<< " / "
		<< to_binary(v.denominator(), nibbleMarker);
	return s.str();
}

// native semantic representation: rational as N/D in decimal
template<unsigned nbits, typename Base, typename bt>
inline std::string to_native(const rational<nbits,Base,bt>& v, bool = false) {
	std::stringstream s;
	s << v.numerator() << " / " << v.denominator();
	return s.str();
}

///////////////////////////////////////////////////////////////////////////////////////////////
// parse: text to rational (#1929)
//
// Accepted forms, with an optional leading sign:
//   integer               12, -7
//   fraction  n/d         3/4, -22/7        (unsigned integers on both sides, d != 0)
//   decimal              1.5, -0.125, .5, 2.
//   with an exponent      2.5e-3, 1E6
// The value is exact or the parse fails: a value the rational cannot hold, a malformed token or a
// zero denominator returns false and leaves v unchanged. The digits are read as an integer before
// the powers of ten are divided out, so that integer must fit as well: 1.5 needs 15.
// The rational arithmetic wraps on overflow, so every step is checked by undoing it.
// The parse uses only the public arithmetic, so it serves every Base.

namespace rational_parse {

// The parse accumulates magnitudes, so r >= 0 throughout and the sign is applied at the end.

// r = r * 10 + digit, false when the result does not fit
template<typename Rational>
bool append_digit(Rational& r, int digit, const Rational& ten, bool tenFits) {
	const Rational dg(digit);
	if (double(dg) != double(digit)) return false;    // too narrow to hold the digit itself
	const Rational zero(0);
	if (r == zero) { r = dg; return true; }           // leading digit: nothing to scale
	if (!tenFits) return false;
	Rational next = r * ten + dg;
	// a wrap that lands negative fools the undo, since the subtraction wraps back: in 8 bits
	// 12*10 + 8 is -128, and (-128 - 8) is 120 again, so check the sign first
	if (next < zero) return false;
	if (!((next - dg) / ten == r)) return false;      // wrapped
	r = next;
	return true;
}

// r = r * 10, false when the result does not fit
template<typename Rational>
bool scale_by_ten(Rational& r, const Rational& ten, bool tenFits) {
	if (!tenFits) return false;
	Rational next = r * ten;
	if (next < Rational(0)) return false;             // wrapped negative
	if (!(next / ten == r)) return false;             // wrapped
	r = next;
	return true;
}

// r = r / 10, false when the result does not fit. One power at a time, so that the value reduces
// as it goes: 125 / 10^3 is 25/2, 5/4, 1/8, and fits where 125/1000 would not.
// The check cannot multiply back: 25/2 * 10 needs 250 before it reduces, which does not fit where
// 25/2 does. It compares in double instead. A wrapped denominator changes the value by at least
// the modulus, a relative error of 100% or more, so the tolerance only absorbs double rounding.
template<typename Rational>
bool divide_by_ten(Rational& r, const Rational& ten, bool tenFits) {
	if (!tenFits) return false;
	Rational next = r / ten;
	const double expected = double(r) / 10.0;
	const double got      = double(next);
	const double diff     = (got > expected) ? got - expected : expected - got;
	const double mag      = (expected < 0.0) ? -expected : expected;
	if (!(diff <= mag * 1.0e-12)) return false;      // wrapped
	r = next;
	return true;
}

constexpr bool is_digit(char c) noexcept { return c >= '0' && c <= '9'; }

} // namespace rational_parse

template<unsigned nbits, typename Base, typename bt>
bool parse(const std::string& txt, rational<nbits, Base, bt>& v) {
	namespace rp = rational_parse;
	using Rational = rational<nbits, Base, bt>;
	const Rational ten(10);
	const bool tenFits = (double(ten) == 10.0);

	const std::size_t n = txt.size();
	std::size_t i = 0;
	bool negative = false;
	if (i < n && (txt[i] == '+' || txt[i] == '-')) { negative = (txt[i] == '-'); ++i; }

	Rational num(0), den(1);
	bool digits = false;
	while (i < n && rp::is_digit(txt[i])) {
		if (!rp::append_digit(num, txt[i] - '0', ten, tenFits)) return false;
		digits = true;
		++i;
	}

	if (i < n && txt[i] == '/') {
		// n/d
		if (!digits) return false;
		++i;
		Rational d(0);
		bool ddigits = false;
		while (i < n && rp::is_digit(txt[i])) {
			if (!rp::append_digit(d, txt[i] - '0', ten, tenFits)) return false;
			ddigits = true;
			++i;
		}
		if (!ddigits || i != n || d == Rational(0)) return false;
		den = d;
	}
	else {
		// decimal: the fraction digits extend the numerator, and each one is a power of ten to
		// divide by. Trailing zeros change nothing, so they are not appended: 1.50 fits wherever
		// 1.5 does.
		long long fractionDigits = 0;
		if (i < n && txt[i] == '.') {
			++i;
			std::size_t end = i;
			while (end < n && rp::is_digit(txt[end])) ++end;
			std::size_t last = end;
			while (last > i && txt[last - 1] == '0') --last;
			for (std::size_t k = i; k < last; ++k) {
				if (!rp::append_digit(num, txt[k] - '0', ten, tenFits)) return false;
				++fractionDigits;
			}
			if (end > i) digits = true;
			i = end;
		}
		if (!digits) return false;
		long long exponent = 0;
		if (i < n && (txt[i] == 'e' || txt[i] == 'E')) {
			++i;
			bool eneg = false;
			if (i < n && (txt[i] == '+' || txt[i] == '-')) { eneg = (txt[i] == '-'); ++i; }
			bool edigits = false;
			while (i < n && rp::is_digit(txt[i])) {
				if (exponent < 1000000) exponent = exponent * 10 + (txt[i] - '0');  // beyond that it cannot fit anyway
				edigits = true;
				++i;
			}
			if (!edigits) return false;
			if (eneg) exponent = -exponent;
		}
		if (i != n) return false;
		// value = num * 10^(exponent - fractionDigits); zero stays zero at any exponent
		if (!(num == Rational(0))) {
			long long power = exponent - fractionDigits;
			for (; power > 0; --power) if (!rp::scale_by_ten(num, ten, tenFits)) return false;
			for (; power < 0; ++power) if (!rp::divide_by_ten(num, ten, tenFits)) return false;
		}
	}

	Rational result = num / den;
	v = negative ? -result : result;
	return true;
}

}} // namespace sw::universal
