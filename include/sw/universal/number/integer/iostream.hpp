#pragma once
// iostream.hpp: the <iostream> layer for integer -- operator<< and operator>>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the integer headers (#1334). Moved out of integer_impl.hpp so a
// translation unit that only computes does not pay for the stream machinery.
// Self-contained: include it directly and it works.
#include <ios>        // std::ios_base::fmtflags, consumed by convert_to_string
#include <iostream>   // std::ostream / std::istream / std::cerr
#include <string>

#include <universal/number/integer/core.hpp>

namespace sw { namespace universal {

template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
std::string convert_to_string(std::ios_base::fmtflags flags, const integer<nbits, BlockType, NumberType>& n) {
	using IntegerBase = integer<nbits, BlockType, NumberType>;

	// set the base of the target number system to convert to
	int base = 10;
	if ((flags & std::ios_base::oct) == std::ios_base::oct) base = 8;
	if ((flags & std::ios_base::hex) == std::ios_base::hex) base = 16;

	std::string result;
	if (base == 8 || base == 16) {
		if (n.sign()) return std::string("negative value: ignored");

		BlockType shift = static_cast<BlockType>(base == 8 ? 3 : 4);
		BlockType mask = static_cast<BlockType>((1u << shift) - 1);
		IntegerBase t(n);
		result.assign(nbits / shift + ((nbits % shift) ? 1 : 0), '0');
		std::string::size_type pos = result.size() - 1u;
		for (unsigned i = 0; i < nbits / static_cast<unsigned>(shift); ++i) {
			char c = '0' + static_cast<char>(t.block(0) & mask);
			if (c > '9')
				c += 'A' - '9' - static_cast<char>(1);
			result[pos--] = c;
			t >>= static_cast<int>(shift);
		}
		if (nbits % shift) {
			mask = static_cast<BlockType>((1u << (nbits % shift)) - 1);
			char c = '0' + static_cast<char>(t.block(0) & mask);
			if (c > '9')
				c += 'A' - '9';
			result[pos] = c;
		}
		//
		// Get rid of leading zeros:
		//
		std::string::size_type fnz = result.find_first_not_of('0');
		if (!result.empty() && (fnz == std::string::npos)) fnz = result.size() - 1;
		result.erase(0, fnz);
		if (flags & std::ios_base::showbase) {
			const char* pp = base == 8 ? "0" : "0x";
			result.insert(0ull, pp);
		}
	}
	else {
		using Integer = integer<nbits + 1, BlockType, NumberType>;  // nbits+1 to be able to represent maxneg in 2's complement form

		Integer t(n);
		if constexpr (NumberType == IntegerNumberType::IntegerNumber) {
			if (t.sign()) t.twosComplement();
		}

		Integer block10;
		unsigned digits_in_block10 = 2;
		if constexpr (IntegerBase::bitsInBlock == 8) {
			block10 = 100u;
			digits_in_block10 = 2;
		}
		else if constexpr (IntegerBase::bitsInBlock == 16) {
			block10 = 10'000u;
			digits_in_block10 = 4;
		}
		else if constexpr (IntegerBase::bitsInBlock == 32) {
			block10 = 1'000'000'000ul;
			digits_in_block10 = 9;
		}
		else if constexpr (IntegerBase::bitsInBlock == 64) {
			block10 = 1'000'000'000'000'000'000ull;
			digits_in_block10 = 18;
		}

		result.assign(nbits / 3 + 1u, '0');
		int pos = static_cast<int>(result.size() - 1u);
		while (!t.iszero()) {
			Integer t2 = t / block10;
			Integer r  = t % block10;
			BlockType v = r.block(0);
			for (unsigned i = 0; i < digits_in_block10; ++i) {
				char c = '0' + static_cast<char>(v % 10);
				v /= 10;
				result[static_cast<unsigned>(pos)] = c;
//				std::cout << "result : " << result << " : pos : " << pos << '\n';
				if (pos-- == 0) break;
			}
			t = t2;
			if (pos < 0) break;
		}

		std::string::size_type firstDigit = result.find_first_not_of('0');
		result.erase(0, firstDigit);
		if (result.empty())	result = std::string("0");
		if (n.isneg()) { // no need to specialize as isneg() will return false for Natural and Whole Number types
			result.insert(static_cast<std::string::size_type>(0), 1, '-');
		}
		else if (flags & std::ios_base::showpos) {
			result.insert(static_cast<std::string::size_type>(0), 1, '+');
		}
	}
	return result;
}

template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
inline std::ostream& operator<<(std::ostream& ostr, const integer<nbits, BlockType, NumberType>& i) {
	std::string s = convert_to_string(ostr.flags(), i);
	std::streamsize width = ostr.width();
	if (width > static_cast<std::streamsize>(s.size())) {
		char fill = ostr.fill();
		// width > s.size() here, so the subtraction stays non-negative
		std::string::size_type pad = static_cast<std::string::size_type>(width) - s.size();
		if ((ostr.flags() & std::ios_base::left) == std::ios_base::left)
			s.append(pad, fill);
		else
			s.insert(static_cast<std::string::size_type>(0), pad, fill);
	}
	return ostr << s;
}

// read an ASCII integer format.
// On parse failure: log a diagnostic to std::cerr AND set failbit on the stream
// so callers (loops with `while (in >> x)`, etc.) can detect the error without
// scraping stderr. Symmetric with fixpnt's operator>>.
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
inline std::istream& operator>>(std::istream& istr, integer<nbits, BlockType, NumberType>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into an integer value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}


}} // namespace sw::universal
