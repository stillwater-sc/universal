#pragma once
// manipulators.hpp: type tag, binary rendering and range reporting for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <universal/native/integer_type_tag.hpp>   // type_tag of the block type
#include <universal/traits/bposit_traits.hpp>

namespace sw { namespace universal {

// bposit<16, 6, 5, uint16_t>
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
std::string type_tag(const bposit<nbits, rs, es, bt>& = {}) {
	std::stringstream str;
	str << "bposit<" << std::setw(3) << nbits << ", " << rs << ", " << es << ", " << type_tag(bt{}) << '>';
	return str.str();
}

// sign.regime.exponent.fraction of the magnitude, as posit's to_binary does: a negative
// value shows its sign bit and the fields of its two's complement
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
std::string to_binary(const bposit<nbits, rs, es, bt>& v, bool nibbleMarker = false) {
	using B = bposit<nbits, rs, es, bt>;
	std::stringstream s;
	const std::uint64_t p = v.raw();
	s << "0b" << (((p >> (nbits - 1)) & 1ull) ? '1' : '0') << '.';
	if (v.iszero() || v.isnar()) {
		for (unsigned i = nbits - 1; i-- > 0;) s << '0';
		return s.str();
	}
	const auto d = v.decode_fields();
	const std::uint64_t a = d.negative ? B::negate_pattern(p) : p;
	unsigned bit = nbits - 1;
	auto emit = [&](unsigned count) {
		for (unsigned i = 0; i < count; ++i) {
			--bit;
			s << (((a >> bit) & 1ull) ? '1' : '0');
			if (nibbleMarker && i + 1 < count && ((count - 1 - i) % 4) == 0) s << '\'';
		}
	};
	emit(d.rlen);
	s << '.';
	emit(es);
	s << '.';
	emit(d.nf);
	return s.str();
}

// range of the configuration
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
std::string bposit_range(const bposit<nbits, rs, es, bt>& v = {}) {
	using B = bposit<nbits, rs, es, bt>;
	std::stringstream s;
	s << type_tag(v) << ": scale [" << B::minscale << ", " << B::maxscale << "], "
	  << B::fbitsmin << ".." << B::fbits << " fraction bits, minpos "
	  << double(B(SpecificValue::minpos)) << ", maxpos " << double(B(SpecificValue::maxpos));
	return s.str();
}

}} // namespace sw::universal
