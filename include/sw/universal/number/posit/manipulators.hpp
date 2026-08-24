#pragma once
// manipulators.hpp: definitions of helper functions for posit type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iomanip>
#include <string>
#include <sstream>
#include <cmath>  // for frexp/frexpf
#include <typeinfo>  // for typeid()

// pull in the color printing for shells utility
#include <universal/utility/color_print.hpp>

// This file contains functions that manipulate a posit type
// using posit number system knowledge.

namespace sw { namespace universal {

	// Generate a type tag for this posit, for example, posit<8,1>
	template<unsigned nbits, unsigned es, typename bt>
	std::string type_tag(const posit<nbits, es, bt> & = {}) {
		std::stringstream str;
		str << "posit<"
			<< std::setw(3) << nbits << ", "
			<< std::setw(1) << es << ", "
			<< type_tag(bt{}) << '>';
		return str.str();
	}

	// report the dynamic range of a posit
	template<unsigned nbits, unsigned es, typename bt = std::uint8_t>
	std::string posit_range() {
		std::stringstream str;
		str << " posit<" << std::setw(3) << nbits << "," << es << "> ";
		str << "useed scale  " << std::setw(4) << useed_scale<es>() << "     ";
		str << "minpos scale " << std::setw(10) << minpos_scale<nbits, es>() << "     ";
		str << "maxpos scale " << std::setw(10) << maxpos_scale<nbits, es>() << "     ";
		str << "minimum " << std::setw(12) << std::numeric_limits<sw::universal::posit<nbits, es, bt>>::min() << "     ";
		str << "maximum " << std::setw(12) << std::numeric_limits<sw::universal::posit<nbits, es, bt>>::max() ;
		return str.str();
	}

	// generate a posit format ASCII format nbits.esxNN...NNp
	template<unsigned nbits, unsigned es, typename bt>
	inline std::string hex_print(const posit<nbits, es, bt>& p) {
		// we need to transform the posit into a string
		std::stringstream str;
		str << nbits << '.' << es << 'x' << to_hex(p.bits()) << 'p';
		return str.str();
	}


	// Generate a string representing the posit components: sign, regime, exponent, significand
	template<unsigned nbits, unsigned es, typename bt>
	std::string components(const posit<nbits, es, bt>& p) {
		constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
		std::stringstream str;
		bool		     		     _sign{false};
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(p.bits(), _sign, _regime, _exponent, _fraction);

		str << "sign: " << (_sign ? '-' : '+')
			<< ", regime: " << _regime.positRegime_k()
			<< ", exponent: " << exponent_value(p)
			<< ", significand: " << std::setprecision(21) << (1.0 + _fraction.value());

		return str.str();
	}

	template<unsigned nbits, unsigned es, typename bt>
	std::string pretty_print(const posit<nbits, es, bt>& p, int printPrecision = std::numeric_limits<double>::max_digits10) {
		constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
		std::stringstream str;
		bool		     		     _sign;
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(p.bits(), _sign, _regime, _exponent, _fraction);
		str << ( _sign ? "s1 r" : "s0 r" );
		blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned> r = _regime.bits();
		int regimeBits = (int)_regime.nrBits();
		int nrOfRegimeBitsProcessed = 0;
		for (int i = nbits - 2; i >= 0; --i) {
			if (regimeBits > nrOfRegimeBitsProcessed++) {
				str << (r.test(static_cast<unsigned>(i)) ? "1" : "0");
			}
		}
		str << " e";
		std::uint32_t expBits = _exponent.bits();
		int exponentBits = (int)_exponent.nrBits();
		int nrOfExponentBitsProcessed = 0;
		for (int i = int(es) - 1; i >= 0; --i) {
			if (exponentBits > nrOfExponentBitsProcessed++) {
				str << ((expBits >> i) & 1 ? "1" : "0");
			}
		}
		str << " f";
		if constexpr (fbits > 0) {
			blockbinary<fbits, bt, BinaryNumberType::Unsigned> f = _fraction.bits();
			int fractionBits = (int)_fraction.nrBits();
			int nrOfFractionBitsProcessed = 0;
			for (int i = int(fbits) - 1; i >= 0; --i) {
				if (fractionBits > nrOfFractionBitsProcessed++) {
					str << (f.test(static_cast<unsigned>(i)) ? "1" : "0");
				}
			}
		}
		str << " q";
		str << quadrant(p) << " v"
			<< std::setprecision(printPrecision) << p
			<< std::setprecision(0);
		return str.str();
	}

	template<unsigned nbits, unsigned es, typename bt>
	std::string info_print(const posit<nbits, es, bt>& p, int printPrecision = 17) {
		constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
		std::stringstream str;
		bool		     		     _sign;
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(p.bits(), _sign, _regime, _exponent, _fraction);

		str << "raw: " << p.bits() << " "
			<< quadrant(p) << " "
			<< (_sign ? "s1 r" : "s0 r")
			<< _regime << " e"
			<< _exponent << " f"
			<< _fraction << " : value "
			<< std::setprecision(printPrecision) << p
			<< std::setprecision(0);
		return str.str();
	}

