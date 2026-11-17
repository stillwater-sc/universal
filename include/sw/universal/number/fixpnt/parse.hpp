#pragma once
// parse.hpp: text to fixpnt, with no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Part of layer 1 (core.hpp): fixpnt::assign(const std::string&) and operator>> both use parse(),
// so it lives with the arithmetic core rather than in iostream.hpp (#2111). It needs no I/O.
#include <cstddef>       // std::size_t
#include <string>
#include <string_view>
#include <vector>        // the decimal fraction digits
#include <universal/number/fixpnt/fixpnt_impl.hpp>

namespace sw { namespace universal {

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

}} // namespace sw::universal
