#pragma once
// roots.hpp: root finding with tile intervals (#1654, Phase 4)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
//   roots a b c     the quadratic a x^2 + b x + c by its formula, each root as written and in
//                   the stable form, with the tightest box, the overestimation, the sign and
//                   the discriminant's verdict per type, and a precision sweep: the narrowest
//                   width at which each question becomes decidable.
//   rootbox f ...   the roots of f in a domain, by bisection over the tiles on the sign
//                   verdict of f: pieces of one sign are dropped, the rest are split.
//
// IMPORTANT: include after uncertainty.hpp and oracle.hpp.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace sw { namespace ucalc {

///////////////////////////////////////////////////////////////////////////
// the quadratic

// The coefficients live in reserved variables, so they are replayed in every type like any
// other definition: roots 3~ 100~ 2~ is ULP-wide in each type's own lattice.
inline constexpr const char* quadratic_names[3] = { "_qa_", "_qb_", "_qc_" };

struct QuadraticForm {
	std::string root;     // r1 (the root the formula as written computes by cancellation) or r2
	std::string form;     // "as written" or "stable"
	std::string expr;
};

// The formula as written, (-b +- sqrt(b^2 - 4ac)) / (2a), cancels in the root where -b and
// the square root have opposite signs.  The stable form takes q = -(b + sign(b) sqrt(d)) / 2:
// the roots are q / a, the formula as written itself, and c / q (Vieta), which replaces the
// cancelling one.  r1 is the root of smaller magnitude, r2 the other, as in #1649.
inline std::vector<QuadraticForm> quadratic_forms(bool b_negative) {
	const std::string a = "_qa_", b = "_qb_", c = "_qc_";
	const std::string d = "sqrt(" + b + "*" + b + " - 4*" + a + "*" + c + ")";
	const std::string plus = "(-" + b + " + " + d + ") / (2*" + a + ")";
	const std::string minus = "(-" + b + " - " + d + ") / (2*" + a + ")";
	if (!b_negative) {
		return { { "r1", "as written", plus }, { "r1", "stable", "(2*" + c + ") / (-" + b + " - " + d + ")" }, { "r2", "as written", minus } };
	}
	return { { "r1", "as written", minus }, { "r1", "stable", "(2*" + c + ") / (-" + b + " + " + d + ")" }, { "r2", "as written", plus } };
}

inline std::string discriminant_expr() { return "_qb_^2 - 4*_qa_*_qc_"; }

inline const char* discriminant_verdict(const std::string& sign) {
	if (sign == "positive") return "two real roots";
	if (sign == "negative") return "complex roots";
	if (sign == "zero") return "a double root";
	return "undecidable";
}

// The session with the coefficients defined, in a tile type so that x~ and [a, b] are read.
inline ExpressionEvaluator quadratic_session(const TypeRegistry& reg, const ExpressionEvaluator& session,
                                             const std::string& a, const std::string& b, const std::string& c) {
	ExpressionEvaluator s(reg.get("poxel64i"));
	s.inherit(session);
	s.evaluate(std::string(quadratic_names[0]) + " = " + a);
	s.evaluate(std::string(quadratic_names[1]) + " = " + b);
	s.evaluate(std::string(quadratic_names[2]) + " = " + c);
	return s;
}

// the coefficients from "a b c" or "a, b, c"
inline bool split_coefficients(const std::string& text, std::vector<std::string>& out) {
	out.clear();
	const bool commas = text.find(',') != std::string::npos;
	std::string cur;
	int depth = 0;
	for (char ch : text) {
		if (ch == '(' || ch == '[') ++depth;
		if (ch == ')' || ch == ']') --depth;
		const bool cut = depth == 0 && (commas ? ch == ',' : (ch == ' ' || ch == '\t'));
		if (cut) {
			if (!cur.empty()) out.push_back(cur);
			cur.clear();
		}
		else cur.push_back(ch);
	}
	if (!cur.empty()) out.push_back(cur);
	for (auto& s : out) {
		const auto b = s.find_first_not_of(" \t"), e = s.find_last_not_of(" \t");
		s = (b == std::string::npos) ? std::string() : s.substr(b, e - b + 1);
	}
	out.erase(std::remove(out.begin(), out.end(), std::string()), out.end());
	return out.size() == 3;
}

struct RootRow {
	QuadraticForm form;
	BoxReport box;
	bool contains = false;        // the oracle's tightest box lies inside the computed box
};

struct QuadraticReport {
	std::string type;
	std::string discriminant_box, discriminant;   // the computed verdict
	std::string exact_discriminant;               // the oracle's verdict, when it has one
	std::vector<RootRow> rows;
	std::string error;
};

