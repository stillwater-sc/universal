#pragma once
// manipulators.hpp: type tag, binary rendering and interval text for the poxel
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <universal/native/integer_type_tag.hpp>   // type_tag of the block type
#include <universal/traits/poxel_traits.hpp>

namespace sw { namespace universal {

// poxel<17, 2, uint32_t>
template<unsigned nbits, unsigned es, typename bt>
std::string type_tag(const poxel<nbits, es, bt>& = {}) {
	std::stringstream str;
	str << "poxel<" << std::setw(3) << nbits << ", " << es << ", " << type_tag(bt{}) << '>';
	return str.str();
}

// sign.regime.exponent.fraction of the lattice point's magnitude, then the ubit:
// 0b0.10.00.0000|0 is exactly 1.0, 0b0.10.00.0000|1 the open tile above it
template<unsigned nbits, unsigned es, typename bt>
std::string to_binary(const poxel<nbits, es, bt>& v, bool = false) {
	using X = poxel<nbits, es, bt>;
	std::stringstream s;
	const std::int64_t P = v.lattice();
	s << "0b" << (P < 0 ? '1' : '0') << '.';
	if (P == 0 || P == X::lattice_nar) {
		for (unsigned i = X::lbits - 1; i-- > 0;) s << '0';
	}
	else {
		const auto d = X::decode_lattice(P);
		const std::uint64_t a = static_cast<std::uint64_t>(P < 0 ? -P : P);
		unsigned bit = X::lbits - 1;
		auto emit = [&](unsigned count) { for (unsigned i = 0; i < count; ++i) { --bit; s << (((a >> bit) & 1ull) ? '1' : '0'); } };
		emit(d.rlen); s << '.';
		emit(d.ebits); s << '.';
		emit(d.nf);
	}
	s << '|' << (v.ubit() ? '1' : '0');
	return s.str();
}

// One lattice point as text.  When long double holds every lattice value of the
// configuration exactly, print it in decimal; otherwise (more fraction bits than the
// long double significand, as on platforms where long double is double, or a scale
// beyond its exponent range) print the exact hexfloat assembled from the posit fields,
// so neighbouring lattice points never print alike.
template<unsigned nbits, unsigned es, typename bt>
std::string lattice_text(std::int64_t P, int precision) {
	using X = poxel<nbits, es, bt>;
	using L = std::numeric_limits<long double>;
	constexpr bool decimal_is_exact = (X::fbits + 1 <= static_cast<unsigned>(L::digits)) && (X::maxscale < L::max_exponent - 1) && (X::minscale - static_cast<int>(X::fbits) > L::min_exponent - L::digits);
	if (P == X::lattice_nar) return "-inf";              // lower end of (-inf, -maxpos)
	std::stringstream s;
	if (P == 0) { s << 0; return s.str(); }
	if constexpr (decimal_is_exact) {
		s << std::setprecision(precision) << X::template lattice_point<long double>(P);
	}
	else {
		const auto d = X::decode_lattice(P);
		const unsigned pad = (4u - d.nf % 4u) % 4u;                 // align the fraction to whole hex digits
		s << (d.negative ? "-" : "") << "0x1";
		if (d.nf > 0) s << '.' << std::hex << std::setw(static_cast<int>((d.nf + pad) / 4u)) << std::setfill('0') << (d.frac << pad) << std::dec;
		s << 'p' << std::showpos << d.scale;
	}
	return s.str();
}

// "1.5" for an exact tile, "(1.5, 1.625)" for an open one, "nar"
template<unsigned nbits, unsigned es, typename bt>
std::string to_interval(const poxel<nbits, es, bt>& v, int precision = 17) {
	using X = poxel<nbits, es, bt>;
	if (v.isnar()) return "nar";
	const std::int64_t P = v.lattice();
	const std::string lo = lattice_text<nbits, es, bt>(P, precision);
	if (v.isexact()) return lo;
	const std::string hi = (P == X::lattice_maxpos) ? std::string("inf") : lattice_text<nbits, es, bt>(P + 1, precision);
	return "(" + lo + ", " + hi + ")";
}

}} // namespace sw::universal
