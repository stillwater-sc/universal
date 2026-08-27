#pragma once
// manipulators.hpp: definitions of helper functions for classic cfloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <regex>       // parse() below
#include <universal/number/support/decimal.hpp>   // support::decimal, for to_decimal_fixpnt_string
#include <universal/internal/blocktriple/manipulators.hpp>   // to_triple(blocktriple), used by to_triple(cfloat)
#include <universal/number/cfloat/core.hpp>
#include <string>        // std::string
#include <sstream>       // std::stringstream
#include <cstddef>       // std::size_t
#include <cstdint>       // the fixed-width integer types
#include <iostream>
#include <iomanip>
#include <typeinfo>  // for typeid()
#include <universal/number/cfloat/cfloat_fwd.hpp>
// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a cfloat type
// using cfloat number system knowledge.

namespace sw { namespace universal {

// Moved out of cfloat_impl.hpp (#1334): these turn a cfloat into a std::string, and
// support/decimal.hpp -- 80,048 lines and four I/O-family headers -- is here only
// because to_decimal_fixpnt_string is its single user in all of cfloat.
///////////////////////////// IOSTREAM operators ///////////////////////////////////////////////

// convert cfloat to decimal fixpnt string, i.e. "-1234.5678"
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
std::string to_decimal_fixpnt_string(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& value, long long precision) {
	constexpr unsigned fbits = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>::fbits;
	constexpr unsigned bias = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>::EXP_BIAS;
	std::stringstream str;
	if (value.iszero()) {
		str << '0';
		return str.str();
	}
	if (value.sign()) str << '-';

	// construct the discretization levels of the fraction part
	support::decimal range, discretizationLevels, step;
	// create the decimal range we are discretizing
	range.setdigit(1);
	range.shiftLeft(fbits); // the decimal range of the fraction
	discretizationLevels.powerOf2(fbits); // calculate the discretization levels of this range
	step = div(range, discretizationLevels);
	// now construct the value of this range by adding the fraction samples
	support::decimal partial, multiplier;
	partial.setzero();  // if you just want the fraction
	multiplier.setdigit(1);
	// convert the fraction part
	for (unsigned i = 0; i < fbits; ++i) {
		if (value.at(i)) {
			support::add(partial, multiplier);
		}
		support::add(multiplier, multiplier);
	}
	if (value.isdenormal()) {
		support::mul(partial, step);
		support::decimal scale;
		scale.powerOf2(bias - 1ull);
		partial = support::div(partial, scale);
	} 
	else {
		support::add(partial, multiplier); // add the hidden bit
		support::mul(partial, step);
		support::decimal scale;
		int exponent = value.scale();
		if (exponent < 0) {
			scale.powerOf2(static_cast<unsigned>(-exponent));
			partial = support::div(partial, scale);
		}
		else {
			scale.powerOf2(static_cast<unsigned>(exponent));
			support::mul(partial, scale);
		}
	}

	// the radix is at fbits
	// The partial represents the parts in the range, so we can deduce
	// the number of leading zeros by comparing to the length of range
	int nrLeadingZeros = static_cast<int>(range.size()) - static_cast<int>(partial.size()) - 1;
	if (nrLeadingZeros >= 0) str << "0.";
	for (int i = 0; i < nrLeadingZeros; ++i) str << '0';
	int digitsWritten = (nrLeadingZeros > 0) ? nrLeadingZeros : 0;
	int position = static_cast<int>(partial.size()) - 1;
	for (support::decimal::const_reverse_iterator rit = partial.rbegin(); rit != partial.rend(); ++rit) {
		str << (int)*rit;
		++digitsWritten;
		if (position == fbits) str << '.';
		--position;
	}
	if (digitsWritten < precision) { // deal with trailing 0s
		for (unsigned i = static_cast<unsigned>(digitsWritten); i < fbits; ++i) {
			str << '0';
		}
	}

	return str.str();
}

// NOTE: the legacy `to_string(const cfloat&, long long precision)` overload was
// removed (#1282). It built the value with an integer-only `support::decimal`
// and was lossy for every value with scale < fbits (e.g. 1.5 rendered as 48),
// and a negative lsbScale would spin `powerOf2` ~2^64 times. It had no callers;
// use operator<< (exact binary-to-decimal via blocktriple::to_string) instead.


//////////////////////////////////////////////////////////////////////////////////////////////
/// stream operators

// ostream output generates an ASCII format for the floating-point argument
// Uses native binary-to-decimal conversion via blocktriple::to_string()
// to produce exact output for all cfloat sizes without double conversion.

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
bool parse(const std::string& txt, cfloat<nbits,es,bt,hasSubnormals,hasMaxExpValues,isSaturating>& v) {
	// check if the txt is of the native cfloat form: nbits.esX[0x]hexvaluec
	std::regex cfloat_regex(R"(^[0-9]+\.[0-9]+[xX](0[xX])?[0-9A-Fa-f]+c?$)");
	if (std::regex_match(txt, cfloat_regex)) {
		// found a cfloat representation: parse nbits.esxHEXVALUEc
		std::string nbitsStr, esStr, bitStr;
		auto it = txt.begin();
		for (; it != txt.end(); ++it) {
			if (*it == '.') break;
			nbitsStr.append(1, *it);
		}
		for (++it; it != txt.end(); ++it) {
			if (*it == 'x' || *it == 'X') break;
			esStr.append(1, *it);
		}
		for (++it; it != txt.end(); ++it) {
			if (*it == 'c') break;
			bitStr.append(1, *it);
		}
		unsigned nbits_in = 0;
		unsigned es_in = 0;
		{
			std::istringstream ss(nbitsStr);
			ss >> nbits_in;
			if (ss.fail()) return false;
		}
		{
			std::istringstream ss(esStr);
			ss >> es_in;
			if (ss.fail()) return false;
		}
		// native cfloat form must match target configuration
		if (nbits_in != nbits || es_in != es) return false;
		uint64_t raw = 0;
		std::istringstream ss(bitStr);
		ss >> std::hex >> raw;
		if (ss.fail()) return false;
		ss >> std::ws;
		if (!ss.eof()) return false;
		v.setbits(raw);
		return true;
	}
	else {
		// Decimal floating-point representation.
		// Route through the high-precision decimal_to_binary utility so that
		// wide cfloat configurations (nbits > 64, including IEEE quad and
		// posit-killer formats) don't lose precision through an intermediate
		// double. The utility delivers a normalized mantissa with
		// target_mantissa_bits bits plus guard/sticky; we package that as a
		// blocktriple and hand it to convert(blocktriple, cfloat), which
		// handles all IEEE-754 edge cases (subnormals, saturation, etc.).
		// Special-value tokens (nan / inf in any common spelling) are
		// recognised directly so callers don't depend on locale-sensitive
		// std::istringstream parsing of those literals.
		using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;
		{
			std::string t; t.reserve(txt.size());
			for (char c : txt) t.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
			bool negative = !t.empty() && t.front() == '-';
			std::string body = t;
			if (!body.empty() && (body.front() == '+' || body.front() == '-')) body.erase(0, 1);
			if (body == "nan") {
				v.setnan(NAN_TYPE_QUIET);
				return true;
			}
			if (body == "inf" || body == "infinity") {
				v.setinf(negative);
				return true;
			}
		}
		// We pack the d2b result into a blocktriple<fbits, MUL, bt>. The MUL
		// layout has bfbits = 2*fbits + 2 and radix = 2*fbits, which gives us
		// exactly fbits of headroom below the cfloat fraction for guard/
		// sticky -- enough room for cfloat's convert() to make a correct
		// round-to-nearest-even decision. (The REP layout has no such
		// headroom: its fraction is exactly cfloat::fbits wide.)
		using BT = blocktriple<Cfloat::fbits, BlockTripleOperator::MUL, bt>;
		constexpr unsigned radix_pos          = static_cast<unsigned>(BT::radix);
		constexpr unsigned target_mantissa_bits = radix_pos + 1u;
		auto d = ::sw::universal::decimal_to_binary::convert(
			std::string_view{txt}, target_mantissa_bits);
		if (!d.valid) return false;
		if (d.is_zero) {
			v.setzero();
			v.setsign(d.negative);
			return true;
		}
		BT bt_val;
		bt_val.setnormal();
		bt_val.setsign(d.negative);
		bt_val.setscale(static_cast<int>(d.binary_scale));
		// d2b mantissa has its MSB at position radix_pos (the hidden bit);
		// below it are radix_pos extra precision bits. Copy bit-for-bit
		// into the blocktriple significand aligned to the same radix.
		for (unsigned i = 0; i <= radix_pos; ++i) {
			if (d.mantissa.at(i)) bt_val.setbit(i, true);
		}
		// Fold d2b's residual guard/sticky into the lowest bit so cfloat's
		// own rounding decision picks them up as sticky tail.
		if (d.guard_bit || d.sticky_bit) bt_val.setbit(0, true);
		convert(bt_val, v);
		return true;
	}
}

// read an ASCII float or cfloat format: nbits.esxNN...NNc, for example: 16.5x7C00c

// transform cfloat to a binary representation
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline std::string to_binary(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& number, bool nibbleMarker = false) {
	std::stringstream s;
	s << "0b";
	unsigned index = nbits;
	s << (number.at(--index) ? '1' : '0') << '.';

	for (int i = int(es) - 1; i >= 0; --i) {
		s << (number.at(--index) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}

	s << '.';

	constexpr int fbits = nbits - 1ull - es;
	for (int i = fbits - 1; i >= 0; --i) {
		s << (number.at(--index) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << '\'';
	}

	return s.str();
}

// native semantic representation: radix-2, delegates to to_binary
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline std::string to_native(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& number, bool nibbleMarker = false) {
	return to_binary(number, nibbleMarker);
}

// transform a cfloat into a triple representation
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
inline std::string to_triple(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& number, bool nibbleMarker = true) {
	std::stringstream s;
	blocktriple<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>::fbits, BlockTripleOperator::REP, bt> triple;
	number.normalize(triple);
	s << to_triple(triple, nibbleMarker);
	return s.str();
}


// Generate a type tag for this cfloat, for example, cfloat<8,1, unsigned char, hasSubnormals, noSupernormals, notSaturating>
//template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
//std::string type_tag(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> & = {}) {}

// generate a type_tag for a cfloat
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true>
inline std::string type_tag([[maybe_unused]] CfloatType v = {}) {
	constexpr unsigned nbits = CfloatType::nbits;
	constexpr unsigned es = CfloatType::es;
	using bt = typename CfloatType::BlockType;
	constexpr bool hasSubnormals = CfloatType::hasSubnormals;
	constexpr bool hasMaxExpValues = CfloatType::hasMaxExpValues;
	constexpr bool isSaturating = CfloatType::isSaturating;
	std::stringstream s;
	if constexpr (nbits == 128 && es == 15 && hasSubnormals && !hasMaxExpValues && !isSaturating) {
		s << "fp128 (IEEE-754 quad)";
	}
	else if constexpr (nbits == 64 && es == 11 && hasSubnormals && !hasMaxExpValues && !isSaturating) {
		s << "fp64 (IEEE-754 binary64)";
	}
	else if constexpr (nbits == 32 && es == 8 && hasSubnormals && !hasMaxExpValues && !isSaturating) {
		s << "fp32 (IEEE-754 binary32)";
	}
	else if constexpr (nbits == 16 && es == 8 && hasSubnormals && !hasMaxExpValues && !isSaturating) {
		s << "bf16 (Google Brain float)";
	}
	else if constexpr (nbits == 16 && es == 5 && hasSubnormals && !hasMaxExpValues && !isSaturating) {
		s << "fp16 (IEEE-754 binary16)";
	}
	else if constexpr (nbits == 8 && es == 2 && hasSubnormals && !hasMaxExpValues && !isSaturating) {
		s << "fp8 (IEEE-754 quarter)";
	}
	else if constexpr (nbits == 8 && es == 2 && hasSubnormals && hasMaxExpValues && !isSaturating) {
		s << "fp8e2m5 (DL 8-bit e2m5)";
	}
	else if constexpr (nbits == 8 && es == 3 && hasSubnormals && hasMaxExpValues && !isSaturating) {
		s << "fp8e3m4 (DL 8-bit e3m4)";
	}
	else if constexpr (nbits == 8 && es == 4 && hasSubnormals && hasMaxExpValues && !isSaturating) {
		s << "fp8e4m3 (OFP 8-bit e4m3)";
	}
	else if constexpr (nbits == 8 && es == 5 && hasSubnormals && hasMaxExpValues && !isSaturating) {
		s << "fp8e5m2 (OFP 8-bit e5m2)";
	}
	else {
		s << "cfloat<"
			<< std::setw(3) << nbits << ", "
			<< std::setw(3) << es << ", "
			<< type_tag(bt()) << ", "
			<< (hasSubnormals ? "hasSubnormals, " : " noSubnormals, ")
			<< (hasMaxExpValues ? "hasMaxExpValues, " : " noMaxExpValues, ")
			<< (isSaturating ? "   Saturating>" : "notSaturating>");
	}
	return s.str();
}

// Generate a type field descriptor for this cfloat
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string type_field(const CfloatType & = {}) {
	std::stringstream s;
//	typename CfloatType::BlockType bt{0};
//	unsigned nbits = CfloatType::nbits;  // total bits
	unsigned ebits = CfloatType::es;     // exponent bits
	unsigned fbits = CfloatType::fbits;  // integer bits
	s << "fields(s:1|e:" << ebits << "|m:" << fbits << ')';
	return s.str();
}

// generate and tabulate subnormals of a cfloat configuration
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline void subnormals() {
	constexpr unsigned nbits       = CfloatType::nbits;
	constexpr unsigned es          = CfloatType::es;
	using bt                       = typename CfloatType::BlockType;
	constexpr bool hasSubnormals   = CfloatType::hasSubnormals;
	constexpr bool hasMaxExpValues = CfloatType::hasMaxExpValues;
	constexpr bool isSaturating    = CfloatType::isSaturating;
	cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> a{ 0 };

	// generate the smallest subnormal with ULP set
	++a;
	if constexpr (hasSubnormals) {
		constexpr unsigned fbits = CfloatType::fbits;
		std::cout << type_tag(a) << " subnormals\n";
		if constexpr (nbits < 65u) {
			for (size_t i = 0; i < fbits; ++i) {
				std::cout << to_binary(a, true) << " : " << color_print(a) << " : " << a << '\n';
				uint64_t fraction = a.fraction_ull();
				fraction <<= 1;
				a.setfraction(fraction);
			}
		}
		else {
#ifdef DEPRECATED
			blockbinary<fbits, bt> fraction{ 0 };
			for (size_t i = 0; i < fbits; ++i) {
				std::cout << to_binary(a, true) << " : " << color_print(a) << " : " << a << '\n';
				a.fraction(fraction);
				fraction <<= 1;
				a.setfraction(fraction);
			}
#endif
			std::cerr << "big cfloat subnormals TBD\n";
		}
	}
	else {
		std::cout << type_tag(a) << " has no subnormals\n";
	}
}

// Generate a string representing the cfloat components: sign, scale, significand
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string components(const CfloatType& v) {
	constexpr unsigned cfbits = CfloatType::fbits;
	using bt = typename CfloatType::BlockType;
	std::stringstream s;
	s << "sign: " << (v.sign() ? '-' : '+');
	if (v.isnan()) {
		s << ", nan";
	}
	else if (v.isinf()) {
		s << ", inf";
	}
	else if (v.iszero()) {
		s << ", zero";
	}
	else {
		blocktriple<cfbits, BlockTripleOperator::REP, bt> a;
		v.normalize(a);
		int _scale = a.scale();
		// compute significand from blocktriple: significand = value / 2^scale
		// use the native to_string in fixed mode to get the significand digits
		int sigDigits = static_cast<int>(cfbits) / 3 + 2;
		if (sigDigits < 6) sigDigits = 6;
		blocktriple<cfbits, BlockTripleOperator::REP, bt> sig(a);
		sig.setscale(0); // normalize to [1,2) for normals, [0,1) for subnormals
		sig.setsign(false);
		std::string sigStr = sig.to_string(sigDigits, 0, false, false, false, false, false, false, ' ');
		s << ", scale: " << _scale
		  << ", significand: " << sigStr;
		if (v.isdenormal()) s << " (subnormal)";
	}
	return s.str();
}

// generate a binary string for cfloat
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string to_hex(const CfloatType& v, bool nibbleMarker = false, bool hexPrefix = true) {
	constexpr unsigned nbits = CfloatType::nbits;
	constexpr char hexChar[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
	};
	std::stringstream s;
	if (hexPrefix) s << "0x" << std::hex;
	int nrNibbles = int(1ull + ((nbits - 1ull) >> 2ull));
	for (int n = nrNibbles - 1; n >= 0; --n) {
		uint8_t nibble = v.nibble(unsigned(n));
		s << hexChar[nibble];
		if (nibbleMarker && n > 0 && (n % 4) == 0) s << '\'';
	}
	return s.str();
}

// generate a cfloat format ASCII hex format nbits.esxNN...NNa
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string hex_print(const CfloatType& c) {
	constexpr unsigned nbits = CfloatType::nbits;
	constexpr unsigned es    = CfloatType::es;
	std::stringstream s;
	s << nbits << '.' << es << 'x' << to_hex(c) << 'c';
	return s.str();
}

template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string pretty_print(const CfloatType& r) {
	constexpr unsigned es     = CfloatType::es;
	constexpr unsigned fhbits = CfloatType::fhbits;
	using bt = typename CfloatType::BlockType;
	bool sign{ false };
	blockbinary<es, bt> e;
	blockbinary<fhbits, bt> f;
	decode(r, sign, e, f);

	std::stringstream s;
	// sign bit
	s << (sign ? '1' : '0');

	// exponent bits
	s << ':';
	for (int i = int(es) - 1; i >= 0; --i) {
		s << (e.test(static_cast<size_t>(i)) ? '1' : '0');
	}

	// fraction bits
	s << ':';
	for (int i = int(r.fbits) - 1; i >= 0; --i) {
		s << (f.test(static_cast<size_t>(i)) ? '1' : '0');
	}

	return s.str();
}

template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string info_print(const CfloatType& p, int printPrecision = 17) {
	return std::string("TBD");
}

// generate a binary, color-coded representation of the cfloat
template<typename CfloatType,
	std::enable_if_t< is_cfloat<CfloatType>, bool> = true
>
inline std::string color_print(const CfloatType& r, bool nibbleMarker = false) {
	constexpr unsigned es     = CfloatType::es;
	constexpr unsigned fhbits = CfloatType::fhbits;
	using bt = typename CfloatType::BlockType;
	bool sign{ false };
	blockbinary<es,bt> e;
	blockbinary<fhbits,bt> f;
	decode(r, sign, e, f);

	Color red(ColorCode::FG_RED);
	Color yellow(ColorCode::FG_YELLOW);
	Color blue(ColorCode::FG_BLUE);
	Color magenta(ColorCode::FG_MAGENTA);
	Color cyan(ColorCode::FG_CYAN);
	Color white(ColorCode::FG_WHITE);
	Color def(ColorCode::FG_DEFAULT);

	std::stringstream s;
	// sign bit
	s << red << (sign ? '1' : '0');

	// exponent bits
	for (int i = int(es) - 1; i >= 0; --i) {
		s << cyan << (e.test(static_cast<size_t>(i)) ? '1' : '0');
		if ((i - es) > 0 && ((i - es) % 4) == 0 && nibbleMarker) s << yellow << '\'';
	}

	// fraction bits
	for (int i = int(r.fbits) - 1; i >= 0; --i) {
		s << magenta << (f.test(static_cast<size_t>(i)) ? '1' : '0');
		if (i > 0 && (i % 4) == 0 && nibbleMarker) s << yellow << '\'';
	}

	s << def;
	return s.str();
}


}} // namespace sw::universal