inline QuadraticReport quadratic_in(const TypeOps& ops, const std::string& alias, const ExpressionEvaluator& qsession,
                                    const std::vector<QuadraticForm>& forms, bool with_tightest) {
	QuadraticReport rep;
	rep.type = alias;
	if (ops.family != "tile interval") { rep.error = "not a tile interval type"; return rep; }
	try {
		ExpressionEvaluator eval = evaluator_for(ops, qsession);
		const Value d = eval.evaluate(discriminant_expr());
		rep.discriminant_box = d.native_rep;
		rep.discriminant = discriminant_verdict(d.tile_sign);
		if (with_tightest && ops.tightest) {
			const TightestReport t = ops.tightest(qsession, discriminant_expr());
			if (t.available && t.proven) {
				rep.exact_discriminant = discriminant_verdict(t.lo_key > 0 ? "positive" : t.hi_key < 0 ? "negative"
				                                              : (t.lo_key == 0 && t.hi_key == 0) ? "zero" : "undecidable");
			}
		}
		for (const QuadraticForm& f : forms) {
			RootRow row;
			row.form = f;
			const Value v = eval.evaluate(f.expr);
			row.box = box_report(alias, v);
			if (std::isnan(v.tile_lower) && d.tile_sign == "negative") row.box.box = "no real root (the discriminant is negative)";
			if (with_tightest && ops.tightest) {
				row.box.tightest = ops.tightest(qsession, f.expr);
				const TightestReport& t = row.box.tightest;
				row.contains = t.available && v.tile_kind == 2 && !std::isnan(v.tile_lower) && t.lo_key >= v.tile_lo_key && t.hi_key <= v.tile_hi_key;
			}
			rep.rows.push_back(std::move(row));
		}
	}
	catch (const std::exception& ex) {
		rep.error = ex.what();
	}
	return rep;
}

// The precision sweep: for each question, the narrowest width of each family that decides it.
struct SweepRow {
	std::string question;
	std::vector<std::pair<std::string, int>> narrowest;   // family, bits (0: not decided at any width)
};

inline std::vector<SweepRow> precision_sweep(const TypeRegistry& reg, const ExpressionEvaluator& qsession,
                                             const std::vector<QuadraticForm>& forms, double digits) {
	const std::vector<std::pair<std::string, std::vector<std::string>>> families{
		{ "areal", { "areal8i", "areal16i", "areal32i", "areal64i" } },
		{ "poxel", { "poxel8i", "poxel16i", "poxel32i", "poxel64i" } },
	};
	std::vector<std::string> questions{ "number of real roots" };
	for (const auto& f : forms) questions.push_back("sign of " + f.root + " " + f.form);
	std::ostringstream dq;
	dq << digits;
	for (const auto& f : forms) questions.push_back(dq.str() + " digits of " + f.root + " " + f.form);
	std::vector<SweepRow> rows;
	for (const auto& q : questions) rows.push_back({ q, {} });
	for (const auto& [family, types] : families) {
		std::vector<int> first(questions.size(), 0);
		for (const std::string& t : types) {
			const TypeOps* ops = reg.find(t);
			if (ops == nullptr) continue;
			const int bits = ops->nbits / 2;
			const QuadraticReport r = quadratic_in(*ops, t, qsession, forms, false);
			if (!r.error.empty()) continue;
			std::vector<bool> decided{ r.discriminant != std::string("undecidable") };
			for (const auto& row : r.rows) decided.push_back(row.box.sign != "undecidable" && row.box.encloses);
			for (const auto& row : r.rows) decided.push_back(row.box.decimals >= digits);
			for (std::size_t i = 0; i < decided.size(); ++i) if (decided[i] && first[i] == 0) first[i] = bits;
		}
		for (std::size_t i = 0; i < rows.size(); ++i) rows[i].narrowest.emplace_back(family, first[i]);
	}
	return rows;
}

///////////////////////////////////////////////////////////////////////////
// rootbox: the roots of f in a domain, by bisection over the tiles

struct RootBox {
	std::int64_t lo_key = 0, hi_key = 0;
	std::string box;
	std::uint64_t tiles = 0;
	std::string status;   // "root" (a sign change proves one), "exact root", "undecided", "unexplored"
	std::string detail;
};

struct RootboxReport {
	std::string type;
	std::vector<RootBox> boxes;
	int evaluations = 0;
	bool exhausted = false;   // the budget ran out: some boxes were not split to single tiles
	std::string error;
};

