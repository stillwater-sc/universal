// muller_kahan.cpp: the Muller-Kahan recurrence -- false convergence, and who notices
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
//
// #1637, test 2.  x_{n+1} = 111 - (1130 - 3000 / x_{n-1}) / x_n,  x_0 = 11/2, x_1 = 61/11.
// The recurrence has fixed points 5, 6 and 100.  From these starting values the exact
// solution is x_n = (6^{n+1} + 5^{n+1}) / (6^n + 5^n), which tends to 6; but 100 is the
// attracting fixed point, and the smallest rounding error excites it.
//
//   - float, double, posit<32,2> settle confidently on 100
//   - a single ubit tile flags every iterate from x_1 on as inexact
//   - a tile_interval contains the exact iterate at every step; once the enclosure can no
//     longer bound the iterate it widens rather than converging to the wrong answer
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <universal/number/areal/areal.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/tile_interval.hpp>

namespace {

constexpr std::size_t N = 30;

// the exact iterate, to double precision
double exact(std::size_t i) { const double n = static_cast<double>(i); return (std::pow(6.0, n + 1) + std::pow(5.0, n + 1)) / (std::pow(6.0, n) + std::pow(5.0, n)); }

template<typename Real>
std::vector<Real> iterate() {
	std::vector<Real> x;
	x.push_back(Real(11) / Real(2));
	x.push_back(Real(61) / Real(11));
	for (std::size_t n = 1; n < N; ++n) x.push_back(Real(111) - (Real(1130) - Real(3000) / x[n - 1]) / x[n]);
	return x;
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
	using IA = tile_interval<areal32>;
	using IP = tile_interval<poxel33>;

	const auto xf = iterate<float>();
	const auto xd = iterate<double>();
	const auto xp = iterate<posit<32, 2>>();
	const auto sa = iterate<areal32>();
	const auto sp = iterate<poxel33>();
	const auto ia = iterate<IA>();
	const auto ip = iterate<IP>();

	std::cout << "Muller-Kahan: x_{n+1} = 111 - (1130 - 3000/x_{n-1}) / x_n, x_0 = 11/2, x_1 = 61/11\n\n";
	std::cout << std::setw(3) << "n" << std::setw(12) << "exact" << std::setw(12) << "float" << std::setw(12) << "double" << std::setw(12) << "posit32"
	          << std::setw(16) << "poxel33 tile" << "   tile_interval<poxel33>\n";
	for (std::size_t n = 0; n <= N; n += (n < 4 ? 1 : 2)) {
		std::cout << std::setw(3) << n << std::setprecision(7) << std::setw(12) << exact(n) << std::setw(12) << xf[n] << std::setw(12) << xd[n] << std::setw(12) << double(xp[n])
		          << std::setw(13) << double(sp[n]) << (ubit_of(sp[n]) ? " u=1" : " u=0") << "   " << ip[n].str(6) << '\n';
	}

	bool ia_contains = true, ip_contains = true, flagged = true;
	for (std::size_t n = 0; n <= N; ++n) {
		ia_contains = ia_contains && ia[n].contains(exact(n));
		ip_contains = ip_contains && ip[n].contains(exact(n));
		if (n >= 1) flagged = flagged && ubit_of(sa[n]) && ubit_of(sp[n]);
	}
	const auto false_convergence = [](double v) { return std::abs(v - 100.0) < 1.0e-3; };

	std::cout << "\nassertions:\n";
	check(false_convergence(xf[N]) && false_convergence(xd[N]) && false_convergence(double(xp[N])), "float, double and posit<32,2> all settle on 100, without warning");
	check(std::abs(exact(N) - 6.0) < 0.01, "the exact iterate is near 6");
	check(flagged, "both single tiles flag every iterate from x_1 on as inexact");
	check(false_convergence(double(sp[N])) && !(sp[N].lower<double>() <= 6.0 && 6.0 <= sp[N].upper<double>()), "the single poxel tile lands on 100 too: the flag warns, but the tile is not an enclosure");
	check(ia_contains && ip_contains, "both tile intervals contain the exact iterate at every step");
	check(!(ia[N].lower<double>() > 6.5) && !(ip[N].lower<double>() > 6.5), "neither tile interval excludes the true limit: no false convergence");

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
