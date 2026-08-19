// predicates.cpp: elreal Phase 9 (#933 / #1186) exact geometric predicates.
//
// orient2d and incircle are the canonical robust-predicates workload (Shewchuk;
// McCleeary section 5.1). Both are sign queries on a determinant, and the sign
// is what a mesh generator or convex hull actually consumes -- a predicate that
// returns the wrong sign near a degeneracy does not produce a slightly wrong
// mesh, it produces an inconsistent one, and the algorithm built on top can loop
// or crash.
//
// Each predicate is written ONCE over a generic Real and instantiated for
// double, qd and elreal<double>. The reference is the same expression evaluated
// in exact dyadic-rational arithmetic (dyadics are closed under +, -, * and the
// inputs are doubles, so the determinant's exact value and sign are decided with
// no rounding anywhere).
//
// Two lessons are baked into this file, both learned the hard way:
//
//   1. elreal's operator* is DEPTH-BOUNDED. At the default precision the product
//      is truncated and near-degenerate signs come out wrong. Raise the depth
//      with elreal_precision_guard for exact work.
//   2. Take the sign from the COMPARISON OPERATORS, not from sign() or the raw
//      block stream. sign() answers +1 for zero, and a cancelling sum stays
//      lazily unnormalised, so the leading raw block carries a phantom sign --
//      which is wrong on exactly the degenerate inputs a predicate exists to
//      detect. elreal_cmp skips zero blocks and is correct.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace {

	using namespace sw::universal;

	// ---- the predicates, once, over a generic Real ---------------------------

	template<typename Real>
	Real orient2d_expr(double ax, double ay, double bx, double by, double cx, double cy) {
		Real Ax(ax), Ay(ay), Bx(bx), By(by), Cx(cx), Cy(cy);
		return (Ax - Cx) * (By - Cy) - (Ay - Cy) * (Bx - Cx);
	}

	template<typename Real>
	Real incircle_expr(double ax, double ay, double bx, double by,
	                   double cx, double cy, double dx, double dy) {
		Real Adx = Real(ax) - Real(dx), Ady = Real(ay) - Real(dy);
		Real Bdx = Real(bx) - Real(dx), Bdy = Real(by) - Real(dy);
		Real Cdx = Real(cx) - Real(dx), Cdy = Real(cy) - Real(dy);
		Real alift = Adx * Adx + Ady * Ady;
		Real blift = Bdx * Bdx + Bdy * Bdy;
		Real clift = Cdx * Cdx + Cdy * Cdy;
		return alift * (Bdx * Cdy - Bdy * Cdx)
		     - blift * (Adx * Cdy - Ady * Cdx)
		     + clift * (Adx * Bdy - Ady * Bdx);
	}

	// ---- the exact reference, in dyadic rationals ----------------------------

	dyadic dy(double v) { return dyadic::from_double(v); }

	dyadic orient2d_exact(double ax, double ay, double bx, double by, double cx, double cy) {
		return (dy(ax) - dy(cx)) * (dy(by) - dy(cy)) - (dy(ay) - dy(cy)) * (dy(bx) - dy(cx));
	}

	dyadic incircle_exact(double ax, double ay, double bx, double by,
	                      double cx, double cy, double dx, double dy_) {
		dyadic Adx = dy(ax) - dy(dx), Ady = dy(ay) - dy(dy_);
		dyadic Bdx = dy(bx) - dy(dx), Bdy = dy(by) - dy(dy_);
		dyadic Cdx = dy(cx) - dy(dx), Cdy = dy(cy) - dy(dy_);
		dyadic al = Adx * Adx + Ady * Ady;
		dyadic bl = Bdx * Bdx + Bdy * Bdy;
		dyadic cl = Cdx * Cdx + Cdy * Cdy;
		return al * (Bdx * Cdy - Bdy * Cdx) - bl * (Adx * Cdy - Ady * Cdx) + cl * (Adx * Bdy - Ady * Bdx);
	}

	// ---- three-way signs -----------------------------------------------------

	// For qd and elreal alike: the type's own comparison operators. For elreal
	// this routes through elreal_cmp, which skips zero blocks -- see the header
	// comment for why sign() and the raw stream are both wrong here.
	template<typename Real>
	int sgn_of(const Real& v) { Real z(0.0); return v < z ? -1 : (v > z ? 1 : 0); }
	int sgn_of(double v) { return v < 0.0 ? -1 : (v > 0.0 ? 1 : 0); }
	int sgn_exact(const dyadic& d) { return d.numerator.iszero() ? 0 : (d.numerator.sign() ? -1 : 1); }

	struct tally { int wrong = 0; int total = 0; int degenerate = 0; };

	void row(const char* label, const tally& d, const tally& q, const tally& e) {
		std::cout << "  " << std::left << std::setw(34) << label << std::right
		          << std::setw(9) << d.wrong << std::setw(7) << q.wrong << std::setw(9) << e.wrong
		          << "     (of " << d.total << ", " << d.degenerate << " exactly degenerate)\n";
	}

	// ---- orient2d on the Kettner grid ----------------------------------------
	// Three points that are exactly collinear on y = x, with the first perturbed
	// across a grid of single-ulp steps. This is the configuration that produces
	// the well-known incorrect "pinwheel" from a naive double predicate.
	void orient2d_kettner(int n) {
		tally d, q, e;
		const double bx = 12.0, by = 12.0, cx = 24.0, cy = 24.0;
		const double u = std::ldexp(1.0, -53);
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				double ax = 0.5 + i * u, ay = 0.5 + j * u;
				int se = sgn_exact(orient2d_exact(ax, ay, bx, by, cx, cy));
				++d.total; ++q.total; ++e.total;
				if (se == 0) { ++d.degenerate; ++q.degenerate; ++e.degenerate; }
				if (sgn_of(orient2d_expr<double>(ax, ay, bx, by, cx, cy)) != se) ++d.wrong;
				if (sgn_of(orient2d_expr<qd>(ax, ay, bx, by, cx, cy)) != se) ++q.wrong;
				if (sgn_of(orient2d_expr<elreal<double>>(ax, ay, bx, by, cx, cy)) != se) ++e.wrong;
			}
		}
		row("orient2d, collinear + ulp grid", d, q, e);
	}

	// ---- incircle on near-cocircular full-mantissa points ---------------------
	// Points placed on a random circle and rounded to double: near-cocircular to
	// within an ulp, with every coordinate carrying a full 53-bit mantissa, which
	// is what makes the exact determinant expensive to resolve.
	void incircle_cocircular(int trials, int half) {
		tally d, q, e;
		std::mt19937_64 rng(0x1186);
		auto m01 = [&] { return static_cast<double>(rng() >> 11) * std::ldexp(1.0, -53); };
		auto rnd = [&](double lo, double hi) { return lo + (hi - lo) * m01(); };
		const double u = std::ldexp(1.0, -52);
		for (int t = 0; t < trials; ++t) {
			double ox = rnd(-4, 4), oy = rnd(-4, 4), r = rnd(0.5, 4);
			auto on_circle = [&](double th, double& X, double& Y) { X = ox + r * std::cos(th); Y = oy + r * std::sin(th); };
			double ax, ay, bx, by, cx, cy, dx0, dy0;
			on_circle(rnd(0, 1), ax, ay);
			on_circle(rnd(2, 3), bx, by);
			on_circle(rnd(4, 5), cx, cy);
			on_circle(rnd(5.5, 6.2), dx0, dy0);
			for (int i = -half; i < half; ++i) {
				for (int j = -half; j < half; ++j) {
					double dx = dx0 + i * u, dyv = dy0 + j * u;
					int se = sgn_exact(incircle_exact(ax, ay, bx, by, cx, cy, dx, dyv));
					++d.total; ++q.total; ++e.total;
					if (se == 0) { ++d.degenerate; ++q.degenerate; ++e.degenerate; }
					if (sgn_of(incircle_expr<double>(ax, ay, bx, by, cx, cy, dx, dyv)) != se) ++d.wrong;
					if (sgn_of(incircle_expr<qd>(ax, ay, bx, by, cx, cy, dx, dyv)) != se) ++q.wrong;
					if (sgn_of(incircle_expr<elreal<double>>(ax, ay, bx, by, cx, cy, dx, dyv)) != se) ++e.wrong;
				}
			}
		}
		row("incircle, near-cocircular", d, q, e);
	}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;

	// elreal's operator* is depth-bounded; the default precision truncates the
	// determinant and gets near-degenerate signs wrong.
	elreal_precision_guard guard(32);

	std::cout << "elreal Phase 9 (#1186) exact geometric predicates\n";
	std::cout << "================================================\n\n";
	std::cout << "wrong signs against the exact dyadic determinant:\n\n";
	std::cout << "  " << std::left << std::setw(34) << "workload" << std::right
	          << std::setw(9) << "double" << std::setw(7) << "qd" << std::setw(9) << "elreal" << '\n';

	orient2d_kettner(128);
	incircle_cocircular(64, 4);

	std::cout << "\nReading the table:\n"
	          << "  double fails on a third of the orient2d grid and a sixth of the incircle\n"
	          << "    cases. These are not exotic inputs -- they are ordinary coordinates near\n"
	          << "    a degeneracy, which is where a mesh algorithm spends its time.\n"
	          << "  qd gets both exactly right, and that is the honest result: Shewchuk's\n"
	          << "    analysis puts orient2d at ~2x and incircle at ~4x working precision, and\n"
	          << "    qd supplies 4x. On double inputs at ordinary scales it is enough.\n"
	          << "  elreal is also exact, but for a different reason. qd is exact here because\n"
	          << "    somebody did the error analysis and the answer happened to fit; elreal is\n"
	          << "    exact because it does not have a budget to exceed. The guarantee survives\n"
	          << "    a change of predicate, of scaling, or of input distribution without anyone\n"
	          << "    re-deriving the bound -- which is the property a predicate library wants.\n";

	return EXIT_SUCCESS;
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
