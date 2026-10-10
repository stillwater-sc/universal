#pragma once
// oracle.hpp: the tightest box a tile type can state for a computation (#1654, Phase 3)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// ubox reports the box a tile interval type computes.  This header computes, independently,
// the box the type could state at best, so the two can be compared: the excess is what the
// formula costs (the dependency problem), the rest is what the inputs force.
//
// The expression is evaluated in the exact dyadic intervals of tile_oracle.hpp, with the
// session's definitions replayed as ubox does.  A literal is its exact decimal value, x~ the
// open tile above x in the target type's lattice, [a, b] the closed set.
//
//   point inputs   The value is one real number, so the tightest box is one tile.  The working
//                  precision rises until the value's interval lies in one tile.  A non-dyadic
//                  route to a lattice point -- (1/3) * 3 -- never separates from it; the count
//                  is still one tile, and the report says its place is unresolved.
//
//   set inputs     The tightest box is the tile hull of the image of the inputs' closure (an
//                  open input such as x~ is taken with its ends, as #1649's corner polynomials
//                  are).  Each value carries an interval gradient over the inputs (forward
//                  automatic differentiation), so monotonicity is PROVEN, not assumed:
//                  - a piece of the input box on which every partial derivative has one sign
//                    maps onto the interval between two corners, whose values the oracle
//                    evaluates exactly;
//                  - any other piece is enclosed by the naive interval form intersected with
//                    the mean-value form f(c) + sum g(piece) (piece - c), whose excess shrinks
//                    quadratically; its centre c is a value the computation takes.  A piece
//                    whose enclosure lies inside the values already taken is dropped, any
//                    other is bisected along the input contributing most, width x |df/dx|;
//                  - pieces still undecided when the budget is spent contribute their interval
//                    enclosure.
//                  The corners give an INNER bound (values the computation takes) and the
//                  pieces an OUTER bound.  When they meet, the tightest box is proven.
//
// IMPORTANT: include after expression.hpp and tiles.hpp.
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <universal/utility/tile_oracle.hpp>

