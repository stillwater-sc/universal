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
#include <vector>        // the decimal fraction digits of parse()
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
//           decimal         )    // decimal value, see below
//
//   decimal:  [0-9]* ( '.' [0-9]* )? ( [eE] [+-]? [0-9]+ )?   with at least one digit
//
// Bit-pattern parsing populates the underlying storage directly: the bits go
// into positions [nbits-1 .. 0], i.e., the most significant input character
// fills the high-order bits. This matches the convention used by setbits()
// and is what test programs typically want for "raw" initialization.
//
// A decimal is the value it names: parse("5") on fixpnt<8,4> yields 5.0 (raw bits 0101.0000),
// and parse("1.5"), "-0.125", "2.5e-1" and "1e2" work too (#1931). The fraction is rounded
// to nearest, ties to even, from the exact decimal value, never through a double, so the
// result is the correctly rounded fixpnt at any rbits. The integer part follows the
// arithmetic policy: Modulo wraps and Saturate clamps, to maxneg for a negative value.
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
		// split digits[.digits][e[+-]digits] into the decimal digits and the position of the
		// decimal point within them
		std::string digits;
		std::size_t i = 0;
		long long point = 0;   // number of digits before the decimal point
		while (i < body.size() && sp::is_decimal_digit(body[i])) { digits.push_back(body[i]); ++i; }
		point = static_cast<long long>(digits.size());
		if (i < body.size() && body[i] == '.') {
			++i;
			while (i < body.size() && sp::is_decimal_digit(body[i])) { digits.push_back(body[i]); ++i; }
		}
		if (digits.empty()) return false;
		if (i < body.size() && (body[i] == 'e' || body[i] == 'E')) {
			++i;
			bool eneg = false;
			if (i < body.size() && (body[i] == '+' || body[i] == '-')) { eneg = (body[i] == '-'); ++i; }
			long long exponent = 0;
			bool edigits = false;
			while (i < body.size() && sp::is_decimal_digit(body[i])) {
				if (exponent < 100000000) exponent = exponent * 10 + (body[i] - '0');   // the value is out of range far below that
				edigits = true;
				++i;
			}
			if (!edigits) return false;
			point += eneg ? -exponent : exponent;
		}
		if (i != body.size()) return false;
		// leading zeros carry no value
		{
			const std::size_t lz = digits.find_first_not_of('0');   // one erase, not one per zero
			if (lz == std::string::npos) digits.clear();
			else { digits.erase(0, lz); point -= static_cast<long long>(lz); }
		}
		if (digits.empty()) { value = tmp; return true; }   // zero

		// integer part: digits [0, point), padded with zeros when the exponent moves the point
		// past the last digit. It accumulates in the fixpnt itself, with the sign applied as it
		// goes, so that Modulo wraps and Saturate clamps the way the arithmetic does: -8 is
		// maxneg of fixpnt<8,4> rather than the negation of a saturated +8. Past the last digit
		// the padding only multiplies by ten, so it stops once that changes nothing: at zero,
		// where a Modulo value lands after at most nbits doublings, or at a saturated bound.
		const Fixpnt ten(10);
		const long long ndigits = static_cast<long long>(digits.size());
		const long long intDigits = (point > 0) ? point : 0;
		const Fixpnt zero(0), maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
		// where 10 itself does not fit, a saturating ten is maxpos: -1 * maxpos is -maxpos, not
		// the maxneg that -10 should clamp to. Any nonzero value times ten is out of range then.
		const bool tenSaturates = (arithmetic == Saturate) && !(double(ten) == 10.0);
		for (long long k = 0; k < intDigits; ++k) {
			if (k >= ndigits) {
				if (tmp == zero) break;
				if (arithmetic == Saturate && (tmp == maxpos || tmp == maxneg)) break;
			}
			if (tenSaturates && !(tmp == zero)) {
				tmp = negative ? maxneg : maxpos;
				break;
			}
			int d = (k < ndigits) ? (digits[static_cast<std::size_t>(k)] - '0') : 0;
			tmp *= ten;
			// add -d rather than subtract d: in fixpnt<8,4> 8 saturates to 7.9375, -8 does not
			tmp += Fixpnt(negative ? -d : d);
		}
		const bool intOdd = (intDigits > 0 && intDigits <= ndigits)
			? (((digits[static_cast<std::size_t>(intDigits - 1)] - '0') & 1) != 0)
			: false;   // padded with zeros (even) or no integer digits

		// fraction: digits [point, ndigits), with -point leading zeros when the point is to the
		// left of the first digit. Doubling the decimal fraction moves its first binary digit
		// into the carry, so rbits doublings give the fraction bits, and one more gives the
		// round bit; whatever is left is the sticky bit.
		std::vector<unsigned char> frac;
		{
			long long lead = (point < 0) ? -point : 0;
			// below 10^-(rbits+2) the value is under half an ulp: it rounds to zero, so a longer
			// run of leading zeros changes nothing
			if (lead > static_cast<long long>(rbits) + 2) lead = static_cast<long long>(rbits) + 2;
			for (long long k = 0; k < lead; ++k) frac.push_back(0);
			for (long long k = (point > 0 ? point : 0); k < ndigits; ++k) frac.push_back(static_cast<unsigned char>(digits[static_cast<std::size_t>(k)] - '0'));
			while (!frac.empty() && frac.back() == 0) frac.pop_back();
		}
		auto doubleFraction = [&frac]() {
			unsigned carry = 0;
			for (std::size_t k = frac.size(); k-- > 0; ) {
				unsigned v = 2u * frac[k] + carry;
				frac[k] = static_cast<unsigned char>(v % 10u);
				carry = v / 10u;
			}
			while (!frac.empty() && frac.back() == 0) frac.pop_back();
			return carry != 0;
		};
		Fixpnt fraction;
		fraction.clear();
		for (unsigned b = rbits; b-- > 0; ) {
			if (frac.empty()) break;
			if (doubleFraction()) fraction.setbit(b, true);
		}
		// ties go to even: the last bit kept is the fraction's lowest, or the integer part's
		// lowest when there are no fraction bits
		const bool lsb = (rbits > 0) ? fraction.at(0) : intOdd;
		bool round = false, sticky = false;
		if (!frac.empty()) {
			round  = doubleFraction();
			sticky = !frac.empty();
		}
		// The fraction holds a magnitude, which is not always a value of the signed type:
		// - with no integer bits (rbits == nbits) its top bit is the sign bit, so a magnitude of
		//   1/2 or more reads as negative. Under Saturate it is out of range either way: at or
		//   beyond the bound of its sign.
		// - rounding it up first can overflow it: in fixpnt<8,7> 0.999 rounds to 1, which
		//   saturates to 127/128 before the sign, and -0.999 came out -127/128 instead of -1.
		// So the clamp comes first, and the rounding ulp is applied after the sign (CodeRabbit
		// on PR #1937). Modulo keeps its wrap.
		if constexpr (arithmetic == Saturate && rbits == nbits) {
			if (fraction.at(nbits - 1)) {
				value = negative ? maxneg : maxpos;
				return true;
			}
		}
		const bool up = round && (sticky || lsb);
		Fixpnt ulp;
		ulp.clear();
		ulp.setbit(0, true);
		if (negative) {
			tmp -= fraction;
			if (up) tmp -= ulp;   // may carry into the integer bits: -0.99999 rounds to -1.0
		}
		else {
			tmp += fraction;
			if (up) tmp += ulp;
		}
		value = tmp;
		return true;
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
