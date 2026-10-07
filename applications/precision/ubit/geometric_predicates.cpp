// geometric_predicates.cpp: the sign of det(M^k) for a nearly singular M
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
//
// #1637, test 3.  M = [ 1.61803398875  1 ; 1  0.61803398875 ], the golden-ratio matrix with its
// entries cut to 11 decimals.  With the exact golden ratio det(M) would be 0; with these
// decimals det(M) = +2.351265625e-13 exactly, so det(M^k) = det(M)^k > 0 for every k --
// about 1e-631 at k = 50, far below the range of every format here.  An orientation
// predicate needs only the sign.
//
// M^k is formed by repeated multiplication and det = m00 m11 - m01 m10, as a geometry code
// would.  The entries grow like 2.618^k while the determinant shrinks like 1e-13^k.
//
//   - double gets k = 1 right, then fails with full confidence.  How depends on whether the
//     compiler fuses m00 m11 - m01 m10 into a multiply-add: fused, it returns exactly 0
//     ("singular") from k = 5; unfused, it reports the wrong sign at k = 20 and about
//     +1e18 at k = 50
//   - a tile_interval never asserts a wrong sign: it decides when its precision allows and
//     says undecidable otherwise; 64-bit tiles decide k = 1, where 32-bit tiles cannot
//   - a single ubit tile is a flag only: it may even report a sign, which is not a proof
#include <universal/utility/directives.hpp>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <universal/number/areal/areal.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/utility/tile_interval.hpp>

namespace {

template<typename Real> using Matrix = std::array<Real, 4>;   // m00 m01 m10 m11

template<typename Real>
Matrix<Real> multiply(const Matrix<Real>& a, const Matrix<Real>& b) {
	return { a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3], a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3] };
}
template<typename Real> Real det(const Matrix<Real>& m) { return m[0] * m[3] - m[1] * m[2]; }

// the entries as the decimals 161803398875e-11 and 61803398875e-11: rounded by a rounding
// format, enclosed by a tile interval
template<typename Real>
Matrix<Real> golden() {
	const Real phi = Real(161803398875.0) / Real(1.0e11), phi1 = Real(61803398875.0) / Real(1.0e11);
	return { phi, Real(1), Real(1), phi1 };
}

constexpr int ks[] = { 1, 2, 5, 10, 20, 50 };

// det(M^k) for each k in ks
template<typename Real>
std::array<Real, 6> dets() {
	std::array<Real, 6> d{};
	const Matrix<Real> m = golden<Real>();
	Matrix<Real> p = m;
	int j = 0;
	for (int k = 1; k <= 50; ++k) {
		if (k > 1) p = multiply(p, m);
		if (k == ks[j]) d[static_cast<std::size_t>(j++)] = det(p);
	}
	return d;
}

const char* sign_of(double v) { return v > 0 ? "positive" : v < 0 ? "negative" : "zero"; }

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

	const auto df = dets<float>();
	const auto dd = dets<double>();
	const auto dp = dets<posit<32, 2>>();
	const auto sp = dets<poxel32>();
	const auto ia = dets<tile_interval<areal32>>();
	const auto ip = dets<tile_interval<poxel32>>();
	const auto ia6 = dets<tile_interval<areal64>>();
	const auto ip6 = dets<tile_interval<poxel64>>();

	std::cout << "sign of det(M^k), M = [1.61803398875 1; 1 0.61803398875]: exactly positive for every k\n\n";
	std::cout << std::setw(4) << "k" << std::setw(11) << "float" << std::setw(11) << "double" << std::setw(11) << "posit32" << std::setw(16) << "poxel32 tile"
	          << std::setw(13) << "TI areal32" << std::setw(13) << "TI poxel32" << std::setw(13) << "TI areal64" << std::setw(13) << "TI poxel64" << '\n';
	for (std::size_t j = 0; j < 6; ++j) {
		std::cout << std::setw(4) << ks[j] << std::setw(11) << sign_of(df[j]) << std::setw(11) << sign_of(dd[j]) << std::setw(11) << sign_of(double(dp[j]))
		          << std::setw(12) << sign_of(double(sp[j])) << (sp[j].ubit() ? " u=1" : " u=0")
		          << std::setw(13) << to_string(ia[j].sign()) << std::setw(13) << to_string(ip[j].sign()) << std::setw(13) << to_string(ia6[j].sign()) << std::setw(13) << to_string(ip6[j].sign()) << '\n';
	}
	std::cout << "\ndouble det(M^k):";
	for (std::size_t j = 0; j < 6; ++j) std::cout << ' ' << dd[j];
	std::cout << "\ntile_interval<poxel<64,2>> det(M):   " << ip6[0].str(6) << '\n';
	std::cout << "tile_interval<poxel<32,2>> det(M^50): " << ip[5].str(6) << '\n';

	bool never_wrong = true;
	for (std::size_t j = 0; j < 6; ++j) {
		for (tile_verdict v : { ia[j].sign(), ip[j].sign(), ia6[j].sign(), ip6[j].sign() }) never_wrong = never_wrong && (v == tile_verdict::positive || v == tile_verdict::undecidable);
	}

	std::cout << "\nassertions:\n";
	check(dd[0] > 0.0, "double decides k = 1 correctly");
	bool double_denies = false;   // zero or negative at some k >= 5: fused gives 0, unfused a negative sign
	for (std::size_t j = 2; j < 6; ++j) double_denies = double_denies || !(dd[j] > 0.0);
	check(double_denies, "double reports det(M^k) <= 0 for some k >= 5, without warning");
	check(never_wrong, "no tile interval ever asserts a wrong sign: positive, or undecidable");
	check(ip6[0].sign() == tile_verdict::positive && ia6[0].sign() == tile_verdict::positive, "both 64-bit tile intervals prove det(M) > 0");
	check(ip[0].sign() == tile_verdict::undecidable && ia[0].sign() == tile_verdict::undecidable, "32-bit tile intervals honestly cannot resolve 2.35e-13 next to entries near 1");
	check(ia[5].sign() == tile_verdict::undecidable && ip[5].sign() == tile_verdict::undecidable && ia6[5].sign() == tile_verdict::undecidable && ip6[5].sign() == tile_verdict::undecidable,
	      "at k = 50 every tile interval is undecidable: the value is 1e-631");

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
