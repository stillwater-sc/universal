// bbp_tail.cpp: the Bailey-Borwein-Plouffe tail -- is it in (0, 1e-16)?
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
//
// #1637, test 4.  S = sum_{k>=15} 16^-k (4/(8k+1) - 2/(8k+4) - 1/(8k+5) - 1/(8k+6)) = 9.109e-22.
// Every term is positive, so S > 0.  Is S in (0, 1e-16)?
//
// The sum is formed as S = 16^-15 * T, T = sum_{j>=0} 16^-j c_{15+j} ~ 1.05e-3, so that only
// the final product approaches the bottom of a format's range.
//
// In double and float nothing underflows (16^-15 = 8.7e-19), and both answer correctly.
// The question bites in 16-bit formats, whose range ends above 1e-19:
//
//   - IEEE half flushes 16^-15 to zero and reports S = 0: it cannot say S > 0
//   - posit<16,2> never underflows, but rounds S up to minpos, 1.4e-17: a confident answer,
//     too large by a factor of 15000
//   - a single 16-bit ubit tile flags the result as inexact
//   - a 16-bit tile_interval encloses 16^-15 in the open tile (0, minpos), so S is
//     strictly positive; T to j = 25 plus an enclosure of its remainder,
//     [0, 16^-26 * 4/329 * 16/15], bounds the whole series.  poxel<17,2>, whose minpos is
//     1.4e-17, thereby PROVES 0 < S < 1e-16; areal<16,5> proves S > 0, but its smallest
//     tile ends at 1.2e-7, so it cannot bound S below 1e-16 -- and says so
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/tile_interval.hpp>

namespace {

constexpr int K0 = 15, K = 40;

template<typename Real>
Real term_coefficient(int k) {
	return Real(4) / Real(8 * k + 1) - Real(2) / Real(8 * k + 4) - Real(1) / Real(8 * k + 5) - Real(1) / Real(8 * k + 6);
}

template<typename Real>
Real power16(int k) {   // 16^-k
	Real p = Real(1);
	for (int i = 0; i < k; ++i) p = p / Real(16);
	return p;
}

// T = sum_{j=0..K-K0} 16^-j c_{K0+j}
template<typename Real>
Real inner_sum() {
	Real t = Real(0), p = Real(1);
	for (int k = K0; k <= K; ++k) {
		t = t + p * term_coefficient<Real>(k);
		p = p / Real(16);
	}
	return t;
}

// S = 16^-K0 * T, truncated at K
template<typename Real>
Real partial_sum() { return power16<Real>(K0) * inner_sum<Real>(); }

// the whole series, enclosed: 16^-K0 * (T + [0, R]), R = 16^-(K-K0+1) * 4/(8(K+1)+1) * 16/15
// bounds the rest of T, since c_k < 4/(8k+1)
template<typename Tile>
sw::universal::tile_interval<Tile> series() {
	using I = sw::universal::tile_interval<Tile>;
	const I r = power16<I>(K - K0 + 1) * I(4) / I(8 * (K + 1) + 1) * I(16) / I(15);
	return power16<I>(K0) * (inner_sum<I>() + I::from_keys(0, r.hi_key()));
}

template<unsigned n, unsigned e, typename bt> bool ubit_of(const sw::universal::areal<n, e, bt>& t) { return t.at(0); }
template<unsigned n, unsigned e, typename bt> bool ubit_of(const sw::universal::poxel<n, e, bt>& t) { return t.ubit(); }

int fails = 0;
void check(bool ok, const std::string& what) {
	std::cout << (ok ? "  ok    " : "  FAIL  ") << what << '\n';
	if (!ok) ++fails;
}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	using areal16 = areal<16, 5, std::uint16_t>;

	const double sd = partial_sum<double>();      // the tail beyond K = 40 is below 1e-50: negligible here
	const float  sf = partial_sum<float>();
	const half   sh = partial_sum<half>();
	const posit<16, 2> sp = partial_sum<posit<16, 2>>();
	const areal16 ta = partial_sum<areal16>();
	const poxel17 tp = partial_sum<poxel17>();
	const auto ia = series<areal16>();
	const auto ip = series<poxel17>();

	std::cout << "BBP tail: S = sum_{k>=15} 16^-k (4/(8k+1) - 2/(8k+4) - 1/(8k+5) - 1/(8k+6)); is S in (0, 1e-16)?\n\n";
	std::cout << std::setprecision(6);
	std::cout << std::setw(30) << "double" << " : " << sd << '\n';
	std::cout << std::setw(30) << "float" << " : " << sf << '\n';
	std::cout << std::setw(30) << "half" << " : " << double(sh) << '\n';
	std::cout << std::setw(30) << "posit<16,2>" << " : " << double(sp) << '\n';
	std::cout << std::setw(30) << "areal<16,5> tile" << " : " << double(ta) << "  ubit = " << ubit_of(ta) << '\n';
	std::cout << std::setw(30) << "poxel<17,2> tile" << " : " << to_interval(tp) << "  ubit = " << ubit_of(tp) << '\n';
	std::cout << std::setw(30) << "tile_interval<areal<16,5>>" << " : " << ia.str(6) << "  sign: " << to_string(ia.sign()) << '\n';
	std::cout << std::setw(30) << "tile_interval<poxel<17,2>>" << " : " << ip.str(6) << "  sign: " << to_string(ip.sign()) << '\n';

	auto decided = [](const auto& i) { return i.sign() == tile_verdict::positive && i.template upper<double>() <= 1.0e-16; };

	std::cout << "\nassertions:\n";
	check(sd > 9.10e-22 && sd < 9.12e-22 && sf > 0.0f && double(sf) < 1.0e-16, "double and float answer correctly: nothing underflows in their range");
	check(!(double(sh) > 0.0), "half flushes the tail to zero: it cannot establish S > 0");
	check(double(sp) > 1.0e-18, "posit<16,2> rounds S up to minpos: too large by four orders of magnitude, without warning");
	check(ubit_of(ta) && ubit_of(tp), "both 16-bit single tiles flag the sum as inexact");
	check(ia.contains(sd) && ip.contains(sd), "both 16-bit tile intervals contain S");
	check(decided(ip), "tile_interval<poxel<17,2>> proves 0 < S < 1e-16");
	check(ia.sign() == tile_verdict::positive && !decided(ia), "tile_interval<areal<16,5>> proves S > 0, and honestly cannot bound S below its smallest tile, which ends at 1.2e-7");

	std::cout << (fails == 0 ? "PASS\n" : "FAIL\n");
	return (fails == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