inline RootboxReport rootbox_in(const TypeOps& ops, const std::string& alias, const ExpressionEvaluator& session,
                                const std::string& expr, const std::string& var, const std::string& domain, int budget = 4000) {
	RootboxReport rep;
	rep.type = alias;
	if (ops.family != "tile interval" || !ops.from_keys) { rep.error = "not a tile interval type"; return rep; }
	try {
		ExpressionEvaluator eval = evaluator_for(ops, session);
		const Value dom = eval.evaluate(domain);
		if (dom.tile_kind != 2 || std::isnan(dom.tile_lower)) { rep.error = "the domain is not a set of real numbers"; return rep; }
		auto f = [&](std::int64_t lo, std::int64_t hi) {
			++rep.evaluations;
			eval.set_variable(var, ops.from_keys(lo, hi));
			return eval.evaluate(expr);
		};
		auto no_value = [](const Value& v) { return v.tile_kind == 0 || std::isnan(v.tile_lower); };

		// depth first, left half first, so the surviving pieces come out in order
		struct Piece { std::int64_t lo, hi; bool exact_zero; bool unexplored; };
		std::vector<Piece> stack{ { dom.tile_lo_key, dom.tile_hi_key, false, false } }, kept;
		while (!stack.empty()) {
			const Piece p = stack.back();
			stack.pop_back();
			const Value v = f(p.lo, p.hi);
			if (no_value(v)) continue;                                   // no real value of f here
			if (v.tile_sign == "positive" || v.tile_sign == "negative") continue;   // one sign: no root
			if (v.tile_sign == "zero") { kept.push_back({ p.lo, p.hi, p.lo == p.hi && (p.lo & 1) == 0, false }); continue; }
			if (p.lo == p.hi) { kept.push_back(p); continue; }
			if (rep.evaluations >= budget) {   // not searched: reported apart, never mixed with candidates
				rep.exhausted = true;
				kept.push_back({ p.lo, p.hi, false, true });
				continue;
			}
			const std::int64_t mid = p.lo + (p.hi - p.lo) / 2;
			stack.push_back({ mid + 1, p.hi, false, false });
			stack.push_back({ p.lo, mid, false, false });
		}

		// adjacent pieces form one box; its outer lattice points decide existence
		for (std::size_t i = 0; i < kept.size();) {
			std::size_t j = i;
			bool exact = kept[i].exact_zero;
			while (j + 1 < kept.size() && kept[j + 1].lo == kept[j].hi + 1 && kept[j + 1].unexplored == kept[i].unexplored) { ++j; exact = exact || kept[j].exact_zero; }
			RootBox rb;
			rb.lo_key = kept[i].lo;
			rb.hi_key = kept[j].hi;
			const Value whole = ops.from_keys(rb.lo_key, rb.hi_key);
			rb.box = whole.native_rep;
			rb.tiles = whole.tile_count;
			if (kept[i].unexplored) {
				rb.status = "unexplored";
				rb.detail = "the budget ran out before this part of the domain was searched";
			}
			else if (exact) {
				rb.status = "exact root";
				rb.detail = "f is exactly 0 at a lattice point";
			}
			else {
				const std::int64_t left = (rb.lo_key & 1) ? rb.lo_key - 1 : rb.lo_key;
				const std::int64_t right = (rb.hi_key & 1) ? rb.hi_key + 1 : rb.hi_key;
				const Value over = f(rb.lo_key, rb.hi_key);
				const bool bounded = !no_value(over) && std::isfinite(over.tile_lower) && std::isfinite(over.tile_upper);
				const Value fl = f(left, left), fr = f(right, right);
				const bool change = (fl.tile_sign == "negative" && fr.tile_sign == "positive") || (fl.tile_sign == "positive" && fr.tile_sign == "negative");
				if (change && bounded) {
					rb.status = "root";
					rb.detail = "f changes sign across the box, and is bounded on it";
				}
				else {
					rb.status = "undecided";
					rb.detail = !bounded ? "f is unbounded on the box (a pole?)"
					          : (fl.tile_sign == fr.tile_sign && fl.tile_sign != "undecidable") ? "f has the same sign at both ends: no root, a double root, or two"
					          : "the sign of f at an end is undecidable";
				}
			}
			rep.boxes.push_back(std::move(rb));
			i = j + 1;
		}
	}
	catch (const std::exception& ex) {
		rep.error = ex.what();
	}
	return rep;
}

// split "<expr> for <var> in [lo, hi] [types...]"
inline bool parse_rootbox(const TypeRegistry& reg, const std::string& text, std::string& expr, std::string& var,
                          std::string& domain, std::vector<std::string>& types, std::string& error) {
	const std::size_t f = text.find(" for ");
	if (f == std::string::npos) { error = "usage: rootbox <expr> for <var> in [lo, hi] [types...]"; return false; }
	expr = text.substr(0, f);
	std::string rest = text.substr(f + 5);
	const std::size_t in = rest.find(" in ");
	if (in == std::string::npos) { error = "usage: rootbox <expr> for <var> in [lo, hi] [types...]"; return false; }
	var = rest.substr(0, in);
	var.erase(std::remove_if(var.begin(), var.end(), [](char ch) { return std::isspace(static_cast<unsigned char>(ch)) != 0; }), var.end());
	rest = rest.substr(in + 4);
	const std::size_t open = rest.find('['), close = rest.find(']');
	if (open == std::string::npos || close == std::string::npos || close < open) { error = "the domain is written [lo, hi]"; return false; }
	domain = rest.substr(open, close - open + 1);
	std::istringstream words(rest.substr(close + 1));
	types.clear();
	for (std::string w; words >> w;) {
		if (reg.find(w) == nullptr) { error = "unknown type: " + w; return false; }
		types.push_back(w);
	}
	if (var.empty()) { error = "no variable"; return false; }
	return true;
}

}} // namespace sw::ucalc