	template<unsigned nbits, unsigned es, typename bt>
	std::string color_print(const posit<nbits, es, bt>& p) {
		constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
		std::stringstream str;
		bool		     		_sign;
		positRegime<nbits, es, bt>   _regime;
		positExponent<nbits, es, bt> _exponent;
		positFraction<fbits, bt>     _fraction;
		decode(p.bits(), _sign, _regime, _exponent, _fraction);

		Color red(ColorCode::FG_RED);
		Color yellow(ColorCode::FG_YELLOW);
		Color cyan(ColorCode::FG_CYAN);
		Color magenta(ColorCode::FG_MAGENTA);
		Color def(ColorCode::FG_DEFAULT);

		// sign bit
		str << red << (_sign ? '1' : '0');

		// regime bits: read decoded field bits directly (no inversion)
		blockbinary<nbits - 1, bt, BinaryNumberType::Unsigned> r = _regime.bits();
		int regimeBits = (int)_regime.nrBits();
		int nrOfRegimeBitsProcessed = 0;
		for (unsigned i = 0; i < nbits - 1; ++i) {
			unsigned bitIndex = nbits - 2ull - i;
			if (regimeBits > nrOfRegimeBitsProcessed++) {
				str << yellow << (r.test(bitIndex) ? '1' : '0');
			}
		}

		// exponent bits: read decoded field bits directly (no inversion)
		blockbinary<es, bt, BinaryNumberType::Unsigned> e = _exponent.bits();
		int exponentBits = (int)_exponent.nrBits();
		int nrOfExponentBitsProcessed = 0;
		for (int i = es - 1; i >= 0; --i) {
			if (exponentBits > nrOfExponentBitsProcessed++) {
				str << cyan << (e.test(static_cast<unsigned>(i)) ? '1' : '0');
			}
		}

		// fraction bits: read decoded field bits directly
		blockbinary<posit<nbits, es>::fbits, bt, BinaryNumberType::Unsigned> f = _fraction.bits();
		int fractionBits = (int)_fraction.nrBits();
		int nrOfFractionBitsProcessed = 0;
		for (int i = int(p.fbits) - 1; i >= 0; --i) {
			if (fractionBits > nrOfFractionBitsProcessed++) {
				str << magenta << (f.test(static_cast<unsigned>(i)) ? '1' : '0');
			}
		}

		str << def;
		return str.str();
	}



// ---------------------------------------------------------------------------
// text produced from a posit (#1334). These lived in posit_impl.hpp, which
// made every translation unit that did arithmetic pay for <sstream>/<iomanip>.
// manipulators.hpp is where the library already keeps to_binary/to_hex and the
// other string producers, so they belong here rather than in a header of their own.
// ---------------------------------------------------------------------------


////////////////// POSIT operators

// stream operators


// generate a posit format ASCII format nbits.esxNN...NNp
template<unsigned nbits, unsigned es, typename bt>
inline std::string hex_format(const posit<nbits, es, bt>& p) {
	// we need to transform the posit into a string
	std::stringstream ss;
	ss << nbits << '.' << es << 'x' << to_hex(p.bits()) << 'p';
	return ss.str();
}

template<typename Float>
inline std::string hex_format(Float f) {
	std::stringstream ss;
	ss << std::hexfloat << std::setprecision(std::numeric_limits<Float>::digits10) << f;
	return ss.str();
}


// convert a posit value to a string using "nar" as designation of NaR
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_string(const posit<nbits, es, bt>& p, std::streamsize precision = 17) {
	if (p.isnar()) return std::string("nar");
	constexpr unsigned pfbits = posit<nbits, es, bt>::fbits;
	if constexpr (pfbits == 0) {
		std::ostringstream oss;
		oss << std::setprecision(precision) << static_cast<double>(p);
		return oss.str();
	} else {
		auto v = p.template to_value<BlockTripleOperator::REP>();
		return v.to_string(precision, 0, false, true, false, false, false, false, ' ');
	}
}


// binary representation of a posit with delimiters: i.e. 0.10.00.000000 => sign.regime.exp.fraction
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_binary(const posit<nbits, es, bt>& number, bool nibbleMarker = false) {
	
	constexpr unsigned fbits = (es + 2ull >= nbits ? 0ull : nbits - 3ull - es);             // maximum number of fraction bits: derived

	bool negative{ false };
	positRegime<nbits, es, bt> r;
	positExponent<nbits, es, bt> e;
	positFraction<fbits, bt> f;
	auto raw = number.bits();
	extract_fields(raw, negative, r, e, f);

	std::stringstream s;
	s << (negative ? "0b1." : "0b0.");
	s << to_string(r, false, nibbleMarker) << "."
	  << to_string(e, false, nibbleMarker) << "."
	  << to_string(f, false, nibbleMarker);

	return s.str();
}


// native semantic representation: radix-2, delegates to to_binary
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_native(const posit<nbits, es, bt>& number, bool nibbleMarker = false) {
	return to_binary(number, nibbleMarker);
}

template<unsigned nbits, unsigned es, typename bt>
inline std::string to_triple(const posit<nbits, es, bt>& number, bool nibbleMarker = false) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);             // maximum number of fraction bits: derived

