#pragma once
// iostream.hpp: stream insertion and extraction for fixpnt
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the fixpnt headers (#1334): the <iostream> half. manipulators.hpp is the
// <iomanip> half. parse() lives here with the stream operators because it is their only
// caller and it needs <regex> and <sstream>.
#include <iomanip>       // std::setw
#include <string_view>   // std::string_view
#include <cstddef>     // std::size_t
#include <cstdint>     // the fixed-width integer types
#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <map>

#include <universal/number/support/decimal.hpp>   // support::decimal, for convert_to_decimal_string
#include <universal/number/fixpnt/core.hpp>
#include <universal/number/fixpnt/manipulators.hpp>

namespace sw { namespace universal {

// convert fixpnt to decimal string, i.e. "-1234.5678"
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
std::string convert_to_decimal_string(const fixpnt<nbits, rbits, arithmetic, bt>& value) {
	std::stringstream str;
	if (value.iszero()) {
		str << '0';
		if constexpr (rbits > 0) {
			str << '.';
			for (unsigned i = 0; i < rbits; ++i) {
				str << '0';
			}
		}
		return str.str();
	}
	if (value.sign()) str << '-';
	support::decimal partial, multiplier;
	fixpnt<nbits, rbits, arithmetic, bt> number;
	number = value.sign() ? sw::universal::twosComplement(value) : value;
	if constexpr (nbits > rbits) {
		// convert the fixed point: start by handling the integer part
		multiplier.setdigit(1);
		// convert fixpnt to decimal by adding and doubling multipliers
		for (unsigned i = rbits; i < nbits; ++i) {
			if (number.at(i)) {
				support::add(partial, multiplier);
			}
			support::add(multiplier, multiplier);
		}
		for (support::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
			str << (int)*rit;
		}
	}
	else {
		str << '0';
	}

	if constexpr (rbits > 0) {
		str << ".";
		// and secondly, the fraction part
		support::decimal range, discretizationLevels, step;
		// create the decimal range we are discretizing
		range.setdigit(1);
		range.shiftLeft(rbits);
		// calculate the discretization levels of this range
		discretizationLevels.setdigit(1);
		for (unsigned i = 0; i < rbits; ++i) {
			support::add(discretizationLevels, discretizationLevels);
		}
		step = div(range, discretizationLevels);
		// now construct the value of this range by adding the fraction samples
		partial.setzero();
		multiplier.setdigit(1);
		// convert the fraction part
		for (unsigned i = 0; i < rbits; ++i) {
			if (number.at(i)) {
				support::add(partial, multiplier);
			}
			support::add(multiplier, multiplier);
		}
		support::mul(partial, step);
		// leading 0s will cause the partial to be represented incorrectly
		// if we simply convert it to digits.
		// The partial represents the parts in the range, so we can deduce
		// the number of leading zeros by comparing to the length of range
		unsigned nrLeadingZeros = static_cast<unsigned>(range.size() - partial.size() - 1);
		for (unsigned i = 0; i < nrLeadingZeros; ++i) str << '0';
		unsigned digitsWritten = nrLeadingZeros;
		for (support::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
			str << (int)*rit;
			++digitsWritten;
		}
		if (digitsWritten < rbits) { // deal with trailing 0s
			for (unsigned i = digitsWritten; i < rbits; ++i) {
				str << '0';
			}
		}
	}
	return str.str();
}

//////////////////////////////////////////////////////////////////////////////////////////////
/// stream operators

// ostream output generates an ASCII format for the fixed-point argument
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::ostream& operator<<(std::ostream& ostr, const fixpnt<nbits, rbits, arithmetic, bt>& i) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the fixpnt into a string
	std::stringstream ss;

	std::streamsize prec = ostr.precision();
	std::streamsize width = ostr.width();
	std::ios_base::fmtflags ff;
	ff = ostr.flags();
	ss.flags(ff);
	ss << std::setw(width) << std::setprecision(prec) << convert_to_decimal_string(i);

	return ostr << ss.str();
}

// Parse an ASCII string into a fixpnt value.
//
// Accepted syntax (Phase B1 of issue #835):
//
//   [+-]? ( 0[bB][01]+      |    // binary bit-pattern, fills _block MSB-first
//           0[oO][0-7]+     |    // octal bit-pattern
//           0[xX][0-9A-F']+ |    // hex bit-pattern (apostrophe allowed as separator)
//           [0-9]+          )    // decimal integer; mapped to a fixpnt by shifting left by rbits
//
// Bit-pattern parsing populates the underlying storage directly: the bits go
// into positions [nbits-1 .. 0], i.e., the most significant input character
// fills the high-order bits. This matches the convention used by setbits()
// and is what test programs typically want for "raw" initialization.
//
// Decimal parsing is integer-only in this phase. The accumulated integer K is
// converted to fixpnt as (K << rbits), i.e., parse("5") on fixpnt<8,4> yields
// the value 5.0 (raw bits 0101.0000). Decimal-fraction parsing ("3.14") is
// deferred to Phase B2 alongside posit/cfloat float-from-string.
//
// All scanning is delegated to the constexpr primitives in
// `<universal/utility/string_parse.hpp>`.
//
// Returns true on successful parse, false on malformed input.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
bool parse(const std::string& number, fixpnt<nbits, rbits, arithmetic, bt>& value) {
	namespace sp = sw::universal::string_parse;
	using Fixpnt = fixpnt<nbits, rbits, arithmetic, bt>;

	// Build into a local temporary and only commit to `value` on a fully
	// successful parse. This keeps malformed input from leaving the caller's
	// object in a partially-mutated state.
	Fixpnt tmp;
	tmp.clear();

	std::string_view s{number};

	auto sg = sp::scan_sign(s);
	const bool negative = sg.negative;
	s = sg.rest;
	if (s.empty()) return false;

	auto pfx = sp::scan_prefix(s);
	std::string_view body = pfx.body;
	if (body.empty()) return false;

	// Bit-pattern branches track digit_found so payloads of only separators or
	// (theoretically) empty strings are rejected rather than silently yielding zero.
	bool digit_found = false;

	switch (pfx.base) {
	case sp::number_base::binary: {
		for (char c : body) {
			if (!sp::is_binary_digit(c)) return false;
			tmp <<= 1;
			if (c == '1') tmp.setbit(0, true);
			digit_found = true;
		}
		if (!digit_found) return false;
		break;
	}
	case sp::number_base::octal: {
		for (char c : body) {
			if (!sp::is_octal_digit(c)) return false;
			tmp <<= 3;
			unsigned digit = static_cast<unsigned>(c - '0');
			for (unsigned b = 0; b < 3; ++b) {
				if ((digit >> b) & 1u) tmp.setbit(b, true);
			}
			digit_found = true;
		}
		if (!digit_found) return false;
		break;
	}
	case sp::number_base::hex: {
		for (char c : body) {
			if (c == '\'') continue;
			if (!sp::is_hex_digit(c)) return false;
			tmp <<= 4;
			unsigned digit = sp::hex_digit_value(c);
			for (unsigned b = 0; b < 4; ++b) {
				if ((digit >> b) & 1u) tmp.setbit(b, true);
			}
			digit_found = true;
		}
		if (!digit_found) return false;
		break;
	}
	case sp::number_base::decimal: {
		// Each digit multiplies the accumulator by 10 in the value domain
		// and adds the digit (as an integer value). We use the fixpnt
		// converting constructor from native int so Saturate / Modulo
		// policy is respected, and so we never shift by rbits at runtime
		// (which would be UB for instantiations with rbits >= 64).
		const Fixpnt ten(10);
		for (char c : body) {
			if (!sp::is_decimal_digit(c)) return false;
			tmp *= ten;
			tmp += Fixpnt(static_cast<int>(c - '0'));
		}
		break;
	}
	default:
		return false;
	}

	if (negative) tmp = -tmp;
	value = tmp;
	return true;
}

// istream input reads an ASCII format and assigns value to the fixed-point argument.
// On parse failure: log a diagnostic to std::cerr AND set failbit on the stream
// so callers (loops with `while (in >> x)`, etc.) can detect the error without
// scraping stderr. Both are useful: the message helps interactive debugging,
// the failbit enables programmatic detection.
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::istream& operator>>(std::istream& istr, fixpnt<nbits, rbits, arithmetic, bt>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a fixpnt value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// string converters

// to_binary generates a binary presentation of the fixed-point number
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string to_binary(const fixpnt<nbits, rbits, arithmetic, bt>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	if constexpr (nbits > rbits) {
		for (int i = static_cast<int>(nbits) - 1; i >= static_cast<int>(rbits); --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && (i - rbits) > 0 && (i - rbits) % 4 == 0) s << '\'';
		}
	}
	else {
		s << '0';
	}
	s << '.';
	if constexpr (rbits > 0) {
		for (int i = int(rbits) - 1; i >= 0; --i) {
			s << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && (rbits - i) % 4 == 0 && i != 0) s << '\'';
		}
	}
	return s.str();
}

// native semantic representation: radix-2, delegates to to_binary
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string to_native(const fixpnt<nbits, rbits, arithmetic, bt>& number, bool nibbleMarker = false) {
	return to_binary(number, nibbleMarker);
}

// to_triple generates a triple (sign,scale,fraction) representation of the fixed-point number
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
inline std::string to_triple(const fixpnt<nbits, rbits, arithmetic, bt>& number) {
	std::stringstream ss;
	ss << (number.sign() ? "(-," : "(+,");
	ss << scale(number) << ',';
	for (int i = static_cast<int>(rbits) - 1; i >= 0; --i) {
		ss << (number.at(static_cast<unsigned>(i)) ? '1' : '0');
	}
	ss << (rbits == 0 ? "~)" : ")");
	return ss.str();
}
}} // namespace sw::universal
