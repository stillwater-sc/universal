// griewank_sign.cpp: the sign of the Griewank structure next to its minimum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
//
// #1637, test 5.  g(x, y) = 1 - cos(x) cos(y) - (x^2 + y^2) / 4000 at x = y = 1e-8 (the double
// nearest it).  Expanding cos, g = x^2 (1 - 1/2000) - x^4/3 + ... = +9.995e-17: positive.
//
// The difficulty is 1 - cos(x) cos(y) ~ 1e-16 against cos(x) ~ 1: a format resolves the sign
// only if its spacing just below 1 is finer than about 1e-16.
//
//   - float, double and posit<32,2> round cos(1e-8) to exactly 1, so 1 - cos cos = 0 and
//     g = -(x^2 + y^2)/4000 < 0: the wrong sign, with full confidence
//   - a single ubit tile flags g as inexact
//   - a tile_interval with an enclosing cos (Taylor bounds, see tile_interval.hpp) never
//     claims the wrong sign.  At 32 bits, and in areal<64,11> whose 51 fraction bits are
//     coarser than double's, the enclosure contains 0: undecidable.  poxel<64,2> carries 58
//     fraction bits near 1, a spacing of 3.5e-18, and PROVES g > 0.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <universal/number/areal/areal.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/tile_interval.hpp>

namespace {

const double x0 = 1.0e-8;

// the exact value to double precision: 1 - cos^2 x = x^2 - x^4/3 + O(x^6)
double exact() { return x0 * x0 * (1.0 - 1.0 / 2000.0) - x0 * x0 * x0 * x0 / 3.0; }

template<typename Real, typename Cos>
Real griewank(const Real& x, const Real& y, Cos cosine) {
	return Real(1) - cosine(x) * cosine(y) - (x * x + y * y) / Real(4000);
}

// cos in a single tile's own arithmetic: the same Taylor polynomial, S_28, by Horner
template<typename Tile>
Tile tile_cos(const Tile& x) {
	const Tile s = x * x;
	Tile c[15];
	c[0] = Tile(1);
	for (int j = 1; j <= 14; ++j) c[j] = c[j - 1] / Tile((2 * j - 1) * (2 * j));
	Tile p = c[14];
	for (int j = 13; j >= 0; --j) p = p * s + ((j % 2) ? -c[j] : c[j]);
	return p;
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
	using areal32 = areal<32, 8, std::uint32_t>;
	using poxel32 = poxel<32, 2, std::uint32_t>;   // equal storage: 32 bits, ubit included
	using areal64 = areal<64, 11, std::uint64_t>;
	using poxel64 = poxel<64, 2, std::uint64_t>;
	using P32 = posit<32, 2>;

	const float  gf = griewank(float(x0), float(x0), [](float v) { return std::cos(v); });
	const double gd = griewank(x0, x0, [](double v) { return std::cos(v); });
	const P32    gp = griewank(P32(x0), P32(x0), [](const P32& v) { return P32(std::cos(double(v))); });   // cos correctly rounded to posit<32,2>
	const auto   sa = griewank(areal32(x0), areal32(x0), tile_cos<areal32>);
	const auto   sp = griewank(poxel32(x0), poxel32(x0), tile_cos<poxel32>);
	auto enclose = [](auto tag) {
		using I = tile_interval<decltype(tag)>;
		return griewank(I(x0), I(x0), [](const I& v) { return cos(v); });
	};
	const auto ia  = enclose(areal32{});
	const auto ip  = enclose(poxel32{});
	const auto ia6 = enclose(areal64{});
	const auto ip6 = enclose(poxel64{});

	std::cout << "Griewank structure g(x, y) = 1 - cos(x) cos(y) - (x^2 + y^2)/4000 at x = y = 1e-8\n";
	std::cout << "exact value: " << std::setprecision(6) << exact() << "  (positive)\n\n";
	std::cout << std::setw(30) << "float" << " : " << gf << '\n';
	std::cout << std::setw(30) << "double" << " : " << gd << '\n';
	std::cout << std::setw(30) << "posit<32,2>" << " : " << double(gp) << '\n';
	std::cout << std::setw(30) << "areal<32,8> tile" << " : " << double(sa) << "  ubit = " << ubit_of(sa) << '\n';
	std::cout << std::setw(30) << "poxel<32,2> tile" << " : " << to_interval(sp) << "  ubit = " << ubit_of(sp) << '\n';
	std::cout << std::setw(30) << "tile_interval<areal<32,8>>" << " : " << ia.str(6) << "  sign: " << to_string(ia.sign()) << '\n';
	std::cout << std::setw(30) << "tile_interval<poxel<32,2>>" << " : " << ip.str(6) << "  sign: " << to_string(ip.sign()) << '\n';
	std::cout << std::setw(30) << "tile_interval<areal<64,11>>" << " : " << ia6.str(6) << "  sign: " << to_string(ia6.sign()) << '\n';
	std::cout << std::setw(30) << "tile_interval<poxel<64,2>>" << " : " << ip6.str(6) << "  sign: " << to_string(ip6.sign()) << '\n';

	const double g = exact();
	std::cout << "\nassertions:\n";
	check(gf < 0.0f && gd < 0.0 && double(gp) < 0.0, "float, double and posit<32,2> report g < 0: the wrong sign, without warning");
	check(ubit_of(sa) && ubit_of(sp), "both single tiles flag g as inexact");
	check(ia.contains(g) && ip.contains(g) && ia6.contains(g) && ip6.contains(g), "every tile interval contains the exact value");
	check(ia.sign() == tile_verdict::undecidable && ip.sign() == tile_verdict::undecidable && ia6.sign() == tile_verdict::undecidable,
	      "32-bit tile intervals and areal<64,11> are honestly undecidable");
	check(ip6.sign() == tile_verdict::positive, "tile_interval<poxel<64,2>> proves g > 0");

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