	bool s{ false };
	positRegime<nbits, es, bt> r;
	positExponent<nbits, es, bt> e;
	positFraction<fbits, bt> f;
	blockbinary<nbits, bt> raw = number.bits();
	std::stringstream ss;
	extract_fields(raw, s, r, e, f);

	if (number.iszero()) {
		ss << "(+, 0, ~)";
	}
	else if (number.isnar()) {
		ss << "(nar)";
	}
	else {
		ss << (s ? "(-, " : "(+, ");
		ss << r.scale() + e.scale()
		   << ", "
		   << to_string(f, false, nibbleMarker)
		   << ')';
	}

	return ss.str();
}



// binary exponent representation: i.e. 1.0101010e2^-37
template<unsigned nbits, unsigned es, typename bt>
inline std::string to_base2_scientific(const posit<nbits, es, bt>& number) {
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);             // maximum number of fraction bits: derived
	bool s{ false };
	scale(number);
	positRegime<nbits, es, bt> r;
	positExponent<nbits, es, bt> e;
	positFraction<fbits, bt> f;
	blockbinary<nbits, bt> raw = number.bits();
	std::stringstream ss;
	extract_fields(raw, s, r, e, f);
	ss << (s ? "-" : "+") << "1." << to_string(f, true) << "e2^" << std::showpos << r.scale() + e.scale();
	return ss.str();
}



// quadrant returns a two character string indicating the quadrant of the projective reals the posit resides: from 0, SE, NE, NaR, NW, SW
template<unsigned nbits, unsigned es, typename bt>
std::string quadrant(const posit<nbits, es, bt>& p) {
	posit<nbits, es, bt> pOne(1), pMinusOne(-1);
	if (sign(p)) {
		// west
		if (p > pMinusOne) {
			return "SW";
		}
		else {
			return "NW";
		}
	}
	else {
		// east
		if (p < pOne) {
			return "SE";
		}
		else {
			return "NE";
		}
	}
}



// or a decimal floating-point representation
template<unsigned nbits, unsigned es, typename bt>
bool parse(const std::string& txt, posit<nbits, es, bt>& p) {
	// check if the txt is of the native posit form: nbits.esXhexvalue
	std::regex posit_regex(R"(^[0-9]+\.[0-9]+[xX][0-9A-Fa-f]+p?$)");
	if (std::regex_match(txt, posit_regex)) {
		// found a posit representation: parse nbits.esxHEXVALUEp
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
			if (*it == 'p') break;
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
		// native posit form must match target configuration
		if (nbits_in != nbits || es_in != es) return false;
		uint64_t raw = 0;
		std::istringstream ss(bitStr);
		ss >> std::hex >> raw;
		if (ss.fail()) return false;
		ss >> std::ws;
		if (!ss.eof()) return false;
		p.setbits(raw);
		return true;
	}
	else {
		// Decimal floating-point representation.
		// Route through the high-precision decimal_to_binary utility so that
		// wide posit configurations (nbits > 64) don't lose precision through
		// an intermediate double. The utility delivers a normalized mantissa
		// with target_mantissa_bits bits plus guard/sticky; we feed that
		// directly into convert_<>() so rounding is done once in the posit
		// encoding step.
		// Special-value literals (nan / inf in any common spelling) map to NaR.
		{
			std::string t; t.reserve(txt.size());
			for (char c : txt) t.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
			std::string body = t;
			if (!body.empty() && (body.front() == '+' || body.front() == '-')) body.erase(0, 1);
			if (body == "nan" || body == "inf" || body == "infinity") {
				p.setnar();
				return true;
			}
		}
		constexpr unsigned extractBits         = nbits + 4;
		constexpr unsigned target_mantissa_bits = extractBits + 1;
		auto d = ::sw::universal::decimal_to_binary::convert(
			std::string_view{txt}, target_mantissa_bits);
		if (!d.valid) return false;
		if (d.is_zero) {
			p.setzero();
			return true;
		}
		blocksignificand<extractBits, bt> frac;
		for (unsigned i = 0; i < extractBits; ++i) {
			if (d.mantissa.at(i)) frac.setbit(i, true);
		}
		// Fold d2b's residual guard/sticky into the lowest bit so convert_'s
		// own sticky accumulator picks them up.
		if (d.guard_bit || d.sticky_bit) frac.setbit(0, true);
		convert_<nbits, es, bt, extractBits>(d.negative,
			static_cast<int>(d.binary_scale), frac, p);
		return true;
	}
}


}} // namespace sw::universal
