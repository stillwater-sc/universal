#pragma once
// subnormal_sources.hpp: areal conversion from float and double sources -- subnormal, and normal
// into every width -- checked exactly
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A conversion places a value in the tile that contains it (#1656): with the ubit clear the areal
// is the value itself, with it set the value lies strictly between the areal and the next lattice
// point of larger magnitude.  The check is exact, in tile_oracle's einteger dyadics, since some
// configurations' lattices do not fit in a double.
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <universal/utility/tile_oracle.hpp>

namespace sw { namespace universal {

// does areal a hold the exact value v?
template<typename Areal, typename Real>
bool tile_holds(const Areal& a, Real v) {
	namespace orc = sw::universal::oracle;
	const orc::dyadic x = orc::from_double(static_cast<double>(v));   // float -> double is exact
	const orc::dyadic lo = orc::exact_value(a);                        // the stored lattice point; the ubit is not read
	if (!a.ubit()) return orc::compare(lo, x) == 0;
	Areal next(a);
	++next;                                                            // the next lattice point, away from zero
	if (next.isinf()) {                                                // (maxpos, inf): no lattice point above
		return v > 0 ? orc::compare(lo, x) < 0 : orc::compare(x, lo) < 0;
	}
	const orc::dyadic hi = orc::exact_value(next);
	return v > 0 ? (orc::compare(lo, x) < 0 && orc::compare(x, hi) < 0)
	             : (orc::compare(hi, x) < 0 && orc::compare(x, lo) < 0);
}

// Every stride-th subnormal of Real, the smallest normals, and a sample around the areal's own
// normal/subnormal boundary, both signs.
template<typename Areal, typename Real>
int VerifySubnormalSources(const std::string& name, bool reportTestCases, std::uint64_t stride) {
	using Bits = std::conditional_t<sizeof(Real) == 4, std::uint32_t, std::uint64_t>;
	constexpr int fb = std::numeric_limits<Real>::digits - 1;
	constexpr Bits last = (Bits(1) << fb) - 1u;                        // the largest subnormal pattern
	int nrOfFailedTests = 0;
	auto check = [&](Real v) {
		for (const Real s : { v, Real(-v) }) {
			const Areal a(s);
			if (!tile_holds(a, s)) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL: " << name << "(" << s << ") = " << to_binary(a) << '\n';
			}
		}
	};
	auto from_bits = [](Bits b) { Real r; std::memcpy(&r, &b, sizeof(r)); return r; };
	for (Bits p = 1; p <= last && p >= 1; p += static_cast<Bits>(stride)) check(from_bits(p));
	check(from_bits(last));
	for (Bits p = 0; p < 64; ++p) check(from_bits((Bits(1) << fb) + p));   // the smallest normals
	// the areal's own boundary between normal and subnormal values, where the scale changes
	const Real edge = std::ldexp(Real(1), Areal::MIN_EXP_NORMAL);
	if (edge > 0 && std::isfinite(edge)) {
		Real v = edge;
		for (int i = 0; i < 64; ++i) { check(v); v = std::nextafter(v, Real(0)); }
		v = edge;
		for (int i = 0; i < 64; ++i) { check(v); v = std::nextafter(v, std::numeric_limits<Real>::infinity()); }
	}
	return nrOfFailedTests;
}

// Normal sources at random over a range of scales, both signs: into areals wider than the source's
// bit assembly (float into more than 32 bits, double into more than 64), where every conversion used
// to be wrong (#1656), and past maxpos, where the value lands in (maxpos, inf).
template<typename Areal, typename Real>
int VerifyNormalSources(const std::string& name, bool reportTestCases, int count) {
	std::mt19937_64 rng(1656);
	int nrOfFailedTests = 0;
	for (int i = 0; i < count; ++i) {
		const Real v = std::ldexp(Real(1) + Real(rng() >> 11) * std::ldexp(Real(1), -53), static_cast<int>(rng() % 160) - 80);
		for (const Real s : { v, Real(-v) }) {
			const Areal a(s);
			if (!tile_holds(a, s)) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL: " << name << "(" << s << ") = " << to_binary(a) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal
