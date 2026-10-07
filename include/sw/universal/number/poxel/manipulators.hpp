#pragma once
// manipulators.hpp: type tag, binary rendering and interval text for the poxel
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

// "1.5" for an exact tile, "(1.5, 1.625)" for an open one, "nar"
template<unsigned nbits, unsigned es, typename bt>
std::string to_interval(const poxel<nbits, es, bt>& v, int precision = 17) {
	std::stringstream s;
	s << std::setprecision(precision);
	if (v.isnar()) return "nar";
	if (v.isexact()) { s << v.template lower<long double>(); return s.str(); }
	s << '(' << v.template lower<long double>() << ", " << v.template upper<long double>() << ')';
	return s.str();
}

}} // namespace sw::universal
