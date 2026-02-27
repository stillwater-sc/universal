#pragma once
// manipulators.hpp: definitions of helper functions for decimal dfloat type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Generate a type tag for this dfloat
	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
	std::string type_tag(const dfloat<ndigits, es, Encoding, bt>& = {}) {
		std::stringstream s;
		s << "dfloat<"
			<< std::setw(3) << ndigits << ", "
			<< std::setw(3) << es << ", "
			<< (Encoding == DecimalEncoding::BID ? "BID" : "DPD") << ", "
			<< typeid(bt).name() << '>';
		return s.str();
	}

	// type field descriptor
	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
	std::string type_field(const dfloat<ndigits, es, Encoding, bt>& = {}) {
		using Dfloat = dfloat<ndigits, es, Encoding, bt>;
		std::stringstream s;
		s << "fields(s:1|comb:" << Dfloat::combBits
		  << "|exp:" << es
		  << "|trail:" << Dfloat::t << ')';
		return s.str();
	}

	// color print: show sign, combination, exponent, trailing with ANSI colors
	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
	std::string color_print(const dfloat<ndigits, es, Encoding, bt>& number, bool nibbleMarker = false) {
		using Dfloat = dfloat<ndigits, es, Encoding, bt>;
		std::stringstream s;

		// sign in red
		s << "\033[31m" << (number.sign() ? '1' : '0') << "\033[0m" << '.';

		// combination field in blue (5 bits)
		s << "\033[34m";
		unsigned combStart = Dfloat::nbits - 2;
		for (unsigned i = 0; i < Dfloat::combBits; ++i) {
			s << (number.getbit(combStart - i) ? '1' : '0');
		}
		s << "\033[0m" << '.';

		// exponent continuation in green
		s << "\033[32m";
		unsigned expStart = Dfloat::nbits - 1 - 1 - Dfloat::combBits;
		for (unsigned i = 0; i < es; ++i) {
			s << (number.getbit(expStart - i) ? '1' : '0');
		}
		s << "\033[0m" << '.';

		// trailing significand in default color
		for (int i = static_cast<int>(Dfloat::t) - 1; i >= 0; --i) {
			s << (number.getbit(static_cast<unsigned>(i)) ? '1' : '0');
			if (nibbleMarker && i > 0 && (i % 4 == 0)) s << '\'';
		}

		return s.str();
	}

	// components: show unpacked sign, exponent, significand
	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename bt>
	std::string components(const dfloat<ndigits, es, Encoding, bt>& number) {
		using Dfloat = dfloat<ndigits, es, Encoding, bt>;
		std::stringstream s;
		if (number.isnan()) {
			s << "nan";
		}
		else if (number.isinf()) {
			s << (number.sign() ? "-inf" : "+inf");
		}
		else {
			bool sign; int exp; typename Dfloat::significand_t sig;
			number.unpack(sign, exp, sig);
			s << (sign ? "(-" : "(+") << Dfloat::sig_to_string(sig) << " * 10^" << exp << ')';
		}
		return s.str();
	}

}} // namespace sw::universal