namespace sw { namespace ucalc {

namespace oracle_detail {

namespace orc = sw::universal::oracle;

// an interval value and its interval gradient over the uncertain inputs (empty: all zero)
struct dual {
	orc::interval v;
	std::vector<orc::interval> g;
};

inline Value make_value(const dual& x) {
	Value v;
	v.native = x;
	v.type_name = "oracle";
	v.native_rep = "<exact>";
	return v;
}

inline dual of(const Value& v) {
	if (const auto* p = std::any_cast<dual>(&v.native)) return *p;
	if (!v.native.has_value() && std::isfinite(v.num)) return { orc::point(orc::from_double(v.num)), {} };
	return { orc::entire(), {} };   // a value from another type: nothing exact is known
}

inline orc::interval zero() { return orc::point(orc::dyadic{}); }

// a o b on gradients of possibly different (empty = zero) lengths
template<typename F>
std::vector<orc::interval> combine(const std::vector<orc::interval>& a, const std::vector<orc::interval>& b, F f) {
	const std::size_t n = std::max(a.size(), b.size());
	std::vector<orc::interval> r(n);
	for (std::size_t i = 0; i < n; ++i) r[i] = f(i < a.size() ? a[i] : zero(), i < b.size() ? b[i] : zero());
	return r;
}
inline std::vector<orc::interval> scale(const std::vector<orc::interval>& g, const orc::interval& s) {
	std::vector<orc::interval> r;
	r.reserve(g.size());
	for (const auto& x : g) r.push_back(orc::mul(x, s));
	return r;
}
inline std::vector<orc::interval> divide(const std::vector<orc::interval>& g, const orc::interval& s, int P) {
	std::vector<orc::interval> r;
	r.reserve(g.size());
	for (const auto& x : g) r.push_back(orc::div(x, s, P));
	return r;
}

// a qd value, exactly: the sum of its four limbs
inline orc::dyadic exact_qd(const sw::universal::qd& q) {
	orc::dyadic s;
	for (int i = 0; i < 4; ++i) if (q[i] != 0.0) s = orc::add(s, orc::from_double(q[i]));
	return s;
}

// The oracle's arithmetic as a TypeOps, for Tile's lattice and a working precision P.
template<typename Tile>
TypeOps oracle_ops(int P) {
	using T = sw::universal::tile_traits<Tile>;
	TypeOps ops;
	ops.name = "oracle";
	ops.type_tag = "oracle";
	ops.max_digits10 = 17;
	ops.nbits = 0;
	ops.family = "oracle";
	auto mk = [](const dual& x) { return make_value(x); };
	auto entire = [](const Value&) { return make_value(dual{ orc::entire(), {} }); };

	ops.from_double = [mk](double v) { return mk({ std::isfinite(v) ? orc::point(orc::from_double(v)) : orc::entire(), {} }); };
	ops.from_literal = [mk, P](const std::string& text) {
		bool ok = false;
		const orc::interval x = orc::from_decimal(text, P, ok);
		if (!ok && text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
			orc::Big m(0);
			for (std::size_t i = 2; i < text.size(); ++i) {
				const int c = std::tolower(static_cast<unsigned char>(text[i]));
				m *= 16;
				m += static_cast<long long>(std::isdigit(c) ? c - '0' : c - 'a' + 10);
			}
			return mk({ orc::point(orc::trim({ m, 0 })), {} });
		}
		return mk({ ok ? x : orc::entire(), {} });
	};
	ops.constant = [mk](const std::string& cname) {
		const sw::universal::qd q = HighPrecisionConstants::lookup(cname);
		if (q.isnan()) return mk({ orc::nan_interval(), {} });
		const orc::dyadic v = exact_qd(q);
		orc::dyadic margin = v;                  // |v| 2^-180: qd's ~212 bits hold the constant well within it
		margin.e -= 180;
		if (orc::sign(margin) < 0) margin = orc::neg(margin);
		return mk({ orc::interval{ orc::sub(v, margin), orc::add(v, margin) }, {} });
	};
	// x~: the open tile above x, when x is a lattice point of Tile; otherwise x itself
	ops.above = [mk](const Value& a) {
		const dual x = of(a);
		if (!orc::is_point(x.v)) return mk(x);
		const std::int64_t k = orc::locate<Tile>(x.v.lo);
		if ((k & 1) != 0 || k >= T::kmax - 1) return mk(x);
		orc::interval r{ x.v.lo, orc::lattice_point<Tile>(k + 2) };
		r.lo_open = r.hi_open = true;
		return mk({ r, {} });
	};
	ops.hull = [mk](const Value& a, const Value& b) { return mk({ orc::hull(of(a).v, of(b).v), {} }); };

	auto plus  = [](const orc::interval& p, const orc::interval& q) { return orc::add(p, q); };
	auto minus = [](const orc::interval& p, const orc::interval& q) { return orc::sub(p, q); };
	ops.add = [mk, plus](const Value& a, const Value& b) {
		const dual x = of(a), y = of(b);
		return mk({ orc::add(x.v, y.v), combine(x.g, y.g, plus) });
	};
	ops.sub = [mk, minus](const Value& a, const Value& b) {
		const dual x = of(a), y = of(b);
		return mk({ orc::sub(x.v, y.v), combine(x.g, y.g, minus) });
	};
	ops.mul = [mk, plus](const Value& a, const Value& b) {               // (xy)' = x'y + xy'
		const dual x = of(a), y = of(b);
		return mk({ orc::mul(x.v, y.v), combine(scale(x.g, y.v), scale(y.g, x.v), plus) });
	};
	ops.div = [mk, minus, P](const Value& a, const Value& b) {           // (x/y)' = (x' - (x/y) y') / y
		const dual x = of(a), y = of(b);
		const orc::interval q = orc::div(x.v, y.v, P);
		return mk({ q, divide(combine(x.g, scale(y.g, q), minus), y.v, P) });
	};
	ops.negate = [mk](const Value& a) {
		const dual x = of(a);
		return mk({ orc::neg(x.v), scale(x.g, orc::point(orc::make_dyadic(-1))) });
	};
	ops.fn_sqrt = [mk, P](const Value& a) {                               // sqrt(x)' = x' / (2 sqrt(x))
		const dual x = of(a);
		const orc::interval r = orc::sqrt(x.v, P);
		return mk({ r, divide(x.g, orc::mul(orc::point(orc::make_dyadic(2)), r), P) });
	};
	ops.fn_abs = [mk](const Value& a) {
		const dual x = of(a);
		orc::interval s = orc::point(orc::make_dyadic(1));
		if (!x.v.hi_inf && orc::sign(x.v.hi) <= 0) s = orc::point(orc::make_dyadic(-1));
		else if (x.v.lo_inf || orc::sign(x.v.lo) < 0) s = orc::interval{ orc::make_dyadic(-1), orc::make_dyadic(1) };
		return mk({ orc::abs(x.v), scale(x.g, s) });
	};
	ops.fn_pow = [mk, P](const Value& a, const Value& b) {                // (x^n)' = n x^(n-1) x'
		const dual x = of(a), e = of(b);
		if (orc::is_point(e.v) && e.v.lo.e >= 0 && orc::msb(e.v.lo.m) + e.v.lo.e < 11) {   // an integer below 2^11
			orc::Big nb = e.v.lo.m;
			nb <<= e.v.lo.e;
			const long long n = static_cast<long long>(nb);
			const orc::interval d = orc::mul(orc::point(orc::make_dyadic(n)), orc::pow_int(x.v, n - 1, P));
			return mk({ orc::pow_int(x.v, n, P), scale(x.g, d) });
		}
		return mk({ orc::entire(), std::vector<orc::interval>(x.g.size(), orc::entire()) });
	};
	// no exact enclosure: the entire line, which makes the tightest box unavailable
	ops.fn_log = ops.fn_exp = ops.fn_sin = ops.fn_cos = ops.fn_tan = entire;
	ops.fn_asin = ops.fn_acos = ops.fn_atan = entire;
	return ops;
}

// a definition is an uncertain input when it writes a set: x~, [a, b], hull(), above()
inline bool is_input_definition(const std::string& text) {
	return text.find('~') != std::string::npos || text.find('[') != std::string::npos
	    || text.find("hull(") != std::string::npos || text.find("above(") != std::string::npos;
}

// Evaluate expr with the session's definitions replayed, the last definition of each input
// name replaced by the given value.
inline dual evaluate(const TypeOps& ops, const ExpressionEvaluator& session, const std::string& expr,
                     const std::map<std::size_t, dual>& overrides) {
	ExpressionEvaluator eval(ops);
	eval.inherit(session);
	const auto& defs = session.definitions();
	for (std::size_t i = 0; i < defs.size(); ++i) {
		const auto it = overrides.find(i);
		if (it != overrides.end()) eval.set_variable(defs[i].first, make_value(it->second));
		else eval.evaluate(defs[i].first + " =" + defs[i].second);
	}
	return of(eval.evaluate(expr));
}

inline orc::dyadic midpoint(const orc::interval& x) {
	orc::dyadic h = orc::add(x.lo, x.hi);
	h.e -= 1;
	return h;
}

}  // namespace oracle_detail

// The tightest box Tile can state for expr over the session's inputs.
template<typename Tile>
TightestReport tightest_box(const ExpressionEvaluator& session, const std::string& expr, int budget = 2000) {
	namespace orc = sw::universal::oracle;
	namespace od = oracle_detail;
	using I = sw::universal::tile_interval<Tile>;
	using Overrides = std::map<std::size_t, od::dual>;
	TightestReport rep;
	// the tile of a value the computation takes, raising the precision until it is one tile
	auto point_box = [&](const Overrides& at) {
		I b;
		for (int P = 128; P <= 2048; P *= 2) {
			b = orc::box<Tile>(od::evaluate(od::oracle_ops<Tile>(P), session, expr, at).v);
			if (b.isnan() || b.lo_key() == b.hi_key()) break;
		}
		return b;
	};
	try {
		// the uncertain inputs: the last definition of each name that writes a set
		const auto& defs = session.definitions();
		std::vector<std::size_t> inputs;
		for (std::size_t i = 0; i < defs.size(); ++i) {
			bool last = true;
			for (std::size_t j = i + 1; j < defs.size(); ++j) if (defs[j].first == defs[i].first) last = false;
			if (last && od::is_input_definition(defs[i].second)) inputs.push_back(i);
		}
		rep.inputs = static_cast<int>(inputs.size());

		if (inputs.empty()) {
			const I box = point_box({});
			if (box.isnan()) { rep.note = "the value is not a real number"; return rep; }
			if (box.lo_key() == -I::kmax && box.hi_key() == I::kmax) { rep.note = "no exact enclosure: a function the oracle cannot bound (log, exp, trigonometric), or a division by a set holding zero"; return rep; }
			rep.available = true;
			rep.inner_tiles = rep.outer_tiles = 1;
			rep.proven = box.lo_key() == box.hi_key();
			rep.text = tiles::box_text(box);
			rep.lo_key = box.lo_key();
			rep.hi_key = box.hi_key();
			if (!rep.proven) rep.note = "one tile among " + std::to_string(box.hi_key() - box.lo_key() + 1) + ": a lattice point reached by a non-dyadic route, or too close to one to separate";
			return rep;
		}
		if (inputs.size() > 8) { rep.note = "more than 8 uncertain inputs"; return rep; }

		// each input's set, in this type's lattice, closed
		const TypeOps ops = od::oracle_ops<Tile>(256);
		const std::size_t K = inputs.size();
		std::vector<orc::interval> sets;
		{
			ExpressionEvaluator eval(ops);
			eval.inherit(session);
			std::size_t next = 0;
			for (std::size_t i = 0; i < defs.size(); ++i) {
				const Value v = eval.evaluate(defs[i].first + " =" + defs[i].second);
				if (next < K && inputs[next] == i) { sets.push_back(orc::closed(od::of(v).v)); ++next; }
			}
		}
		for (const auto& s : sets) {
			if (s.nan || s.lo_inf || s.hi_inf) { rep.note = "an input is not a bounded set"; return rep; }
		}

		using Piece = std::vector<orc::interval>;
		// the inputs as duals over a piece: input d carries the unit gradient e_d
		auto seeded = [&](const Piece& x) {
			Overrides m;
			for (std::size_t d = 0; d < K; ++d) {
				std::vector<orc::interval> g(K, od::zero());
				g[d] = orc::point(orc::make_dyadic(1));
				m[inputs[d]] = { x[d], g };
			}
			return m;
		};
		auto at_point = [&](const std::vector<orc::dyadic>& p) {
			Overrides m;
			for (std::size_t d = 0; d < K; ++d) m[inputs[d]] = { orc::point(p[d]), {} };
			return m;
		};

		std::int64_t inner_lo = I::kmax, inner_hi = -I::kmax, outer_lo = I::kmax, outer_hi = -I::kmax;
		// a value the computation takes certifies its tile; one unresolved among [a, b] still
		// certifies [min of b, max of a] over all of them
		auto certify = [&](const I& b) {
			if (b.isnan()) return;
			inner_lo = std::min(inner_lo, b.hi_key());
			inner_hi = std::max(inner_hi, b.lo_key());
			outer_lo = std::min(outer_lo, b.lo_key());
			outer_hi = std::max(outer_hi, b.hi_key());
		};
		std::vector<Piece> queue{ sets };
		std::size_t head = 0;
		int evaluations = 0;
		bool unbounded = false, all_monotone = true;
		while (head < queue.size()) {
			const Piece piece = queue[head++];
			const od::dual f = od::evaluate(ops, session, expr, seeded(piece));
			++evaluations;
			// the centre: a value the computation takes, and the anchor of the mean-value form
			std::vector<orc::dyadic> c;
			for (const auto& x : piece) c.push_back(od::midpoint(x));
			const orc::interval fc = od::evaluate(ops, session, expr, at_point(c)).v;
			const I cb = orc::box<Tile>(fc);
			certify((cb.isnan() || cb.lo_key() == cb.hi_key()) ? cb : point_box(at_point(c)));
			// enclose the piece by the naive form intersected with the mean-value form
			// f(c) + sum_d g_d(piece) (piece_d - c_d), whose excess shrinks quadratically
			orc::interval enc = f.v;
			bool finite_gradient = f.g.size() == K;
			for (const auto& g : f.g) finite_gradient = finite_gradient && !g.nan && !g.lo_inf && !g.hi_inf;
			if (finite_gradient && !fc.nan) {
				orc::interval mv = fc;
				for (std::size_t d = 0; d < K; ++d) {
					mv = orc::add(mv, orc::mul(f.g[d], orc::interval{ orc::sub(piece[d].lo, c[d]), orc::sub(piece[d].hi, c[d]) }));
				}
				const orc::interval both = orc::intersect(enc, mv);
				if (!both.nan) enc = both;
			}
			const I b = orc::box<Tile>(enc);
			const bool bounded = !b.isnan() && b.lo_key() != -I::kmax && b.hi_key() != I::kmax;
			// the sign of each partial derivative over the piece: +1, -1, 0 (constant), 2 (undecided)
			std::vector<int> dir(K, 0);
			bool monotone = bounded;
			for (std::size_t d = 0; d < K && monotone; ++d) {
				const orc::interval g = d < f.g.size() ? f.g[d] : od::zero();
				if (g.nan) monotone = false;
				else if (!g.lo_inf && orc::sign(g.lo) > 0) dir[d] = 1;
				else if (!g.hi_inf && orc::sign(g.hi) < 0) dir[d] = -1;
				else if (orc::is_point(g) && orc::sign(g.lo) == 0) dir[d] = 0;
				else { dir[d] = 2; monotone = false; }
			}
			if (monotone) {   // the image of the piece runs between two corners: evaluate them exactly
				std::vector<orc::dyadic> lo, hi;
				for (std::size_t d = 0; d < K; ++d) {
					lo.push_back(dir[d] >= 0 ? piece[d].lo : piece[d].hi);
					hi.push_back(dir[d] >= 0 ? piece[d].hi : piece[d].lo);
				}
				certify(point_box(at_point(lo)));
				certify(point_box(at_point(hi)));
				continue;
			}
			all_monotone = false;
			if (bounded && b.lo_key() >= inner_lo && b.hi_key() <= inner_hi) continue;   // inside the known image
			// bisect along the input that contributes most to the variation, width(x_d) |df/dx_d|
			// (in log2): splitting it narrows the dependency-induced widths fastest.  A decided
			// input can be the right one to split -- its width is what keeps the others undecided.
			int split = -1;
			int best = 0;
			for (std::size_t d = 0; d < K; ++d) {
				const orc::dyadic w = orc::sub(piece[d].hi, piece[d].lo);
				if (orc::sign(w) == 0) continue;
				const orc::interval g = d < f.g.size() ? f.g[d] : od::zero();
				int mag = 0;                                       // log2 of max(|g.lo|, |g.hi|)
				if (g.nan || g.lo_inf || g.hi_inf) mag = 1 << 20;
				else if (orc::sign(g.lo) == 0 && orc::sign(g.hi) == 0) continue;
				else mag = std::max(orc::sign(g.lo) == 0 ? -(1 << 20) : orc::msb(g.lo.m) + g.lo.e, orc::sign(g.hi) == 0 ? -(1 << 20) : orc::msb(g.hi.m) + g.hi.e);
				const int score = (orc::msb(w.m) + w.e) + mag;
				if (split < 0 || score > best) { best = score; split = static_cast<int>(d); }
			}
			if (evaluations >= budget || split < 0) {   // accept the enclosure as it is
				if (!bounded) { unbounded = true; break; }
				outer_lo = std::min(outer_lo, b.lo_key());
				outer_hi = std::max(outer_hi, b.hi_key());
				continue;
			}
			Piece left = piece, right = piece;
			const orc::dyadic m = od::midpoint(piece[static_cast<std::size_t>(split)]);
			left[static_cast<std::size_t>(split)].hi = m;
			right[static_cast<std::size_t>(split)].lo = m;
			queue.push_back(std::move(left));
			queue.push_back(std::move(right));
		}
		if (unbounded) { rep.note = "no exact enclosure: a function the oracle cannot bound (log, exp, trigonometric), or a division by a set holding zero"; return rep; }

		rep.available = true;
		rep.subdivisions = evaluations;
		rep.outer_tiles = static_cast<std::uint64_t>(outer_hi) - static_cast<std::uint64_t>(outer_lo) + 1u;
		rep.inner_tiles = inner_lo <= inner_hi ? static_cast<std::uint64_t>(inner_hi) - static_cast<std::uint64_t>(inner_lo) + 1u : 1u;
		rep.proven = inner_lo == outer_lo && inner_hi == outer_hi;
		rep.text = tiles::box_text(I::from_keys(outer_lo, outer_hi));
		rep.lo_key = outer_lo;
		rep.hi_key = outer_hi;
		if (!rep.proven) {
			rep.note = all_monotone ? std::string("a corner value is a lattice point reached by a non-dyadic route")
			                        : "bracketed, not proven: the computation is not monotone everywhere on the inputs, and its extreme values were not reached at a point the search evaluates (" + std::to_string(evaluations) + " evaluations)";
		}
	}
	catch (const std::exception& ex) {
		rep.available = false;
		rep.note = ex.what();
	}
	return rep;
}

// attach the oracle to a tile interval type's TypeOps
template<typename Tile>
TypeOps with_oracle(TypeOps ops) {
	ops.tightest = [](const ExpressionEvaluator& session, const std::string& expr) { return tightest_box<Tile>(session, expr); };
	return ops;
}

}} // namespace sw::ucalc
