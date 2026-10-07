// rump_polynomial.cpp: Rump's polynomial -- what each number system claims, and whether it is true
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
//
// #1637, test 1.  f(a, b) = 333.75 b^6 + a^2 (11 a^2 b^2 - b^6 - 121 b^4 - 2) + 5.5 b^8 + a / (2b)
// at a = 77617, b = 33096.  The exact value is -54767/66192 = -0.827396059946821...; the
// polynomial part cancels to exactly -2 from terms near 7.9e36 = 2^123, so more than 120
// bits are needed even to get the sign.
//
//   - float, double, posit<32,2> round silently and print a confident, wrong number
//   - a single ubit tile (areal, poxel) is a flag: it reports that the result is inexact,
//     but the tile it returns need not contain the true value
//   - a tile_interval is an enclosure: it must contain -0.8274, and at 32 and 64 bits it
//     is honest that it cannot even decide the sign
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

constexpr double exact_value = -54767.0 / 66192.0;

template<typename Real>
Real rump(const Real& a, const Real& b) {
	const Real b2 = b * b, b4 = b2 * b2, b6 = b4 * b2, b8 = b4 * b4, a2 = a * a;
	return Real(333.75) * b6 + a2 * (Real(11) * a2 * b2 - b6 - Real(121) * b4 - Real(2)) + Real(5.5) * b8 + a / (Real(2) * b);
}

// the ubit of a single tile
template<unsigned n, unsigned e, typename bt> bool ubit_of(const sw::universal::areal<n, e, bt>& t) { return t.at(0); }
template<unsigned n, unsigned e, typename bt> bool ubit_of(const sw::universal::poxel<n, e, bt>& t) { return t.ubit(); }

int fails = 0;
void check(bool ok, const std::string& what) {
	std::cout << (ok ? "  ok    " : "  FAIL  ") << what << '\n';
	if (!ok) ++fails;
}

// rounding formats: a single number, no warning
template<typename Real>
double rounded(const char* name) {
	const double v = double(rump(Real(77617), Real(33096)));
	std::cout << std::setw(30) << name << " : " << std::setw(26) << std::setprecision(15) << v << "   (no warning)\n";
	return v;
}

// a single tile: the flag
template<typename Tile>
Tile single(const char* name) {
	const Tile r = rump(Tile(77617), Tile(33096));
	std::cout << std::setw(30) << name << " : " << std::setw(26) << std::setprecision(15) << double(r) << "   ubit = " << ubit_of(r) << '\n';
	return r;
}

// a pair of tiles: the enclosure
template<typename Tile>
sw::universal::tile_interval<Tile> enclosed(const char* name) {
	using I = sw::universal::tile_interval<Tile>;
	const I r = rump(I(77617), I(33096));
	std::cout << std::setw(30) << name << " : " << r.str(6) << "   sign: " << to_string(r.sign()) << '\n';
	return r;
}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	using areal32 = areal<32, 8, std::uint32_t>;
	using poxel32 = poxel<32, 2, std::uint32_t>;   // equal storage: 32 bits, ubit included
	using areal64 = areal<64, 11, std::uint64_t>;
	using poxel64 = poxel<64, 2, std::uint64_t>;

	std::cout << "Rump's polynomial at a = 77617, b = 33096\n";
	std::cout << "exact value: -54767/66192 = " << std::setprecision(15) << exact_value << "\n\n";

	std::cout << "rounding (IEEE-754 and posit):\n";
	const double f  = rounded<float>("float");
	const double d  = rounded<double>("double");
	const double p  = rounded<posit<32, 2>>("posit<32,2>");

	std::cout << "\nsingle tile (sticky ubit):\n";
	const auto sa = single<areal32>("areal<32,8>");
	const auto sp = single<poxel32>("poxel<32,2>");

	std::cout << "\ntile interval (enclosure):\n";
	const auto ia  = enclosed<areal32>("tile_interval<areal<32,8>>");
	const auto ip  = enclosed<poxel32>("tile_interval<poxel<32,2>>");
	const auto ia6 = enclosed<areal64>("tile_interval<areal<64,11>>");
	const auto ip6 = enclosed<poxel64>("tile_interval<poxel<64,2>>");

	std::cout << "\nassertions:\n";
	check(std::abs(f - exact_value) > 1.0, "float is wrong, without warning");
	check(std::abs(d - exact_value) > 1.0, "double is wrong by some 21 orders of magnitude, without warning");
	check(std::abs(p - exact_value) > 1.0, "posit<32,2> is wrong, without warning");
	check(ubit_of(sa) && ubit_of(sp), "both single tiles carry the ubit: the result is flagged inexact");
	check(ia.contains(exact_value) && ip.contains(exact_value) && ia6.contains(exact_value) && ip6.contains(exact_value), "every tile interval contains the exact value");
	check(ia.sign() == tile_verdict::undecidable && ip.sign() == tile_verdict::undecidable && ia6.sign() == tile_verdict::undecidable && ip6.sign() == tile_verdict::undecidable,
	      "at 32 and 64 bits no tile interval claims a sign: more than 120 bits are needed");

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
