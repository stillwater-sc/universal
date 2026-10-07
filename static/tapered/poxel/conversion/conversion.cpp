// conversion.cpp: the poxel encoding and conversion, against the independent posit lattice
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A poxel<nbits, es> is the lattice of a posit<nbits - 1, es> plus a ubit.  The reference
// for every lattice value is Universal's own posit, which shares no code with the poxel.
// For every tile of each small configuration:
//   - the lower endpoint is the posit value at the tile's lattice index, and the upper
//     endpoint of an open tile is the next posit value (inf above maxpos)
//   - tiles are ordered: lower(T) <= upper(T) <= lower(T + 1)
//   - negation is two's-complement negation of the pattern, for exact and open tiles
// and for conversion from double:
//   - every lattice value converts to its exact tile
//   - every midpoint, and a hair either side of every lattice value, converts to the
//     unique open tile that contains it -- never rounded
//   - above maxpos, below minpos and their negatives land on the four end tiles
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <universal/number/poxel/poxel.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<unsigned nbits, unsigned es>
int VerifyTiles(bool report) {
	using X = sw::universal::poxel<nbits, es, std::uint16_t>;
	using P = sw::universal::posit<nbits - 1, es, std::uint16_t>;
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(X{}) << ": " << what << '\n'; };
	auto posit_value = [](std::int64_t lattice) {
		P p;
		p.setbits(static_cast<std::uint64_t>(lattice) & ((1ull << (nbits - 1)) - 1ull));
		return p.isnar() ? -INFINITY : double(p);
	};
	const std::int64_t half = std::int64_t(1) << (nbits - 1);
	double previous_upper = -INFINITY;
	for (std::int64_t t = -half; t < half; ++t) {
		X x;
		x.setbits(static_cast<std::uint64_t>(t));
		if (x.isnar()) continue;
		const std::int64_t L = t >> 1;
		const bool u = (t & 1) != 0;
		if (x.lattice() != L || x.ubit() != u) fail("pattern " + std::to_string(t) + " decodes to the wrong lattice index or ubit");
		const double lo = x.template lower<double>(), hi = x.template upper<double>();
		if (lo != posit_value(L)) fail("lower endpoint of tile " + std::to_string(t));
		const double want_hi = !u ? lo : (L == X::lattice_maxpos ? INFINITY : posit_value(L + 1));
		if (hi != want_hi) fail("upper endpoint of tile " + std::to_string(t));
		if (u ? !(lo < hi) : (lo != hi)) fail("tile " + std::to_string(t) + " is not a point or an open interval");
		if (!(lo >= previous_upper)) fail("tiles are not ordered at " + std::to_string(t));
		previous_upper = hi;
		// negation: two's complement of the pattern
		const X n = -x;
		const double nlo = n.template lower<double>(), nhi = n.template upper<double>();
		if (nlo != -hi || nhi != -lo || n.ubit() != u) fail("negation of tile " + std::to_string(t));
	}
	return fails;
}

template<unsigned nbits, unsigned es>
int VerifyConversion(bool report) {
	using X = sw::universal::poxel<nbits, es, std::uint16_t>;
	using P = sw::universal::posit<nbits - 1, es, std::uint16_t>;
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report && fails < 10) std::cerr << "FAIL " << sw::universal::type_tag(X{}) << ": " << what << '\n'; };
	// a real v lands in a tile that contains it: exactly, or strictly inside an open one
	auto contains = [](const X& x, double v) {
		const double lo = x.template lower<double>(), hi = x.template upper<double>();
		return x.isexact() ? (lo == v) : (lo < v && v < hi);
	};
	const unsigned N = 1u << (nbits - 1);
	for (unsigned p = 0; p < N; ++p) {
		P pv; pv.setbits(p);
		if (pv.isnar()) continue;
		const double v = double(pv);
		const X x(v);
		if (!x.isexact() || x.template lower<double>() != v) fail("lattice value " + std::to_string(v) + " is not an exact tile");
		for (double probe : { std::nextafter(v, INFINITY), std::nextafter(v, -INFINITY) }) {
			if (probe == 0.0) continue;
			const X y(probe);
			if (!contains(y, probe) || y.isexact()) fail("a hair from " + std::to_string(v) + " is not in an open tile containing it");
		}
		P next(pv); ++next;
		if (!next.isnar() && double(next) > v) {
			const double mid = 0.5 * (v + double(next));
			const X m(mid);
			if (!contains(m, mid) || m.isexact() || m.template lower<double>() != v) fail("midpoint above " + std::to_string(v) + " is not in the open tile above it");
		}
	}
	// the end tiles
	const double maxpos = double(X(sw::universal::SpecificValue::maxpos)), minpos = double(X(sw::universal::SpecificValue::minpos));
	if (X(maxpos * 4.0) != X(sw::universal::SpecificValue::infpos)) fail("above maxpos is not (maxpos, inf)");
	if (X(-maxpos * 4.0) != X(sw::universal::SpecificValue::infneg)) fail("below -maxpos is not (-inf, -maxpos)");
	if (!contains(X(minpos / 4.0), minpos / 4.0) || X(minpos / 4.0).template lower<double>() != 0.0) fail("below minpos is not (0, minpos)");
	if (!contains(X(-minpos / 4.0), -minpos / 4.0)) fail("above -minpos is not (-minpos, 0)");
	if (X(INFINITY) != X(sw::universal::SpecificValue::infpos) || X(-INFINITY) != X(sw::universal::SpecificValue::infneg)) fail("infinities map to the end tiles");
	if (!X(std::nan("")).isnar()) fail("NaN converts to NaR");
	if (!X(0.0).iszero() || !X(-0.0).iszero()) fail("zero converts to the exact zero tile");
	for (int k = -300; k <= 300; ++k) if (X(k) != X(double(k))) { fail("integer " + std::to_string(k) + " converts differently from its double"); break; }
	return fails;
}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "poxel encoding and conversion";
	std::string test_tag    = "conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyTiles<8, 0>(reportTestCases),       "poxel<8,0>",  "tiles");
	nrOfFailedTestCases += ReportTestResult(VerifyTiles<9, 2>(reportTestCases),       "poxel<9,2>",  "tiles");
	nrOfFailedTestCases += ReportTestResult(VerifyTiles<10, 1>(reportTestCases),      "poxel<10,1>", "tiles");
	nrOfFailedTestCases += ReportTestResult(VerifyTiles<12, 3>(reportTestCases),      "poxel<12,3>", "tiles");
	nrOfFailedTestCases += ReportTestResult(VerifyTiles<16, 2>(reportTestCases),      "poxel<16,2>", "tiles");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<8, 0>(reportTestCases),  "poxel<8,0>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<9, 2>(reportTestCases),  "poxel<9,2>",  test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 1>(reportTestCases), "poxel<10,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 3>(reportTestCases), "poxel<12,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 2>(reportTestCases), "poxel<16,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
