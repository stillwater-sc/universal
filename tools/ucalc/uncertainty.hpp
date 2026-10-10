#pragma once
// uncertainty.hpp: the uncertainty box of a computation, and the questions it can decide
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// The ubox and decide commands (#1654, Phase 2).  An expression is evaluated in a set of
// tile interval types; each result is a box guaranteed to contain the true value.  A box
// is summarized by its width in tiles, its relative width, the decimals of accuracy its
// midpoint is guaranteed to have, and its sign verdict.  A predicate on the result is
// decidable in a type when it has the same answer for every point of the box.
//
// IMPORTANT: include after expression.hpp and tiles.hpp.
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace sw { namespace ucalc {

// the tile interval types of equal storage that #1649 compares, and the rounded types of
// the same widths
inline const std::vector<std::string>& default_box_types() {
	static const std::vector<std::string> types{ "areal16i", "poxel16i", "areal32i", "poxel32i", "areal64i", "poxel64i" };
	return types;
}
inline const std::vector<std::string>& default_rounded_types() {
	static const std::vector<std::string> types{ "fp16", "posit16", "float", "posit32", "double" };
	return types;
}

// An evaluator for `ops` that carries a session's variables.  The values come first, as a
// fallback; then every definition is replayed in this type, so `b = 100~` is the open tile
// above 100 in this type's own lattice.  Definitions this type cannot evaluate (x~ in a
// rounded type) are listed in `failed`; their variables keep the session's value.
inline ExpressionEvaluator evaluator_for(const TypeOps& ops, const ExpressionEvaluator& session,
                                         std::vector<std::string>* failed = nullptr) {
	ExpressionEvaluator eval(ops);
	eval.inherit(session);
	for (const auto& [name, text] : session.definitions()) {
		try {
			eval.evaluate(name + " =" + text);
		}
		catch (const std::exception&) {
			if (failed) failed->push_back(name);
		}
	}
	return eval;
}

///////////////////////////////////////////////////////////////////////////
// box summary

struct BoxReport {
	std::string type;
	std::string box;              // the set, as native_rep shows it
	std::uint64_t tiles = 0;
	std::string sign;             // negative, zero, positive, undecidable
	bool exact = false;           // one exact point: the value itself
	bool bounded = false;         // both ends finite
	double rel_width = 0.0;       // (upper - lower) / |midpoint|; inf when unbounded or straddling zero
	double decimals = 0.0;        // -log10(rel_width / 2): the guaranteed decimals of the midpoint
	bool encloses = false;        // the set is known to contain the true value
	std::string error;
};

// Decimals of accuracy as in docs/tutorials/decimals-of-accuracy.md: a rounded x carries
// -log10(ulp / 2|x|).  A box [l, u] guarantees its midpoint m a relative error of at most
// (u - l) / 2|m|, so it carries -log10((u - l) / 2|m|) decimals, and 0 when that is negative
// or the box straddles zero.  An exact point carries all of them: decimals is inf.
inline BoxReport box_report(const std::string& type, const Value& v) {
	BoxReport r;
	r.type = type;
	r.box = v.native_rep;
	r.tiles = v.tile_count;
	r.sign = v.tile_sign;
	r.encloses = v.tile_encloses;
	const long double l = v.tile_lower, u = v.tile_upper;
	if (v.tile_kind == 0) { r.error = "not a tile type"; return r; }
	if (std::isnan(l) || std::isnan(u)) { r.sign = "undecidable"; r.rel_width = std::numeric_limits<double>::quiet_NaN(); return r; }
	r.bounded = !std::isinf(l) && !std::isinf(u);
	r.exact = r.bounded && l == u && !v.ubit;
	if (r.exact) {
		r.rel_width = 0.0;
		r.decimals = std::numeric_limits<double>::infinity();
		return r;
	}
	const bool one_sign = (l > 0) || (u < 0) || (l == 0 && v.tile_lower_open) || (u == 0 && v.tile_upper_open);
	if (!r.bounded || !one_sign) {
		r.rel_width = std::numeric_limits<double>::infinity();
		return r;
	}
	const long double m = (l + u) / 2;
	r.rel_width = static_cast<double>((u - l) / std::fabs(m));
	r.decimals = std::max(0.0, -std::log10(r.rel_width / 2));
	return r;
}

///////////////////////////////////////////////////////////////////////////
// predicates

enum class Relation { sign, lt, le, gt, ge, eq, ne };
enum class Verdict { yes, no, undecidable };

inline const char* to_string(Verdict v) {
	switch (v) {
	case Verdict::yes: return "yes";
	case Verdict::no:  return "no";
	default:           return "undecidable";
	}
}

struct Predicate {
	Relation rel = Relation::sign;
	std::string lhs, rhs;         // rhs is empty for sign
	std::string op;               // the operator as written
};

// "sign <expr>", or "<expr> op <expr>" with op one of < <= > >= == !=, split at the one
// comparison outside parentheses and brackets
inline bool parse_predicate(const std::string& text, Predicate& p, std::string& error) {
	auto trim = [](const std::string& s) {
		const auto b = s.find_first_not_of(" \t");
		if (b == std::string::npos) return std::string();
		return s.substr(b, s.find_last_not_of(" \t") - b + 1);
	};
	const std::string t = trim(text);
	if (t.rfind("sign ", 0) == 0 || t.rfind("sign\t", 0) == 0) {
		p.rel = Relation::sign;
		p.op = "sign";
		p.lhs = trim(t.substr(4));
		if (p.lhs.empty()) { error = "sign needs an expression"; return false; }
		return true;
	}
	int depth = 0;
	std::size_t at = std::string::npos, len = 0;
	for (std::size_t i = 0; i < t.size(); ++i) {
		const char c = t[i];
		if (c == '(' || c == '[') ++depth;
		else if (c == ')' || c == ']') --depth;
		else if (depth == 0 && (c == '<' || c == '>' || c == '=' || c == '!')) {
			const bool two = i + 1 < t.size() && t[i + 1] == '=';
			if (c == '=' && !two) { error = "use == for equality"; return false; }
			if (c == '!' && !two) { error = "use != for inequality"; return false; }
			if (at != std::string::npos) { error = "one comparison per predicate"; return false; }
			at = i;
			len = two ? 2 : 1;
			i += len - 1;
		}
	}
	if (at == std::string::npos) { error = "expected 'sign <expr>' or '<expr> op <expr>' with op one of < <= > >= == !="; return false; }
	p.op = t.substr(at, len);
	p.lhs = trim(t.substr(0, at));
	p.rhs = trim(t.substr(at + len));
	if (p.lhs.empty() || p.rhs.empty()) { error = "comparison needs an expression on each side"; return false; }
	if (p.op == "<") p.rel = Relation::lt;
	else if (p.op == "<=") p.rel = Relation::le;
	else if (p.op == ">") p.rel = Relation::gt;
	else if (p.op == ">=") p.rel = Relation::ge;
	else if (p.op == "==") p.rel = Relation::eq;
	else p.rel = Relation::ne;
	return true;
}

// The ends of a set, for comparison: the bounds and whether each end is excluded.
struct Ends {
	long double lo, hi;
	bool lo_open, hi_open;
	bool valid;
};

inline Ends ends_of(const Value& v) {
	Ends e{ v.tile_lower, v.tile_upper, v.tile_lower_open, v.tile_upper_open, true };
	e.valid = v.tile_kind != 0 && v.tile_encloses && !std::isnan(e.lo) && !std::isnan(e.hi);
	return e;
}

// every a is below every b
inline bool all_below(const Ends& a, const Ends& b) {
	return a.hi < b.lo || (a.hi == b.lo && (a.hi_open || b.lo_open));
}
// every a is at or below every b
inline bool all_at_or_below(const Ends& a, const Ends& b) {
	return a.hi <= b.lo;
}

// The verdict of a op b over every pair of points of the two sets.  The bounds are exact
// where long double holds the lattice and rounded outward elsewhere, so a verdict is never
// wrong; at worst a decidable question reads as undecidable.
inline Verdict compare(Relation rel, const Ends& a, const Ends& b) {
	if (!a.valid || !b.valid) return Verdict::undecidable;
	auto verdict = [](bool always, bool never) { return always ? Verdict::yes : (never ? Verdict::no : Verdict::undecidable); };
	switch (rel) {
	case Relation::lt: return verdict(all_below(a, b), all_at_or_below(b, a));
	case Relation::le: return verdict(all_at_or_below(a, b), all_below(b, a));
	case Relation::gt: return verdict(all_below(b, a), all_at_or_below(a, b));
	case Relation::ge: return verdict(all_at_or_below(b, a), all_below(a, b));
	case Relation::eq: {
		const bool same_point = a.lo == a.hi && b.lo == b.hi && a.lo == b.lo && !a.lo_open && !a.hi_open && !b.lo_open && !b.hi_open;
		return verdict(same_point, all_below(a, b) || all_below(b, a));
	}
	case Relation::ne: {
		const Verdict eq = compare(Relation::eq, a, b);
		return eq == Verdict::yes ? Verdict::no : (eq == Verdict::no ? Verdict::yes : Verdict::undecidable);
	}
	default: return Verdict::undecidable;
	}
}

struct Decision {
	std::string type;
	int nbits = 0;
	Verdict verdict = Verdict::undecidable;
	std::string answer;           // the verdict, or the sign for a sign predicate
	std::string lhs_box, rhs_box;
	std::string error;
};

// decide p in one type, with the session's variables replayed there
inline Decision decide_in(const TypeOps& ops, const std::string& alias, const ExpressionEvaluator& session, const Predicate& p) {
	Decision d;
	d.type = alias;
	d.nbits = ops.nbits;
	if (ops.family.empty()) { d.error = "not a tile type"; d.answer = "-"; return d; }
	try {
		ExpressionEvaluator eval = evaluator_for(ops, session);
		const Value a = eval.evaluate(p.lhs);
		d.lhs_box = a.native_rep;
		if (p.rel == Relation::sign) {
			const bool known = a.tile_encloses && a.tile_sign != "undecidable";
			d.verdict = known ? Verdict::yes : Verdict::undecidable;
			d.answer = a.tile_encloses ? a.tile_sign : std::string("undecidable");
			return d;
		}
		const Value b = eval.evaluate(p.rhs);
		d.rhs_box = b.native_rep;
		d.verdict = compare(p.rel, ends_of(a), ends_of(b));
		d.answer = to_string(d.verdict);
	}
	catch (const std::exception& ex) {
		d.error = ex.what();
		d.answer = "error";
	}
	return d;
}

// the narrowest type whose verdict is decided, by storage width; the first listed on a tie
inline const Decision* narrowest_decided(const std::vector<Decision>& ds) {
	const Decision* best = nullptr;
	for (const Decision& d : ds) {
		if (!d.error.empty() || d.verdict == Verdict::undecidable) continue;
		if (best == nullptr || d.nbits < best->nbits) best = &d;
	}
	return best;
}

///////////////////////////////////////////////////////////////////////////
// rounded types next to the boxes: what they compute, and how far that is from a qd
// reference -- which they give no sign of

struct RoundedReport {
	std::string type;
	std::string value;
	double rel_error = 0.0;
	std::string error;
};

inline std::vector<RoundedReport> rounded_reports(const TypeRegistry& reg, const ExpressionEvaluator& session,
                                                  const std::string& expr, std::string& reference, std::string& note) {
	std::vector<RoundedReport> out;
	std::vector<std::string> failed;
	sw::universal::qd ref;
	try {
		ExpressionEvaluator ref_eval = evaluator_for(reg.get("qd"), session, &failed);
		if (!failed.empty()) {
			note = "the inputs use tile syntax (" + failed.front() + (failed.size() > 1 ? ", ..." : "") + "), so there is no single true value to compare against";
			return out;
		}
		const Value r = ref_eval.evaluate(expr);
		ref = extract<sw::universal::qd>(r);
		reference = r.native_rep;
	}
	catch (const std::exception& ex) {
		note = std::string("no qd reference: ") + ex.what();
		return out;
	}
	for (const std::string& alias : default_rounded_types()) {
		const TypeOps* ops = reg.find(alias);
		if (ops == nullptr) continue;
		RoundedReport rr;
		rr.type = alias;
		try {
			ExpressionEvaluator eval = evaluator_for(*ops, session);
			const Value v = eval.evaluate(expr);
			rr.value = v.native_rep;
			if (ref == 0) rr.rel_error = (v.num == 0.0) ? 0.0 : std::numeric_limits<double>::infinity();
			else rr.rel_error = double(sw::universal::abs((sw::universal::qd(v.num) - ref) / ref));
		}
		catch (const std::exception& ex) {
			rr.error = ex.what();
		}
		out.push_back(std::move(rr));
	}
	return out;
}

// split "<text> in t1 t2 ..." into the text and the types, when every word after the
// last " in " names a registered type; otherwise the whole line is the text
inline std::vector<std::string> split_types(const TypeRegistry& reg, std::string& text) {
	const std::size_t at = text.rfind(" in ");
	if (at == std::string::npos) return {};
	std::vector<std::string> types;
	std::istringstream words(text.substr(at + 4));
	for (std::string w; words >> w;) {
		if (reg.find(w) == nullptr) return {};
		types.push_back(w);
	}
	if (types.empty()) return {};
	text.resize(at);
	return types;
}

}} // namespace sw::ucalc
