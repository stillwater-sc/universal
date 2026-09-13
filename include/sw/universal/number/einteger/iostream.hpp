#pragma once
// iostream.hpp: stream insertion and extraction for einteger
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2b of the einteger headers (#1334, Phase 2 group 5b, #1455): the <iostream>
// half. manipulators.hpp is the <iomanip> half -- everything that turns an einteger
// into a std::string. Self-contained.
//
// This header includes only core.hpp. operator>> hands its token to parse(), which is
// in the core because assign(const std::string&) is built on it; and convert_to_string
// renders from the limbs, never through a stream. So neither half of the text layer
// depends on the other, and there is no cycle for #pragma once to mask (#1427).
#include <ios>        // std::ios_base::fmtflags, consumed by convert_to_string
#include <iostream>   // std::ostream / std::istream / std::cerr
#include <string>

#include <universal/number/einteger/core.hpp>

namespace sw { namespace universal {

template<typename BlockType>
std::string convert_to_string(std::ios_base::fmtflags flags, const einteger<BlockType>& n) {
	using AdaptiveInteger = einteger<BlockType>;

	if (n.limbs() == 0) return std::string("0");

	// set the base of the target number system to convert to
	int base = 10;
	if ((flags & std::ios_base::oct) == std::ios_base::oct) base = 8;
	if ((flags & std::ios_base::hex) == std::ios_base::hex) base = 16;

	unsigned nbits = n.limbs() * sizeof(BlockType) * 8;

	std::string result;
	if (base == 8 || base == 16) {
		if (n.sign()) return std::string("negative value: ignored");

		size_t shift = (base == 8 ? 3ull : 4ull);
		BlockType mask = static_cast<BlockType>((1u << shift) - 1);
		AdaptiveInteger t(n);
		result.assign(nbits / shift + ((nbits % shift) ? 1 : 0), '0');
		size_t pos = result.size() - 1ull;
		for (size_t i = 0; i < nbits / shift; ++i) {
			char c = '0' + static_cast<char>(t.block(0) & mask);
			if (c > '9')
				c += 'A' - '9' - 1;
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
			result.insert(static_cast<std::string::size_type>(0), pp);
		}
	}
	else {
		unsigned block10;
		unsigned digits_in_block10;
		if constexpr (AdaptiveInteger::bitsInBlock == 8) {
			block10 = 100u;
			digits_in_block10 = 2;
		}
		else if constexpr (AdaptiveInteger::bitsInBlock == 16) {
			block10 = 10'000ul;
			digits_in_block10 = 4;
		}
		else if constexpr (AdaptiveInteger::bitsInBlock == 32) {
			block10 = 1'000'000'000ul;
			digits_in_block10 = 9;
		}
		else if constexpr (AdaptiveInteger::bitsInBlock == 64) {
			// not allowed as the whole multi-digit arithmetic
			// requires that there is a 'larger' type that
			// can receive carries and borrows.
			// If your platform does have a native 128bit
			// integer, this could be enabled
			//block10 = 1'000'000'000'000'000'000ull;
			//digits_in_block10 = 18;
		}
		result.assign(nbits / 3 + 1ull, '0');
		size_t pos = result.size() - 1ull;
		AdaptiveInteger t(n);
		while (!t.iszero()) {
			AdaptiveInteger q,r;
			q.reduce(t, block10, r);
			BlockType v = r.block(0);
//			std::cout << "v  " << uint32_t(v) << '\n';
			for (unsigned i = 0; i < digits_in_block10; ++i) {
				char c = '0' + static_cast<char>(v % 10);
				v /= 10;
				result[pos] = c;
//				std::cout << result << " pos: " << pos << '\n';
				if (pos-- == 0)	break;
			}
			t = q;
		}

		std::string::size_type firstDigit = result.find_first_not_of('0');
		result.erase(0, firstDigit);
		if (result.empty())
			result = "0";
		if (n.isneg())
			result.insert(0ull, 1ull, '-');
		else if (flags & std::ios_base::showpos)
			result.insert(0ull, 1ull, '+');
	}
	return result;
}

// generate an einteger format ASCII format
template<typename BlockType>
inline std::ostream& operator<<(std::ostream& ostr, const einteger<BlockType>& i) {
	std::string s = convert_to_string(ostr.flags(), i);
	std::streamsize width = ostr.width();
	if (width > static_cast<std::streamsize>(s.size())) {
		char fill = ostr.fill();
		if ((ostr.flags() & std::ios_base::left) == std::ios_base::left)
			s.append(static_cast<std::string::size_type>(width - s.size()), fill);
		else
			s.insert(static_cast<std::string::size_type>(0), static_cast<std::string::size_type>(width - s.size()), fill);
	}
	return ostr << s;
}

// read an ASCII einteger format

template<typename BlockType>
inline std::istream& operator>>(std::istream& istr, einteger<BlockType>& p) {
	std::string txt;
	if (!(istr >> txt)) {
		// extraction failed (already-bad stream or EOF); failbit set by >>.
		return istr;
	}
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into an einteger value\n";
		istr.setstate(std::ios::failbit);
	}
	return istr;
}

}} // namespace sw::universal
