// ubox_trace.cpp: watch the uncertainty box expand and contract through the quadratic formula
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
//
// #196.  The smaller root of 3x^2 + 100x + 2 = 0 (The End of Error, pp. 181-184; see also
// quadratic_roots.cpp), evaluated step by step in tile intervals of areal and poxel at 16
// and 32 bits, in two forms:
//     as written    r1 = (-b + sqrt(b^2 - 4ac)) / (2a)
//     rearranged    r1 = 2c / (-b - sqrt(b^2 - 4ac))
// and with two kinds of operands: exact a, b, c, and a, b, c known only to within one tile
// on either side.  Every intermediate box is recorded with universal/utility/ubox_trace.hpp
// and the run writes an HTML page that shows each box per step.  In the formula as written
// the box of -b + sqrt(d) jumps by orders of magnitude in relative width (cancellation) and
// never recovers; the rearranged form adds instead and keeps the box of sqrt(d).
//
// usage: ubox_trace [output.html]        (default: ubox_trace.html in the working directory)
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <universal/number/areal/areal.hpp>
#include <universal/number/poxel/poxel.hpp>
#include <universal/utility/tile_interval.hpp>
#include <universal/utility/ubox_trace.hpp>

namespace {

using sw::universal::tile_interval;
using sw::universal::tile_traits;
using sw::universal::ubox_box;
using sw::universal::ubox_series;
using sw::universal::ubox_trace;

// the exact value of every step for a = 3, b = 100, c = 2
const long double sd_ref = std::sqrt(9976.0L);
constexpr long double r1_ref = -0.020012014421636353441812506440166613377L;

void as_written_steps(ubox_trace& t) {
	t.step("a", 3); t.step("b", 100); t.step("c", 2);
	t.step("b*b", 10000); t.step("4*a*c", 24); t.step("d = b*b - 4*a*c", 9976);
	t.step("sqrt(d)", sd_ref);
	t.step("-b + sqrt(d)", -100.0L + sd_ref);
	t.step("2*a", 6);
	t.step("r1 = (-b + sqrt(d)) / (2*a)", r1_ref);
}
void rearranged_steps(ubox_trace& t) {
	t.step("a", 3); t.step("b", 100); t.step("c", 2);
	t.step("b*b", 10000); t.step("4*a*c", 24); t.step("d = b*b - 4*a*c", 9976);
	t.step("sqrt(d)", sd_ref);
	t.step("-b - sqrt(d)", -100.0L - sd_ref);
	t.step("2*c", 4);
	t.step("r1 = 2*c / (-b - sqrt(d))", r1_ref);
}

// the same expressions, in the same order, as quadratic_roots.cpp
template<typename I>
I as_written(ubox_series& s, const I& a, const I& b, const I& c) {
	s.record(a); s.record(b); s.record(c);
	const I bb = s.record(b * b);
	const I fac = s.record(I(4) * a * c);
	const I d = s.record(bb - fac);
	const I sd = s.record(sqrt(d));
	const I num = s.record(-b + sd);
	const I ta = s.record(I(2) * a);
	return s.record(num / ta);
}
template<typename I>
I rearranged(ubox_series& s, const I& a, const I& b, const I& c) {
	s.record(a); s.record(b); s.record(c);
	const I bb = s.record(b * b);
	const I fac = s.record(I(4) * a * c);
	const I d = s.record(bb - fac);
	const I sd = s.record(sqrt(d));
	const I den = s.record(-b - sd);
	const I tc = s.record(I(2) * c);
	return s.record(tc / den);
}

// v known to within one tile on either side: (prev(v), next(v)), three tiles
template<typename Tile>
tile_interval<Tile> wide(int v) {
	const auto k = tile_traits<Tile>::key(Tile(v));
	return tile_interval<Tile>::from_keys(k - 1, k + 1);
}

template<typename Tile>
void trace_type(const std::string& name, int digits, ubox_trace& tw, ubox_trace& tr, ubox_trace& uw, ubox_trace& ur) {
	using I = tile_interval<Tile>;
	const I a(3), b(100), c(2);
	as_written(tw.series(name, digits), a, b, c);
	rearranged(tr.series(name, digits), a, b, c);
	const I A = wide<Tile>(3), B = wide<Tile>(100), C = wide<Tile>(2);
	as_written(uw.series(name, digits), A, B, C);
	rearranged(ur.series(name, digits), A, B, C);
}

int fails = 0;
void check(bool ok, const std::string& what) {
	std::cout << (ok ? "  ok    " : "  FAIL  ") << what << '\n';
	if (!ok) ++fails;
}

// every box of the trace contains the exact value of its step
bool contains_references(const ubox_trace& t) {
	for (const auto& s : t.all_series())
		for (std::size_t i = 0; i < t.steps().size(); ++i)
			if (!s.boxes[i].contains(t.steps()[i].reference)) {
				std::cout << "        " << s.name << " step '" << t.steps()[i].label << "' " << s.boxes[i].text << " misses " << double(t.steps()[i].reference) << '\n';
				return false;
			}
	return true;
}
// wider operands give a wider box at every step (interval arithmetic is inclusion monotone)
bool contains_boxes(const ubox_trace& wider, const ubox_trace& narrower) {
	for (std::size_t k = 0; k < wider.all_series().size(); ++k) {
		const auto& W = wider.all_series()[k].boxes;
		const auto& N = narrower.all_series()[k].boxes;
		for (std::size_t i = 0; i < W.size(); ++i)
			if (W[i].lo > N[i].lo || W[i].hi < N[i].hi || W[i].tiles < N[i].tiles) return false;
	}
	return true;
}
const ubox_box& final_box(const ubox_trace& t, std::size_t k) { return t.all_series()[k].boxes.back(); }
const ubox_box& box_at(const ubox_trace& t, std::size_t k, const std::string& label) {
	for (std::size_t i = 0; i < t.steps().size(); ++i)
		if (t.steps()[i].label == label) return t.all_series()[k].boxes[i];
	throw std::runtime_error("no step " + label);
}

void summary(const ubox_trace& t) {
	std::cout << t.title() << '\n';
	for (const auto& s : t.all_series())
		std::cout << "    " << std::setw(12) << std::left << s.name << std::right << std::setw(46) << s.boxes.back().text << std::setw(8) << s.boxes.back().tiles << " tiles\n";
}

}  // anonymous namespace

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using areal16 = areal<16, 5, std::uint16_t>;
	using areal32 = areal<32, 8, std::uint32_t>;
	using poxel16 = poxel<16, 2, std::uint16_t>;
	using poxel32 = poxel<32, 2, std::uint32_t>;

	const std::string path = argc > 1 ? argv[1] : "ubox_trace.html";

	ubox_trace tw("r1 as written, exact a, b, c", "r1 = (-b + sqrt(b*b - 4*a*c)) / (2*a)   at a = 3, b = 100, c = 2",
	              "-b + sqrt(d) subtracts two numbers near 100 to get about -0.12: the absolute width of the box carries over, "
	              "so its relative width jumps by about three orders of magnitude, and the division by 2a cannot take it back.");
	ubox_trace tr("r1 rearranged, exact a, b, c", "r1 = 2*c / (-b - sqrt(b*b - 4*a*c))",
	              "Algebraically equal, but -b - sqrt(d) adds two numbers of the same sign: the relative width of sqrt(d) "
	              "survives to the root.");
	ubox_trace uw("r1 as written, a, b, c within one tile", "a, b, c = (prev(v), next(v)): three tiles each",
	              "Uncertain operands start the boxes wide.  a and b occur twice, and interval arithmetic treats each "
	              "occurrence as independent (the dependency problem), so the boxes grow beyond the true range of r1.");
	ubox_trace ur("r1 rearranged, a, b, c within one tile", "r1 = 2*c / (-b - sqrt(b*b - 4*a*c))",
	              "The rearranged form avoids the cancellation.  a and b still occur twice, so its box can be wider than the "
	              "true range of r1.  At 32 bits it ends in 9 to 15 tiles, against 8195 to 17753 as written.");
	as_written_steps(tw); rearranged_steps(tr); as_written_steps(uw); rearranged_steps(ur);

	trace_type<areal16>("areal<16,5>", 6, tw, tr, uw, ur);
	trace_type<poxel16>("poxel<16,2>", 6, tw, tr, uw, ur);
	trace_type<areal32>("areal<32,8>", 9, tw, tr, uw, ur);
	trace_type<poxel32>("poxel<32,2>", 9, tw, tr, uw, ur);

	const std::vector<ubox_trace> traces{ tw, tr, uw, ur };
	std::ostringstream page;
	write_html(page, "The uncertainty box through the quadratic formula",
	           "Each step of r1 for 3x^2 + 100x + 2 = 0 is evaluated in tile intervals: guaranteed enclosures built "
	           "from areal (float lattice) and poxel (posit lattice) tiles, at 16 and 32 bits.  The chart plots the "
	           "relative width of each box per step; the rows draw every type's box on one number line around the "
	           "exact value (dashed).  Solid caps are closed ends, dashed caps open ends; a box narrower than the "
	           "scale resolves is drawn as a thin mark.  Hover for the interval and its width in tiles.",
	           traces);
	std::ofstream f(path);
	f << page.str();
	f.close();

	std::cout << "the uncertainty box of r1 for 3x^2 + 100x + 2 = 0, step by step\n\n";
	for (const auto& t : traces) summary(t);
	std::cout << "\nwrote " << path << '\n';

	std::cout << "\nassertions:\n";
	check(f.good() && page.str().size() > 10000, "the HTML page was written");
	std::size_t svgs = 0;
	for (std::size_t p = page.str().find("<svg"); p != std::string::npos; p = page.str().find("<svg", p + 1)) ++svgs;
	check(svgs == 4 * (1 + 10), "the page has a chart and ten step strips per trace");
	check(contains_references(tw) && contains_references(tr) && contains_references(uw) && contains_references(ur),
	      "every box at every step contains the exact value of that step");
	check(contains_boxes(uw, tw) && contains_boxes(ur, tr), "operands one tile wide give a box at least as wide at every step as exact operands");
	bool narrower = true, jump = true;
	for (std::size_t k = 0; k < 4; ++k) {
		narrower = narrower && final_box(tr, k).tiles < final_box(tw, k).tiles && final_box(ur, k).tiles < final_box(uw, k).tiles;
		jump = jump && box_at(tw, k, "-b + sqrt(d)").relative_width() > 100 * box_at(tw, k, "sqrt(d)").relative_width();
	}
	check(narrower, "the rearranged r1 ends in fewer tiles than r1 as written, for every type and both kinds of operands");
	check(jump, "cancellation: the relative width of -b + sqrt(d) is more than 100 times that of sqrt(d)");
	// the same expressions as quadratic_roots.cpp, so the same tile counts (README: 1365+ tiles as written, 3 rearranged)
	check(final_box(tw, 2).tiles >= 1365 && final_box(tw, 3).tiles >= 1365 && final_box(tr, 2).tiles <= 3 && final_box(tr, 3).tiles <= 3,
	      "at 32 bits r1 as written spans 1365+ tiles and the rearranged r1 at most 3, as quadratic_roots.cpp reports");

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
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
